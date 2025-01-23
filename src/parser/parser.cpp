#include <lib2k/defer.hpp>
#include <lib2k/string_utils.hpp>
#include <lib2k/types.hpp>
#include <parser/constant_definition.hpp>
#include <parser/parser.hpp>
#include <parser/parser_note.hpp>
#include <parser/type_definition.hpp>
#include <parser/variable_declarations.hpp>
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
            current_is(TokenType::Label) ? tl::optional{ this->label_declarations() } : tl::nullopt;
        auto constant_definitions =
            current_is(TokenType::Const) ? tl::optional{ this->constant_definitions() } : tl::nullopt;
        auto type_definitions = current_is(TokenType::Type) ? tl::optional{ this->type_definitions() } : tl::nullopt;
        auto variable_declarations =
            current_is(TokenType::Var) ? tl::optional{ this->variable_declarations() } : tl::nullopt;
        return Block{
            std::move(label_declarations),
            std::move(constant_definitions),
            std::move(type_definitions),
            std::move(variable_declarations),
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
            return std::make_unique<ConstantReference>(sign, identifier_token.value());
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

        if (auto const up_arrow_token = match(TokenType::UpArrow)) {
            return std::make_unique<PointerTypeDefinition>(pointer_type(up_arrow_token.value()));
        }

        if (auto const real_token = match(TokenType::Real)) {
            return std::make_unique<RealType>(real_token.value());
        }
        return ordinal_type();
    }

    [[nodiscard]] PointerTypeDefinition pointer_type(std::same_as<Token const> auto& up_arrow_token) {
        if (auto const identifier = match(TokenType::Identifier)) {
            return PointerTypeDefinition{ up_arrow_token, Identifier{ identifier.value() } };
        }
        if (auto const integer = match(TokenType::Integer)) {
            return PointerTypeDefinition{ up_arrow_token, IntegerType{ integer.value() } };
        }
        if (auto const real = match(TokenType::Real)) {
            return PointerTypeDefinition{ up_arrow_token, RealType{ real.value() } };
        }
        if (auto const char_ = match(TokenType::Char)) {
            return PointerTypeDefinition{ up_arrow_token, CharType{ char_.value() } };
        }
        if (auto const boolean = match(TokenType::Boolean)) {
            return PointerTypeDefinition{ up_arrow_token, BooleanType{ boolean.value() } };
        }

        throw ParserError{ "Expected type reference after `^`.", up_arrow_token.source_location() };
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
        if (auto const set = match(TokenType::Set)) {
            return std::make_unique<SetTypeDefinition>(set_type_definition(set.value()));
        }
        if (auto const file = match(TokenType::File)) {
            return std::make_unique<FileTypeDefinition>(file_type_definition(file.value()));
        }
        // TODO: File types.
        // TODO: Pointer types.
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

        auto field_list = this->field_list();
        auto const& end = expect(TokenType::End, "Expected `end`.");
        return RecordTypeDefinition{
            record_token,
            std::move(field_list),
            end,
        };
    }

    [[nodiscard]] SetTypeDefinition set_type_definition(std::same_as<Token const> auto& set_token) {
        expect(TokenType::Of, "Expected `of` in set type definition.");
        auto base_type = ordinal_type();
        return SetTypeDefinition{ set_token, std::move(base_type) };
    }

    [[nodiscard]] FileTypeDefinition file_type_definition(std::same_as<Token const> auto& file_token) {
        expect(TokenType::Of, "Expected `of` in file type definition.");
        auto component_type = type();
        return FileTypeDefinition{ file_token, std::move(component_type) };
    }

    [[nodiscard]] VariableDeclarations variable_declarations() {
        auto const& var_token = expect(TokenType::Var, "Expected `var`.");
        auto declarations = std::vector<VariableDeclaration>{};
        declarations.push_back(this->variable_declaration());
        expect(TokenType::Semicolon, "Expected `;`.");

        while (current_is(TokenType::Identifier)) {
            declarations.push_back(this->variable_declaration());
            expect(TokenType::Semicolon, "Expected `;`.");
        }

        return VariableDeclarations{ var_token, std::move(declarations) };
    }

    [[nodiscard]] VariableDeclaration variable_declaration() {
        auto identifiers = identifier_list();
        expect(TokenType::Colon, "Expected `:`.");
        auto type = this->type();
        return VariableDeclaration{ std::move(identifiers), std::move(type) };
    }

    [[nodiscard]] FieldList field_list() {
        auto fixed_part = tl::optional<FixedPart>{};
        auto variant_part = tl::optional<VariantPart>{};

        if (current_is(TokenType::Identifier)) {
            fixed_part = record_fixed_part();
            if (continues_with(TokenType::Semicolon, TokenType::Case)) {
                expect(TokenType::Semicolon, "Expected `;`.");  // Should never fail.
                auto const& case_ = expect(TokenType::Case, "Expected `case`.");  // Should never fail.
                variant_part = this->variant_part(case_);
            }
        } else if (auto const case_ = match(TokenType::Case)) {
            variant_part = this->variant_part(case_.value());
        } else {
            throw_parser_error("Expected field list.", current().source_location());
        }

        std::ignore = match(TokenType::Semicolon);

        return FieldList{ std::move(fixed_part), std::move(variant_part) };
    }

    [[nodiscard]] VariantPart variant_part(std::same_as<Token const> auto& case_token) {
        auto variant_selector = this->variant_selector();
        expect(TokenType::Of, "Expected `of`.");
        auto variant_list = this->variant_list();
        return VariantPart{ case_token, std::move(variant_selector), std::move(variant_list) };
    }

    [[nodiscard]] VariantSelector variant_selector() {
        auto tag_field = tl::optional<Identifier>{};
        if (continues_with(TokenType::Identifier, TokenType::Colon)) {
            // The next two lines should never fail.
            tag_field.emplace(expect(TokenType::Identifier, "Expected identifier."));
            expect(TokenType::Colon, "Expected `:`.");
        }
        auto tag_type = ordinal_type();
        return VariantSelector{ std::move(tag_field), std::move(tag_type) };
    }

    [[nodiscard]] VariantList variant_list() {
        auto list = std::vector<Variant>{};
        list.push_back(variant());
        while (match(TokenType::Semicolon) and current_is_none_of(TokenType::End, TokenType::RightParenthesis)) {
            list.push_back(variant());
        }
        return VariantList{ std::move(list) };
    }

    [[nodiscard]] Variant variant() {
        auto case_constant_list = this->case_constant_list();
        expect(TokenType::Colon, "Expected `:`.");
        expect(TokenType::LeftParenthesis, "Expected `(`.");
        if (auto const& closing_parenthesis = match(TokenType::RightParenthesis)) {
            return Variant{
                std::move(case_constant_list),
                tl::nullopt,
                closing_parenthesis.value(),
            };
        }
        auto field_list = this->field_list();
        auto const& closing_parenthesis = expect(TokenType::RightParenthesis, "Expected `)`.");
        return Variant{
            std::move(case_constant_list),
            std::make_unique<FieldList>(std::move(field_list)),
            closing_parenthesis,
        };
    }

    [[nodiscard]] CaseConstantList case_constant_list() {
        auto constants = std::vector<std::unique_ptr<Constant>>{};
        constants.push_back(constant());
        while (match(TokenType::Comma)) {
            constants.push_back(constant());
        }
        return CaseConstantList{ std::move(constants) };
    }

    [[nodiscard]] FixedPart record_fixed_part() {
        auto record_sections = std::vector<RecordSection>{};
        record_sections.push_back(record_section());
        while (continues_with(TokenType::Semicolon, TokenType::Identifier)) {
            std::ignore = match(TokenType::Semicolon);
            record_sections.push_back(record_section());
        }
        return FixedPart{ std::move(record_sections) };
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

    [[nodiscard]] bool current_is_none_of(std::same_as<TokenType> auto const... types) const {
        return not current_is_any_of(types...);
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
