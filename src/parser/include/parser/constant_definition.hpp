#pragma once

#include <lexer/token.hpp>
#include <lib2k/types.hpp>
#include <variant>

struct ConstantReference final {
    enum class Sign {
        Positive,
        Negative,
        None,
    };

    Sign sign;
    Token const* identifier;

    [[nodiscard]] ConstantReference(Sign const sign, Token const& identifier)
        : sign{ sign }, identifier{ &identifier } {}
};

class ConstantDefinition final {
public:
    using Constant = std::variant<i64, double, char, std::string, ConstantReference>;

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
