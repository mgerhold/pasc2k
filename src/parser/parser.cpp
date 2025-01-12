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
        auto label_declarations =
            current_is(TokenType::Label) ? std::optional{ this->label_declarations() } : std::nullopt;
        auto constant_definitions =
            current_is(TokenType::Const) ? std::optional{ this->constant_definitions() } : std::nullopt;
        auto type_definitions = current_is(TokenType::Type) ? std::optional{ this->type_definitions() } : std::nullopt;
        return Ast{
            std::move(m_tokens),
            Block{
                  std::move(label_declarations),
                  std::move(constant_definitions),
                  std::move(type_definitions),
                  }
        };
    }

private:
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

        auto definitions = std::vector<std::unique_ptr<ConstantDefinition>>{};
        definitions.push_back(constant_definition(create_notes));
        expect(TokenType::Semicolon, "Expected semicolon after constant definition.", create_notes());
        while (current_is(TokenType::Identifier)) {
            definitions.push_back(constant_definition(create_notes));
            expect(TokenType::Semicolon, "Expected semicolon after constant definition.", create_notes());
        }
        return ConstantDefinitions{ const_token, std::move(definitions) };
    }

    [[nodiscard]] std::unique_ptr<ConstantDefinition> constant_definition(auto const& create_notes) {
        auto const& identifier =
            expect(TokenType::Identifier, "Expected identifier in constant definition.", create_notes());

        expect(TokenType::Equals, "Expected equals sign in constant definition.", create_notes());

        if (auto const integer_token = match(TokenType::IntegerNumber)) {
            return std::make_unique<IntegerConstant>(identifier, IntegerLiteral{ integer_token.value() });
        }
        if (auto const real_token = match(TokenType::RealNumber)) {
            return std::make_unique<RealConstant>(identifier, RealLiteral{ real_token.value() });
        }
        if (auto const char_token = match(TokenType::Char)) {
            return std::make_unique<CharConstant>(identifier, CharLiteral{ char_token.value() });
        }
        if (auto const string_token = match(TokenType::String)) {
            return std::make_unique<StringConstant>(identifier, StringLiteral{ string_token.value() });
        }

        auto const sign = [&]() -> tl::optional<Token const&> {
            if (auto const plus_token = match(TokenType::Plus)) {
                return plus_token.value();
            }
            if (auto const minus_token = match(TokenType::Minus)) {
                return minus_token.value();
            }
            return tl::nullopt;
        }();

        if (auto const identifier_token = match(TokenType::Identifier)) {
            return std::make_unique<ConstantReference>(identifier, sign, identifier_token.value());
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

        auto definitions = std::vector<std::unique_ptr<TypeDefinition>>{};
        definitions.push_back(type_definition(create_notes));
        expect(TokenType::Semicolon, "Expected semicolon after type definition.", create_notes());
        while (current_is(TokenType::Identifier)) {
            definitions.push_back(type_definition(create_notes));
            expect(TokenType::Semicolon, "Expected semicolon after type definition.", create_notes());
        }
        return TypeDefinitions{ type_token, std::move(definitions) };
    }

    [[nodiscard]] std::unique_ptr<TypeDefinition> type_definition(auto const& create_notes) {
        auto const& identifier =
            expect(TokenType::Identifier, "Expected identifier in type definition.", create_notes());

        expect(TokenType::Equals, "Expected equals sign in type definition.", create_notes());

        if (auto const alias = match(TokenType::Identifier)) {
            return std::make_unique<TypeAlias>(identifier, alias.value());
        }

        throw ParserError{
            "Expected type definition.",
            current().source_location(),
            create_notes(),
        };
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

    [[nodiscard]] Token const& peek() const {
        if (m_index + 1 < m_tokens.size()) {
            return m_tokens.at(m_index + 1);
        }
        return m_tokens.back();
    }

    [[nodiscard]] bool current_is(TokenType const type) const {
        return current().type() == type;
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
