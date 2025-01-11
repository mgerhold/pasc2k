#pragma once

#include <format>
#include "source_location.hpp"
#include "token_type.hpp"

class Token final {
private:
    TokenType m_type;
    SourceLocation m_source_location;

public:
    [[nodiscard]] constexpr Token(TokenType const type, SourceLocation const& source_location)
        : m_type{ type }, m_source_location{ source_location } {}

    [[nodiscard]] TokenType type() const {
        return m_type;
    }

    [[nodiscard]] SourceLocation const& source_location() const {
        return m_source_location;
    }

    [[nodiscard]] std::string_view lexeme() const {
        return m_source_location.text();
    }
};

template<>
struct std::formatter<Token> : std::formatter<std::string> {
    auto format(Token const& token, std::format_context& context) const {
        switch (token.type()) {
            case TokenType::Identifier:
            case TokenType::IntegerNumber:
            case TokenType::RealNumber:
            case TokenType::Char:
            case TokenType::String:
                return std::formatter<std::string>::format(std::format("{}({})", token.type(), token.lexeme()), context);
            default:
                return std::formatter<std::string>::format(std::format("{}", token.type()), context);
        }
    }
};
