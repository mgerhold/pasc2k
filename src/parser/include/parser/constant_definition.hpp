#pragma once

#include <lexer/token.hpp>
#include <lib2k/types.hpp>
#include <variant>

class ConstantDefinition final {
public:
    using Constant = std::variant<i64, double, char, std::string>;

private:
    Token const* m_identifier;
    Constant m_constant;

public:
    [[nodiscard]] explicit ConstantDefinition(Token const& identifier, Constant const& constant)
        : m_identifier{ &identifier }, m_constant{ constant } {}

    [[nodiscard]] Token const& identifier() const {
        return *m_identifier;
    }

    [[nodiscard]] Constant const& constant() const {
        return m_constant;
    }
};
