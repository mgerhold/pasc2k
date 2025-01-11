#include <algorithm>
#include <cassert>
#include <lexer/lexer.hpp>
#include <limits>
#include <ranges>

class Lexer final {
private:
    std::string_view m_path;
    std::string_view m_source;
    usize m_index = 0;
    std::vector<Token> m_tokens;
    bool m_encountered_token_separator = true;

public:
    [[nodiscard]] Lexer(std::string_view const path, std::string_view const source)
        : m_path{ path }, m_source{ source } {}

    void tokenize() {
        while (not is_at_end()) {
            if (not is_ascii(current())) {
                throw NonAsciiCharacter{ current_source_location() };
            }

            // 6.1.8 Comments.
            if (current() == '{' or (current() == '(' and peek() == '*')) {
                advance();
                while (not is_at_end()) {
                    if (current() == '}') {
                        break;
                    }
                    if (current() == '*' and peek() == ')') {
                        advance();
                        break;
                    }
                    advance();
                }
                if (is_at_end()) {
                    throw UnterminatedComment{ current_source_location() };
                }
                advance();
                m_encountered_token_separator = true;
                continue;
            }

            if (std::isspace(static_cast<unsigned char>(current()))) {
                advance();
                m_encountered_token_separator = true;
                continue;
            }

            // 6.1.2 Special symbols.
            switch (current_upper()) {
                case '+':
                    if (is_digit(peek())) {
                        number();
                    } else {
                        emit_token(TokenType::Plus);
                        advance();
                    }
                    break;
                case '-':
                    if (is_digit(peek())) {
                        number();
                    } else {
                        emit_token(TokenType::Minus);
                        advance();
                    }
                    break;
                case '*':
                    emit_token(TokenType::Asterisk);
                    advance();
                    break;
                case '/':
                    emit_token(TokenType::Slash);
                    advance();
                    break;
                case '=':
                    emit_token(TokenType::Equals);
                    advance();
                    break;
                case '<':
                    switch (peek()) {
                        case '>':
                            // <>
                            emit_token(TokenType::LessThanGreaterThan, 2);
                            advance();
                            break;
                        case '=':
                            // <=
                            emit_token(TokenType::LessThanEquals, 2);
                            advance();
                            break;
                        default:
                            emit_token(TokenType::LessThan);
                            break;
                    }
                    advance();
                    break;
                case '>':
                    if (peek() == '=') {
                        // >=
                        emit_token(TokenType::GreaterThanEquals, 2);
                        advance();
                    } else {
                        emit_token(TokenType::GreaterThan);
                    }
                    advance();
                    break;
                case '[':
                    emit_token(TokenType::LeftSquareBracket);
                    advance();
                    break;
                case ']':
                    emit_token(TokenType::RightSquareBracket);
                    advance();
                    break;
                case '.':
                    switch (peek()) {
                        case ')':
                            emit_token(TokenType::RightSquareBracket, 2);
                            advance();
                            break;
                        case '.':
                            emit_token(TokenType::DotDot, 2);
                            advance();
                            break;
                        default:
                            emit_token(TokenType::Dot);
                            break;
                    }
                    advance();
                    break;
                case ',':
                    emit_token(TokenType::Comma);
                    advance();
                    break;
                case ':':
                    if (peek() == '=') {
                        emit_token(TokenType::ColonEquals, 2);
                        advance();
                    } else {
                        emit_token(TokenType::Colon);
                    }
                    advance();
                    break;
                case ';':
                    emit_token(TokenType::Semicolon);
                    advance();
                    break;
                case '^':
                case '@':
                    emit_token(TokenType::UpArrow);
                    advance();
                    break;
                case '(':
                    if (peek() == '.') {
                        emit_token(TokenType::LeftSquareBracket, 2);
                        advance();
                    } else {
                        emit_token(TokenType::LeftParenthesis);
                    }
                    advance();
                    break;
                case ')':
                    emit_token(TokenType::RightParenthesis);
                    advance();
                    break;
                case '\'':
                    character_or_string();
                    break;
                default: {
                    // Number, word symbol, or identifier.
                    if (is_digit(current())) {
                        number();
                        continue;
                    }
                    if (not is_letter(current_upper())) {
                        throw UnexpectedCharacter{ current_source_location(), current(), "number, word symbol, or identifier" };
                    }
                    word_symbol_or_identifier();
                    break;
                }
            }
        }
        emit_token(TokenType::EndOfFile);
    }

    [[nodiscard]] std::vector<Token> take_tokens() & = delete;

    [[nodiscard]] std::vector<Token> take_tokens() && {
        return std::move(m_tokens);
    }

private:
    [[nodiscard]] bool is_at_end() const {
        return m_index >= m_source.size();
    }

    [[nodiscard]] char current() const {
        return is_at_end() ? '\0' : m_source.at(m_index);
    }

    [[nodiscard]] char current_upper() const {
        auto const c = current();
        if (c >= 'a' and c <= 'z') {
            return static_cast<char>('A' + (c - 'a'));
        }
        return c;
    }

    [[nodiscard]] char peek() const {
        if (m_index + 1 >= m_source.size()) {
            return '\0';
        }
        return m_source.at(m_index + 1);
    }

    [[nodiscard]] char peek_upper() const {
        auto const c = peek();
        if (c >= 'a' and c <= 'z') {
            return static_cast<char>('A' + (c - 'a'));
        }
        return c;
    }

    void advance() {
        if (not is_at_end()) {
            ++m_index;
        }
    }

    [[nodiscard]] static bool is_letter(char const c) {
        // 6.1.1
        // We expect the caller to have already converted the character to uppercase.
        return c >= 'A' and c <= 'Z';
    }

    [[nodiscard]] static bool is_digit(char const c) {
        // 6.1.1
        return c >= '0' and c <= '9';
    }

    [[nodiscard]] static bool is_ascii(char const c) {
        // If `char` is signed and therefore has a max value of 127,
        // GCC issues a warning because the comparison is always true.
        // Therefore, we suppress the warning here.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
        return c >= 0 and c <= 127;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    }

    void emit_token(TokenType const type, usize const length = 1) {
        emit_token(type, m_index, length);
    }

    void emit_token(TokenType const type, usize const start, usize const length) {
        auto token = Token{
            type,
            SourceLocation{ m_path, m_source, start, length }
        };
        if (not m_tokens.empty()) {
            auto const& previous_token = m_tokens.back();
            if ((previous_token.type() == TokenType::Identifier or is_word_symbol(previous_token.type())
                 or is_unsigned_integer_number(previous_token))
                and (token.type() == TokenType::Identifier or is_word_symbol(token.type()) or is_unsigned_integer_number(token))
                and not m_encountered_token_separator) {
                auto const source_location = SourceLocation{ m_path,
                                                             m_source,
                                                             previous_token.source_location().m_offset
                                                                 + previous_token.source_location().length(),
                                                             1 };
                throw UnexpectedCharacter{ source_location, source_location.text().front(), "token separator" };
            }
        }
        m_tokens.emplace_back(token);
        m_encountered_token_separator = false;
    }

    [[nodiscard]] SourceLocation current_source_location(usize const length = 1) const {
        return SourceLocation{ m_path, m_source, m_index, length };
    }

    void number() {
        // 6.1.5
        auto const start = m_index;

        // Optional sign.
        if (current() == '+' or current() == '-') {
            advance();
        }

        // Digit sequence.
        if (not is_digit(current())) {
            throw InternalCompilerError{ "Expected digit." };
        }
        advance();
        while (is_digit(current())) {
            advance();
        }

        if (current() != '.' and current_upper() != 'E') {
            // Integer.
            emit_token(TokenType::IntegerNumber, start, m_index - start);
            return;
        }

        // Real.

        if (current() == '.') {
            // Fractional part (digit sequence, optional).
            advance();
            if (not is_digit(current())) {
                throw UnexpectedCharacter{ current_source_location(), current(), "digit" };
            }
            while (is_digit(current())) {
                advance();
            }
        }

        if (current_upper() == 'E') {
            // Scale factor.
            advance();
            if (current() == '+' or current() == '-') {
                advance();
            }
            if (not is_digit(current())) {
                throw UnexpectedCharacter{ current_source_location(), current(), "digit" };
            }
            while (is_digit(current())) {
                advance();
            }
        }

        emit_token(TokenType::RealNumber, start, m_index - start);
    }

    void word_symbol_or_identifier() {
        auto const start = m_index;
        advance();
        while (is_letter(current_upper()) or is_digit(current())) {
            advance();
        }
        auto const lexeme = m_source.substr(start, m_index - start);

        // The following code could be optimized, but we don't care about performance for now.
        // TODO: Optimize!
        if (equals_case_insensitive(lexeme, "AND")) {
            emit_token(TokenType::And, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "ARRAY")) {
            emit_token(TokenType::Array, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "BEGIN")) {
            emit_token(TokenType::Begin, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "CASE")) {
            emit_token(TokenType::Case, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "CONST")) {
            emit_token(TokenType::Const, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "DIV")) {
            emit_token(TokenType::Div, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "DO")) {
            emit_token(TokenType::Do, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "DOWNTO")) {
            emit_token(TokenType::DownTo, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "ELSE")) {
            emit_token(TokenType::Else, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "END")) {
            emit_token(TokenType::End, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "FILE")) {
            emit_token(TokenType::File, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "FOR")) {
            emit_token(TokenType::For, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "FUNCTION")) {
            emit_token(TokenType::Function, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "GOTO")) {
            emit_token(TokenType::Goto, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "IF")) {
            emit_token(TokenType::If, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "IN")) {
            emit_token(TokenType::In, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "LABEL")) {
            emit_token(TokenType::Label, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "MOD")) {
            emit_token(TokenType::Mod, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "NIL")) {
            emit_token(TokenType::Nil, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "NOT")) {
            emit_token(TokenType::Not, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "OF")) {
            emit_token(TokenType::Of, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "OR")) {
            emit_token(TokenType::Or, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "PACKED")) {
            emit_token(TokenType::Packed, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "PROCEDURE")) {
            emit_token(TokenType::Procedure, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "PROGRAM")) {
            emit_token(TokenType::Program, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "RECORD")) {
            emit_token(TokenType::Record, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "REPEAT")) {
            emit_token(TokenType::Repeat, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "SET")) {
            emit_token(TokenType::Set, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "THEN")) {
            emit_token(TokenType::Then, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "TO")) {
            emit_token(TokenType::To, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "TYPE")) {
            emit_token(TokenType::Type, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "UNTIL")) {
            emit_token(TokenType::Until, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "VAR")) {
            emit_token(TokenType::Var, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "WHILE")) {
            emit_token(TokenType::While, start, m_index - start);
        } else if (equals_case_insensitive(lexeme, "WITH")) {
            emit_token(TokenType::With, start, m_index - start);
        } else {
            emit_token(TokenType::Identifier, start, m_index - start);
        }
    }

    [[nodiscard]] static bool is_valid_string_character(char const current) {
        return current >= 32 and current <= 126;
    }

    void character_or_string() {
        auto const start = m_index;
        advance();
        auto num_apostrophe_images = usize{ 0 };
        while (true) {
            if (current() == '\'') {
                if (peek() == '\'') {
                    ++num_apostrophe_images;
                    advance();
                    advance();
                    continue;
                }
                break;
            }
            if (not is_valid_string_character(current())) {
                throw UnexpectedCharacter{ current_source_location(), current(), "string character" };
            }
            advance();
        }
        if (current() != '\'') {
            throw UnterminatedCharacterString{ current_source_location() };
        }
        advance();
        auto const num_characters = m_index - start - 2 - num_apostrophe_images;
        if (num_characters == 1) {
            emit_token(TokenType::Char, start, m_index - start);
        } else {
            assert(num_characters > 1);
            emit_token(TokenType::String, start, m_index - start);
        }
    }

    [[nodiscard]] static bool equals_case_insensitive(std::string_view const lhs, std::string_view const rhs) {
        if (lhs.length() != rhs.length()) {
            return false;
        }
        return std::ranges::all_of(std::views::zip(lhs, rhs), [](auto const& pair) {
            auto const [left, right] = pair;
            auto const left_upper = static_cast<char>(std::toupper(left));
            auto const right_upper = static_cast<char>(std::toupper(right));
            return left_upper == right_upper;
        });
    }

    [[nodiscard]] static bool is_word_symbol(TokenType const type) {
        switch (type) {
            case TokenType::Plus:
            case TokenType::Minus:
            case TokenType::Asterisk:
            case TokenType::Slash:
            case TokenType::Equals:
            case TokenType::LessThan:
            case TokenType::GreaterThan:
            case TokenType::LeftSquareBracket:
            case TokenType::RightSquareBracket:
            case TokenType::Dot:
            case TokenType::Comma:
            case TokenType::Colon:
            case TokenType::Semicolon:
            case TokenType::UpArrow:
            case TokenType::LeftParenthesis:
            case TokenType::RightParenthesis:
            case TokenType::LessThanGreaterThan:
            case TokenType::LessThanEquals:
            case TokenType::GreaterThanEquals:
            case TokenType::ColonEquals:
            case TokenType::DotDot:
                return false;
            case TokenType::And:
            case TokenType::Array:
            case TokenType::Begin:
            case TokenType::Case:
            case TokenType::Const:
            case TokenType::Div:
            case TokenType::Do:
            case TokenType::DownTo:
            case TokenType::Else:
            case TokenType::End:
            case TokenType::File:
            case TokenType::For:
            case TokenType::Function:
            case TokenType::Goto:
            case TokenType::If:
            case TokenType::In:
            case TokenType::Label:
            case TokenType::Mod:
            case TokenType::Nil:
            case TokenType::Not:
            case TokenType::Of:
            case TokenType::Or:
            case TokenType::Packed:
            case TokenType::Procedure:
            case TokenType::Program:
            case TokenType::Record:
            case TokenType::Repeat:
            case TokenType::Set:
            case TokenType::Then:
            case TokenType::To:
            case TokenType::Type:
            case TokenType::Until:
            case TokenType::Var:
            case TokenType::While:
            case TokenType::With:
                return true;
            case TokenType::Identifier:
            case TokenType::Directive:
            case TokenType::IntegerNumber:
            case TokenType::RealNumber:
            case TokenType::Char:
            case TokenType::String:
            case TokenType::EndOfFile:
                return false;
        }
        throw InternalCompilerError{ "Unknown TokenType" };
    }

    [[nodiscard]] static bool is_unsigned_integer_number(Token const& token) {
        return token.type() == TokenType::IntegerNumber and token.lexeme().front() != '+'
               and token.lexeme().front() != '-';
    }
};

[[nodiscard]] std::vector<Token> tokenize(std::string_view const path, std::string_view const source) {
    auto lexer = Lexer{ path, source };
    lexer.tokenize();
    return std::move(lexer).take_tokens();
}
