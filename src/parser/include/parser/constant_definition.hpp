#pragma once

#include <lexer/token.hpp>
#include <lib2k/types.hpp>
#include <memory>
#include <tl/optional.hpp>
#include <variant>
#include "ast_node.hpp"
#include "identifier.hpp"
#include "literals.hpp"

class Constant : public AstNode {};

class ConstantDefinition final : public AstNode {
protected:
    Identifier m_identifier;
    std::unique_ptr<Constant> m_constant;

public:
    [[nodiscard]] explicit ConstantDefinition(Identifier const& identifier, std::unique_ptr<Constant> constant)
        : m_identifier{ identifier }, m_constant{ std::move(constant) } {}

public:
    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifier.source_location().join(m_constant->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "ConstantDefinition");
        context.print_children(m_identifier, *m_constant);
    }
};

class IntegerConstant final : public Constant {
private:
    tl::optional<Token const&> m_sign;
    IntegerLiteral m_integer_literal;

public:
    template<std::same_as<Token const> T>
    [[nodiscard]] explicit IntegerConstant(tl::optional<T&> const& sign, IntegerLiteral const& integer_literal)
        : m_sign{ sign }, m_integer_literal{ integer_literal } {}

    [[nodiscard]] IntegerLiteral const& integer_literal() const {
        return m_integer_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        if (m_sign.has_value()) {
            return m_sign.value().source_location().join(integer_literal().source_location());
        }
        return integer_literal().source_location();
    }

    void print(PrintContext& context) const override {
        if (m_sign.has_value()) {
            context.print(*this, "IntegerConstant", m_sign.value().lexeme());
        } else {
            context.print(*this, "IntegerConstant");
        }
        context.print_children(integer_literal());
    }
};

class RealConstant final : public Constant {
private:
    tl::optional<Token const&> m_sign;
    RealLiteral m_real_literal;

public:
    template<std::same_as<Token const> T>
    [[nodiscard]] explicit RealConstant(tl::optional<T&> const& sign, RealLiteral const& real_literal)
        : m_sign{ sign }, m_real_literal{ real_literal } {}

    [[nodiscard]] RealLiteral const& real_literal() const {
        return m_real_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        if (m_sign.has_value()) {
            return m_sign.value().source_location().join(real_literal().source_location());
        }
        return real_literal().source_location();
    }

    void print(PrintContext& context) const override {
        if (m_sign.has_value()) {
            context.print(*this, "RealConstant", m_sign.value().lexeme());
        } else {
            context.print(*this, "RealConstant");
        }
        context.print_children(real_literal());
    }
};

class CharConstant final : public Constant {
private:
    CharLiteral m_char_literal;

public:
    [[nodiscard]] explicit CharConstant(CharLiteral const& char_literal)
        : m_char_literal{ char_literal } {}

    [[nodiscard]] CharLiteral const& char_literal() const {
        return m_char_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return char_literal().source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "CharConstant");
        context.print_children(char_literal());
    }
};

class StringConstant final : public Constant {
private:
    StringLiteral m_string_literal;

public:
    [[nodiscard]] explicit StringConstant(StringLiteral const& string_literal)
        : m_string_literal{ string_literal } {}

    [[nodiscard]] StringLiteral const& string_literal() const {
        return m_string_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return string_literal().source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "StringConstant");
        context.print_children(string_literal());
    }
};

class ConstantReference final : public Constant {
private:
    tl::optional<Token const&> m_sign;
    Token const* m_referenced_constant;

public:
    template<std::same_as<Token const> T>
    [[nodiscard]] explicit ConstantReference(
        tl::optional<T&> const& sign,
        std::same_as<Token const> auto& referenced_constant
    ) : m_sign{ sign }, m_referenced_constant{ &referenced_constant } {}

    [[nodiscard]] tl::optional<Token const&> const& sign() const {
        return m_sign;
    }

    [[nodiscard]] Token const& referenced_constant() const {
        return *m_referenced_constant;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        if (m_sign.has_value()) {
            return m_sign.value().source_location().join(referenced_constant().source_location());
        }
        return referenced_constant().source_location();
    }

    void print(PrintContext& context) const override {
        if (m_sign.has_value()) {
            context.print(*this, "ConstantReference", m_sign.value().lexeme(), referenced_constant().lexeme());
        } else {
            context.print(*this, "ConstantReference", referenced_constant().lexeme());
        }
    }
};
