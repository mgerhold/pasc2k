#include <lib2k/string_utils.hpp>
#include <lib2k/types.hpp>
#include <parser/constant_definition.hpp>
#include <parser/parser.hpp>
#include <parser/parser_note.hpp>
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

    [[nodiscard]] Block parse() & = delete;

    [[nodiscard]] Block parse() && {
        auto label_declarations = this->label_declarations();
        auto constant_definitions = this->constant_definitions();
        return Block{
            std::move(m_tokens),
            std::move(label_declarations),
            std::move(constant_definitions),
        };
    }

private:
    [[nodiscard]] std::vector<LabelDeclaration> label_declarations() {
        auto const label_token = match(TokenType::Label);
        if (not label_token.has_value()) {
            return {};
        }

        auto const create_notes = [&] {
            return std::vector{
                ParserNote{ label_token.value().source_location(), "In label declarations starting from here." }
            };
        };

        auto declarations = std::vector<LabelDeclaration>{};
        declarations.push_back(label(create_notes));
        while (match(TokenType::Comma)) {
            declarations.push_back(label(create_notes));
        }
        expect(TokenType::Semicolon, "Expected semicolon after label declarations.", create_notes());
        return declarations;
    }

    [[nodiscard]] LabelDeclaration label(auto const& create_notes) {
        // 6.1.6
        auto const& token = expect(TokenType::IntegerNumber, "Expected label.");
        if (not std::isdigit(static_cast<unsigned char>(token.lexeme().at(0)))) {
            throw ParserError{ "Expected label", token.source_location(), create_notes() };
        }
        auto const parsed = c2k::parse<i64>(token.lexeme());
        if (not parsed.has_value() or parsed.value() > 9999) {
            throw ParserError{ "Label value out of bounds (0 to 9999)", token.source_location(), create_notes() };
        }
        return LabelDeclaration{ parsed.value() };
    }

    [[nodiscard]] std::vector<ConstantDefinition> constant_definitions() {
        auto const const_token = match(TokenType::Const);
        if (not const_token.has_value()) {
            return {};
        }

        auto const create_notes = [&] {
            return std::vector{
                ParserNote{ const_token.value().source_location(), "In constant definitions starting from here." }
            };
        };

        auto definitions = std::vector<ConstantDefinition>{};
        definitions.push_back(constant_definition(create_notes));
        expect(TokenType::Semicolon, "Expected semicolon after constant definition.", create_notes());
        while (current_is(TokenType::Identifier)) {
            definitions.push_back(constant_definition(create_notes));
            expect(TokenType::Semicolon, "Expected semicolon after constant definition.", create_notes());
        }
        return definitions;
    }

    [[nodiscard]] ConstantDefinition constant_definition(auto const& create_notes) {
        auto const& identifier =
            expect(TokenType::Identifier, "Expected identifier in constant definition.", create_notes());

        expect(TokenType::Equals, "Expected equals sign in constant definition.", create_notes());

        if (auto const integer_token = match(TokenType::IntegerNumber)) {
            auto const number = c2k::parse<i64>(integer_token.value().lexeme());
            if (not number.has_value()) {
                throw ParserError{
                    "Integer constant out of range.",
                    integer_token.value().source_location(),
                    create_notes(),
                };
            }
            return ConstantDefinition{ identifier, number.value() };
        }
        if (auto const real_token = match(TokenType::RealNumber)) {
            return ConstantDefinition{ identifier, extract_real_number(real_token.value()) };
        }
        if (auto const char_token = match(TokenType::Char)) {
            return ConstantDefinition{ identifier, extract_char(char_token.value()) };
        }
        if (auto const string_token = match(TokenType::String)) {
            return ConstantDefinition{ identifier, extract_string(string_token.value()) };
        }

        auto const sign = [&] {
            if (match(TokenType::Plus)) {
                return ConstantReference::Sign::Positive;
            }
            if (match(TokenType::Minus)) {
                return ConstantReference::Sign::Negative;
            }
            return ConstantReference::Sign::None;
        }();

        if (auto const identifier_token = match(TokenType::Identifier)) {
            return ConstantDefinition{
                identifier,
                ConstantReference{ sign, identifier_token.value() }
            };
        }
        throw ParserError{
            "Expected constant value in constant definition.",
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

    [[nodiscard]] static double extract_real_number(Token const& token) {
        if (token.type() != TokenType::RealNumber) {
            throw InternalCompilerError{ "Expected real number." };
        }
        auto stream = std::istringstream{ std::string{ token.lexeme() } };
        auto result = 0.0;
        stream >> result;
        if (not stream.eof()) {
            throw InternalCompilerError{ "Failed to parse real number." };
        }
        return result;
    }

    [[nodiscard]] static char extract_char(Token const& token) {
        if (token.type() != TokenType::Char) {
            throw InternalCompilerError{ "Expected character." };
        }
        if (token.lexeme().length() > 3) {
            if (token.lexeme().at(1) != '\'' or token.lexeme().at(2) != '\'' or token.lexeme().length() != 4) {
                throw InternalCompilerError{ "Invalid character literal." };
            }
            return '\'';
        }
        if (token.lexeme().length() != 3) {
            throw InternalCompilerError{ "Invalid character literal." };
        }
        return token.lexeme().at(1);
    }

    [[nodiscard]] static std::string extract_string(Token const& token) {
        auto result = std::string{};
        auto const lexeme = token.lexeme();
        for (auto i = usize{ 1 }; i < lexeme.length() - 1;) {
            if (lexeme.at(i) == '\'' and lexeme.at(i + 1) == '\'') {
                result += '\'';
                i += 2;
            } else {
                result += lexeme.at(i);
                ++i;
            }
        }
        return result;
    }
};

[[nodiscard]] Block parse(std::vector<Token>&& tokens) {
    return Parser{ std::move(tokens) }.parse();
}
