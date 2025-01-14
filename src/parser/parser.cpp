#include <lib2k/defer.hpp>
#include <lib2k/string_utils.hpp>
#include <lib2k/types.hpp>
#include <parser/constant_definition.hpp>
#include <parser/parser.hpp>
#include <parser/parser_note.hpp>
#include <parser/type_definition.hpp>
#include <sstream>
#include <tl/optional.hpp>

class Parser final {
private:
    std::vector<Token> m_tokens;
    usize m_index = 0;
    std::vector<ParserNote> m_notes_stack;

public:
    [[nodiscard]] explicit Parser(std::vector<Token>&& tokens)
        : m_tokens{ std::move(tokens) } {
        assert(not m_tokens.empty());
        assert(m_tokens.back().type() == TokenType::EndOfFile);
    }

    [[nodiscard]] Ast parse() & = delete;

    [[nodiscard]] Ast parse() && {
        auto block = this->block();
        expect(TokenType::EndOfFile, "Expected end of file.");
        return Ast{ std::move(m_tokens), std::move(block) };
    }

private:
    [[nodiscard]] auto scoped_note(SourceLocation const& location, std::string message) {
        m_notes_stack.emplace_back(location, std::move(message));
        return c2k::Defer([this] { m_notes_stack.pop_back(); });
    }

    [[nodiscard]] Block block() {
        auto label_declarations =
            current_is(TokenType::Label) ? std::optional{ this->label_declarations() } : std::nullopt;
        auto constant_definitions =
            current_is(TokenType::Const) ? std::optional{ this->constant_definitions() } : std::nullopt;
        auto type_definitions = current_is(TokenType::Type) ? std::optional{ this->type_definitions() } : std::nullopt;
        return Block{
            std::move(label_declarations),
            std::move(constant_definitions),
            std::move(type_definitions),
        };
    }

    [[nodiscard]] LabelDeclarations label_declarations() {
        auto const& label_token = expect(TokenType::Label, "Expected label.");

        auto const _ = scoped_note(label_token.source_location(), "In label declarations starting from here.");

        auto declarations = std::vector<LabelDeclaration>{};
        declarations.push_back(label());
        while (match(TokenType::Comma)) {
            declarations.push_back(label());
        }
        expect(TokenType::Semicolon, "Expected semicolon after label declarations.");
        return LabelDeclarations{ label_token, declarations };
    }

    [[nodiscard]] LabelDeclaration label() {
        // 6.1.6
        auto const& token = expect(TokenType::IntegerNumber, "Expected label.");
        if (not std::isdigit(static_cast<unsigned char>(token.lexeme().at(0)))) {
            throw_parser_error("Expected label", token.source_location());
        }
        return LabelDeclaration{ IntegerLiteral{ token } };
    }

    [[nodiscard]] ConstantDefinitions constant_definitions() {
        auto const& const_token = expect(TokenType::Const, "Expected const.");
        auto const _ = scoped_note(const_token.source_location(), "In constant definitions starting from here.");

        auto definitions = std::vector<ConstantDefinition>{};
        definitions.push_back(constant_definition());
        expect(TokenType::Semicolon, "Expected semicolon after constant definition.");
        while (current_is(TokenType::Identifier)) {
            definitions.push_back(constant_definition());
            expect(TokenType::Semicolon, "Expected semicolon after constant definition.");
        }
        return ConstantDefinitions{ const_token, std::move(definitions) };
    }

    [[nodiscard]] ConstantDefinition constant_definition() {
        auto const& identifier = expect(TokenType::Identifier, "Expected identifier in constant definition.");
        expect(TokenType::Equals, "Expected equals sign in constant definition.");
        return ConstantDefinition{ Identifier{ identifier }, constant() };
    }

    [[nodiscard]] std::unique_ptr<Constant> constant() {
        auto const sign = [&]() -> tl::optional<Token const&> {
            if (auto const plus_token = match(TokenType::Plus)) {
                return plus_token.value();
            }
            if (auto const minus_token = match(TokenType::Minus)) {
                return minus_token.value();
            }
            return tl::nullopt;
        }();

        // clang-format off
        if (
            sign.has_value()
            and not current_is(TokenType::IntegerNumber)
            and not current_is(TokenType::RealNumber)
            and not current_is(TokenType::Identifier)
        ) {
            // clang-format on
            throw_parser_error(
                "Expected integer, real, or identifier after sign in constant definition.",
                current().source_location()
            );
        }

        if (auto const integer_token = match(TokenType::IntegerNumber)) {
            return std::make_unique<IntegerConstant>(sign, IntegerLiteral{ integer_token.value() });
        }
        if (auto const real_token = match(TokenType::RealNumber)) {
            return std::make_unique<RealConstant>(sign, RealLiteral{ real_token.value() });
        }
        if (auto const identifier_token = match(TokenType::Identifier)) {
            return std::make_unique<ConstantReference>(identifier_token, identifier_token.value());
        }

        if (auto const char_token = match(TokenType::CharValue)) {
            return std::make_unique<CharConstant>(CharLiteral{ char_token.value() });
        }
        if (auto const string_token = match(TokenType::StringValue)) {
            return std::make_unique<StringConstant>(StringLiteral{ string_token.value() });
        }

        throw_parser_error("Expected constant value in constant definition.", current().source_location());
    }

    [[nodiscard]] TypeDefinitions type_definitions() {
        auto const& type_token = expect(TokenType::Type, "Expected `type`.");

        auto const _ = scoped_note(type_token.source_location(), "In type definitions starting from here.");

        auto definitions = std::vector<TypeDefinition>{};
        definitions.push_back(type_definition());
        expect(TokenType::Semicolon, "Expected semicolon after type definition.");
        while (current_is(TokenType::Identifier)) {
            definitions.push_back(type_definition());
            expect(TokenType::Semicolon, "Expected semicolon after type definition.");
        }
        return TypeDefinitions{ type_token, std::move(definitions) };
    }

    [[nodiscard]] TypeDefinition type_definition() {
        auto const& identifier = expect(TokenType::Identifier, "Expected identifier in type definition.");

        auto const _ =
            scoped_note(identifier.source_location(), std::format("In type definition of `{}`.", identifier.lexeme()));

        expect(TokenType::Equals, "Expected equals sign in type definition.");

        return TypeDefinition{ Identifier{ identifier }, type() };
    }

    [[nodiscard]] std::unique_ptr<Type> type() {
        // clang-format off
        if (
            current_is_any_of(
                TokenType::Array,
                TokenType::Record,
                TokenType::Set,
                TokenType::File,
                TokenType::Packed
            )
        ) {
            return std::make_unique<StructuredTypeDefinition>(structured_type_definition());
        }
        // clang-format on

        if (auto const real_token = match(TokenType::Real)) {
            return std::make_unique<RealType>(real_token.value());
        }
        return ordinal_type();
    }

    [[nodiscard]] StructuredTypeDefinition structured_type_definition() {
        auto const packed = match(TokenType::Packed);
        return StructuredTypeDefinition{ packed, unpacked_structured_type_definition() };
    }

    [[nodiscard]] std::unique_ptr<UnpackedStructuredTypeDefinition> unpacked_structured_type_definition() {
        if (auto const array = match(TokenType::Array)) {
            return std::make_unique<ArrayTypeDefinition>(array_type_definition(array.value()));
        }
        if (auto const record = match(TokenType::Record)) {
            return std::make_unique<RecordTypeDefinition>(record_type_definition(record.value()));
        }
        /*if (auto const set = match(TokenType::Set)) {
            return set_type_definition(create_notes);
        }
        if (auto const file = match(TokenType::File)) {
            return file_type_definition(create_notes);
        }*/
        throw_parser_error("Expected structured type definition.", current().source_location());
    }

    [[nodiscard]] ArrayTypeDefinition array_type_definition(Token const& array_token) {
        expect(TokenType::LeftSquareBracket, "Expected `[` in array type definition.");
        auto index_types = std::vector<std::unique_ptr<OrdinalType>>{};
        index_types.push_back(ordinal_type());
        while (match(TokenType::Comma)) {
            index_types.push_back(ordinal_type());
        }
        expect(TokenType::RightSquareBracket, "Expected `]` in array type definition.");
        expect(TokenType::Of, "Expected `of` in array type definition.");
        auto element_type = type();
        return ArrayTypeDefinition{ array_token, std::move(index_types), std::move(element_type) };
    }

    [[nodiscard]] RecordTypeDefinition record_type_definition(Token const& record_token) {
        if (auto const end = match(TokenType::End)) {
            return RecordTypeDefinition{ record_token, tl::nullopt, end.value() };
        }
        // TODO: Variant part, if current token is `CASE`.
        auto fixed_part = record_fixed_part();
        std::ignore = match(TokenType::Semicolon);
        auto const& end_token = expect(TokenType::End, "Expected `end`.");
        return RecordTypeDefinition{ record_token, FieldList{ std::move(fixed_part) }, end_token };
    }

    [[nodiscard]] RecordFixedPart record_fixed_part() {
        auto record_sections = std::vector<RecordSection>{};
        record_sections.push_back(record_section());
        while (continues_with(TokenType::Semicolon, TokenType::Identifier)) {
            std::ignore = match(TokenType::Semicolon);
            record_sections.push_back(record_section());
        }
        return RecordFixedPart{ std::move(record_sections) };
    }

    [[nodiscard]] RecordSection record_section() {
        auto identifiers = identifier_list();
        expect(TokenType::Colon, "Expected `:` in record section.");
        auto type = this->type();
        return RecordSection{ std::move(identifiers), std::move(type) };
    }

    [[nodiscard]] std::unique_ptr<OrdinalType> ordinal_type() {
        if (auto const boolean_token = match(TokenType::Boolean)) {
            return std::make_unique<BooleanType>(boolean_token.value());
        }
        if (auto const char_token = match(TokenType::Char)) {
            return std::make_unique<CharType>(char_token.value());
        }
        if (auto const integer_token = match(TokenType::Integer)) {
            return std::make_unique<IntegerType>(integer_token.value());
        }

        if (current_is(TokenType::LeftParenthesis)) {
            return enumerated_type_definition();
        }

        auto const is_subrange_type =
            current_is_any_of(TokenType::Plus, TokenType::Minus, TokenType::CharValue, TokenType::IntegerNumber)
            or continues_with(TokenType::Identifier, TokenType::DotDot);

        if (is_subrange_type) {
            return std::make_unique<SubrangeTypeDefinition>(subrange_type());
        }

        // We don't really know whether a type alias is an ordinal type. This will be
        // resolved during semantic analysis. For now, we treat it as an ordinal type.
        return std::make_unique<TypeAliasDefinition>(Identifier{
            expect(TokenType::Identifier, "Expected identifier in type definition."),
        });
    }

    [[nodiscard]] SubrangeTypeDefinition subrange_type() {
        // clang-format off
        if (
            current_is_any_of(TokenType::Plus, TokenType::Minus, TokenType::CharValue, TokenType::IntegerNumber)
            or continues_with(TokenType::Identifier, TokenType::DotDot)
        ) {
            // clang-format on
            auto from = constant();
            expect(TokenType::DotDot, "Expected `..` in subrange type definition.");
            auto to = constant();
            return SubrangeTypeDefinition(std::move(from), std::move(to));
        }

        throw_parser_error("Expected type definition.", current().source_location());
    }

    [[nodiscard]] std::unique_ptr<EnumeratedTypeDefinition> enumerated_type_definition() {
        auto const& left_parenthesis = expect(TokenType::LeftParenthesis, "Expected `(` in enumerated type definition.");
        auto identifiers = identifier_list();
        auto const& right_parenthesis =
            expect(TokenType::RightParenthesis, "Expected `)` in enumerated type definition.");
        return std::make_unique<EnumeratedTypeDefinition>(left_parenthesis, std::move(identifiers), right_parenthesis);
    }

    [[nodiscard]] IdentifierList identifier_list() {
        auto identifiers = std::vector<Identifier>{};
        identifiers.emplace_back(expect(TokenType::Identifier, "Expected identifier."));
        while (match(TokenType::Comma)) {
            identifiers.emplace_back(expect(TokenType::Identifier, "Expected identifier."));
        }
        return IdentifierList{ std::move(identifiers) };
    }

    [[nodiscard]] bool is_at_end() const {
        return m_index >= m_tokens.size() or m_tokens.at(m_index).type() == TokenType::EndOfFile;
    }

    [[nodiscard]] Token const& current() const {
        if (not is_at_end()) {
            return m_tokens.at(m_index);
        }
        return m_tokens.back();
    }

    [[nodiscard]] Token const& peek(usize const offset = 1) const {
        auto const position = m_index + offset;
        if (position < m_tokens.size()) {
            return m_tokens.at(position);
        }
        return m_tokens.back();
    }

    [[nodiscard]] bool current_is(TokenType const type) const {
        return current().type() == type;
    }

    [[nodiscard]] bool current_is_any_of(std::same_as<TokenType> auto const... types) const {
        return ((current().type() == types) or ...);
    }

    [[nodiscard]] bool continues_with(std::same_as<TokenType> auto const... types) const {
        auto offset = usize{ 0 };
        return ([&] {
            auto const actual_type = peek(offset).type();
            auto const result = actual_type == types;
            ++offset;
            return result;
        }() and ...);
    }

    [[nodiscard]] tl::optional<Token const&> match(TokenType const type) {
        if (current_is(type)) {
            auto const& result = current();
            advance();
            return result;
        }
        return tl::nullopt;
    }

    Token const& expect(TokenType const type, std::string const& error_message) {
        if (auto const& token = match(type)) {
            return token.value();
        }
        throw_parser_error(error_message, current().source_location());
    }

    void advance() {
        if (not is_at_end()) {
            m_index++;
        }
    }

    [[noreturn]] void throw_parser_error(std::string const& message, SourceLocation const& location) const {
        throw ParserError{ message, location, m_notes_stack };
    }
};

[[nodiscard]] Ast parse(std::vector<Token>&& tokens) {
    return Parser{ std::move(tokens) }.parse();
}
