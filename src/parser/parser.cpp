#include <lib2k/string_utils.hpp>
#include <lib2k/types.hpp>
#include <parser/parser.hpp>
#include <parser/parser_note.hpp>
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
        return Block{ std::move(m_tokens), std::move(label_declarations) };
    }

private:
    [[nodiscard]] std::vector<LabelDeclaration> label_declarations() {
        auto const label_token = match(TokenType::Label);
        if (not label_token.has_value()) {
            return {};
        }

        auto const create_note = [&] {
            return std::vector{
                ParserNote{ label_token.value().source_location(), "In label declarations starting from here." }
            };
        };

        auto declarations = std::vector<LabelDeclaration>{};
        declarations.push_back(label(create_note));
        while (match(TokenType::Comma)) {
            declarations.push_back(label(create_note));
        }
        expect(TokenType::Semicolon, "Expected semicolon after label declarations.", create_note());
        return declarations;
    }

    [[nodiscard]] LabelDeclaration label(auto const& create_note) {
        // 6.1.6
        auto const& token = expect(TokenType::IntegerNumber, "Expected label.");
        if (not std::isdigit(static_cast<unsigned char>(token.lexeme().at(0)))) {
            throw ParserError{ "Expected label", token.source_location(), create_note() };
        }
        auto const parsed = c2k::parse<i64>(token.lexeme());
        if (not parsed.has_value() or parsed.value() > 9999) {
            throw ParserError{ "Label value out of bounds (0 to 9999)", token.source_location(), create_note() };
        }
        return LabelDeclaration{ parsed.value() };
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

[[nodiscard]] Block parse(std::vector<Token>&& tokens) {
    return Parser{ std::move(tokens) }.parse();
}
