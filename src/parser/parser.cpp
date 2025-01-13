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

        auto const create_notes = [&] {
            return std::vector{
                ParserNote{ label_token.source_location(), "In label declarations starting from here." }
            };
        };

        auto declarations = std::vector<LabelDeclaration>{};
        declarations.push_back(label(create_notes));
        while (match(TokenType::Comma)) {
            declarations.push_back(label(create_notes));
        }
        expect(TokenType::Semicolon, "Expected semicolon after label declarations.", create_notes());
        return LabelDeclarations{ label_token, declarations };
    }

    [[nodiscard]] LabelDeclaration label(auto const& create_notes) {
        // 6.1.6
        auto const& token = expect(TokenType::IntegerNumber, "Expected label.");
        if (not std::isdigit(static_cast<unsigned char>(token.lexeme().at(0)))) {
            throw ParserError{ "Expected label", token.source_location(), create_notes() };
        }
        return LabelDeclaration{ IntegerLiteral{ token } };
    }

    [[nodiscard]] ConstantDefinitions constant_definitions() {
        auto const& const_token = expect(TokenType::Const, "Expected const.");
        auto const create_notes = [&] {
            return std::vector{
                ParserNote{ const_token.source_location(), "In constant definitions starting from here." }
            };
        };

        auto definitions = std::vector<ConstantDefinition>{};
        definitions.push_back(constant_definition(create_notes));
        expect(TokenType::Semicolon, "Expected semicolon after constant definition.", create_notes());
        while (current_is(TokenType::Identifier)) {
            definitions.push_back(constant_definition(create_notes));
            expect(TokenType::Semicolon, "Expected semicolon after constant definition.", create_notes());
        }
        return ConstantDefinitions{ const_token, std::move(definitions) };
    }

    [[nodiscard]] ConstantDefinition constant_definition(auto const& create_notes) {
        auto const& identifier =
            expect(TokenType::Identifier, "Expected identifier in constant definition.", create_notes());
        expect(TokenType::Equals, "Expected equals sign in constant definition.", create_notes());
        return ConstantDefinition{ Identifier{ identifier }, constant(create_notes) };
    }

    [[nodiscard]] std::unique_ptr<Constant> constant(auto const& create_notes) {
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
            throw ParserError{
                "Expected integer, real, or identifier after sign in constant definition.",
                current().source_location(),
                create_notes(),
            };
        }

        if (auto const integer_token = match(TokenType::IntegerNumber)) {
            return std::make_unique<IntegerConstant>(sign, IntegerLiteral{ integer_token.value() });
        }
        if (auto const real_token = match(TokenType::RealNumber)) {
            return std::make_unique<RealConstant>(sign, RealLiteral{ real_token.value() });
        }
        if (auto const identifier_token = match(TokenType::Identifier)) {
            return std::make_unique<ConstantReference>(sign, identifier_token.value());
        }

        if (auto const char_token = match(TokenType::CharValue)) {
            return std::make_unique<CharConstant>(CharLiteral{ char_token.value() });
        }
        if (auto const string_token = match(TokenType::StringValue)) {
            return std::make_unique<StringConstant>(StringLiteral{ string_token.value() });
        }

        throw ParserError{
            "Expected constant value in constant definition.",
            current().source_location(),
            create_notes(),
        };
    }

    [[nodiscard]] TypeDefinitions type_definitions() {
        auto const& type_token = expect(TokenType::Type, "Expected `type`.");
        auto const create_notes = [&] {
            return std::vector{
                ParserNote{
                           type_token.source_location(),
                           "In type definitions starting from here.", }
            };
        };

        auto definitions = std::vector<TypeDefinition>{};
        definitions.push_back(type_definition(create_notes));
        expect(TokenType::Semicolon, "Expected semicolon after type definition.", create_notes());
        while (current_is(TokenType::Identifier)) {
            definitions.push_back(type_definition(create_notes));
            expect(TokenType::Semicolon, "Expected semicolon after type definition.", create_notes());
        }
        return TypeDefinitions{ type_token, std::move(definitions) };
    }

    [[nodiscard]] TypeDefinition type_definition(auto const& create_notes) {
        auto const& identifier =
            expect(TokenType::Identifier, "Expected identifier in type definition.", create_notes());

        expect(TokenType::Equals, "Expected equals sign in type definition.", create_notes());

        return TypeDefinition{ Identifier{ identifier }, type(create_notes) };
    }

    [[nodiscard]] std::unique_ptr<Type> type(auto const& create_notes) {
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
            return std::make_unique<StructuredTypeDefinition>(structured_type_definition(create_notes));
        }
        // clang-format on

        if (auto const real_token = match(TokenType::Real)) {
            return std::make_unique<RealType>(real_token.value());
        }
        return ordinal_type(create_notes);
    }

    [[nodiscard]] StructuredTypeDefinition structured_type_definition(auto const& create_notes) {
        auto const packed = match(TokenType::Packed);
        return StructuredTypeDefinition{ packed, unpacked_structured_type_definition(create_notes) };
    }

    [[nodiscard]] std::unique_ptr<UnpackedStructuredTypeDefinition> unpacked_structured_type_definition(
        auto const& create_notes
    ) {
        if (auto const array = match(TokenType::Array)) {
            return std::make_unique<ArrayTypeDefinition>(array_type_definition(array.value(), create_notes));
        }
        if (auto const record = match(TokenType::Record)) {
            return std::make_unique<RecordTypeDefinition>(record_type_definition(record.value(), create_notes));
        }
        /*if (auto const set = match(TokenType::Set)) {
            return set_type_definition(create_notes);
        }
        if (auto const file = match(TokenType::File)) {
            return file_type_definition(create_notes);
        }*/
        throw ParserError{
            "Expected structured type definition.",
            current().source_location(),
            create_notes(),
        };
    }

    [[nodiscard]] ArrayTypeDefinition array_type_definition(Token const& array_token, auto const& create_notes) {
        expect(TokenType::LeftSquareBracket, "Expected `[` in array type definition.", create_notes());
        auto index_types = std::vector<std::unique_ptr<OrdinalType>>{};
        index_types.push_back(ordinal_type(create_notes));
        while (match(TokenType::Comma)) {
            index_types.push_back(ordinal_type(create_notes));
        }
        expect(TokenType::RightSquareBracket, "Expected `]` in array type definition.", create_notes());
        expect(TokenType::Of, "Expected `of` in array type definition.", create_notes());
        auto element_type = type(create_notes);
        return ArrayTypeDefinition{ array_token, std::move(index_types), std::move(element_type) };
    }

    [[nodiscard]] RecordTypeDefinition record_type_definition(Token const& record_token, auto const& create_notes) {
        if (auto const end = match(TokenType::End)) {
            return RecordTypeDefinition{ record_token, tl::nullopt, end.value() };
        }
        // TODO: Variant part, if current token is `CASE`.
        auto fixed_part = record_fixed_part(create_notes);
        std::ignore = match(TokenType::Semicolon);
        auto const& end_token = expect(TokenType::End, "Expected `end`.", create_notes());
        return RecordTypeDefinition{ record_token, std::move(fixed_part), end_token };
    }

    [[nodiscard]] RecordFixedPart record_fixed_part(auto const& create_notes) {
        auto record_sections = std::vector<RecordSection>{};
        record_sections.push_back(record_section(create_notes));
        while (continues_with(TokenType::Semicolon, TokenType::Identifier)) {
            std::ignore = match(TokenType::Semicolon);
            record_sections.push_back(record_section(create_notes));
        }
        return RecordFixedPart{ std::move(record_sections) };
    }

    [[nodiscard]] RecordSection record_section(auto const& create_notes) {
        auto identifiers = identifier_list(create_notes);
        expect(TokenType::Colon, "Expected `:` in record section.", create_notes());
        auto type = this->type(create_notes);
        return RecordSection{ std::move(identifiers), std::move(type) };
    }

    [[nodiscard]] std::unique_ptr<OrdinalType> ordinal_type(auto const& create_notes) {
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
            return enumerated_type_definition(create_notes);
        }

        auto const is_subrange_type =
            current_is_any_of(TokenType::Plus, TokenType::Minus, TokenType::CharValue, TokenType::IntegerNumber)
            or continues_with(TokenType::Identifier, TokenType::DotDot);

        if (is_subrange_type) {
            return std::make_unique<SubrangeTypeDefinition>(subrange_type(create_notes));
        }

        // We don't really know whether a type alias is an ordinal type. This will be
        // resolved during semantic analysis. For now, we treat it as an ordinal type.
        return std::make_unique<TypeAliasDefinition>(Identifier{
            expect(TokenType::Identifier, "Expected identifier in type definition.", create_notes()),
        });
    }

    [[nodiscard]] SubrangeTypeDefinition subrange_type(auto const& create_notes) {
        // clang-format off
        if (
            current_is_any_of(TokenType::Plus, TokenType::Minus, TokenType::CharValue, TokenType::IntegerNumber)
            or continues_with(TokenType::Identifier, TokenType::DotDot)
        ) {
            // clang-format on
            auto from = constant(create_notes);
            expect(TokenType::DotDot, "Expected `..` in subrange type definition.", create_notes());
            auto to = constant(create_notes);
            return SubrangeTypeDefinition(std::move(from), std::move(to));
        }

        throw ParserError{
            "Expected type definition.",
            current().source_location(),
            create_notes(),
        };
    }

    [[nodiscard]] std::unique_ptr<EnumeratedTypeDefinition> enumerated_type_definition(auto const& create_notes) {
        auto const& left_parenthesis =
            expect(TokenType::LeftParenthesis, "Expected `(` in enumerated type definition.", create_notes());
        auto identifiers = identifier_list(create_notes);
        auto const& right_parenthesis =
            expect(TokenType::RightParenthesis, "Expected `)` in enumerated type definition.", create_notes());
        return std::make_unique<EnumeratedTypeDefinition>(left_parenthesis, std::move(identifiers), right_parenthesis);
    }

    [[nodiscard]] IdentifierList identifier_list(auto const& create_notes) {
        auto identifiers = std::vector<Identifier>{};
        identifiers.emplace_back(expect(TokenType::Identifier, "Expected identifier.", create_notes()));
        while (match(TokenType::Comma)) {
            identifiers.emplace_back(expect(TokenType::Identifier, "Expected identifier.", create_notes()));
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

    Token const& expect(TokenType const type, std::string const& error_message, std::vector<ParserNote> notes = {}) {
        if (auto const& token = match(type)) {
            return token.value();
        }
        throw ParserError{ error_message, current().source_location(), std::move(notes) };
    }

    void advance() {
        if (not is_at_end()) {
            m_index++;
        }
    }
};

[[nodiscard]] Ast parse(std::vector<Token>&& tokens) {
    return Parser{ std::move(tokens) }.parse();
}
