#pragma once

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
