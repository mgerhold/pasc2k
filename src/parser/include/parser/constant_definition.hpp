#pragma once

#include <lexer/token.hpp>
#include <lib2k/types.hpp>
#include <memory>
#include <tl/optional.hpp>
#include <variant>
#include "ast_node.hpp"
#include "literals.hpp"

class ConstantDefinition : public AstNode {
protected:
    Token const* m_identifier;

protected:
    [[nodiscard]] explicit ConstantDefinition(Token const& identifier)
        : m_identifier{ &identifier } {}

public:
    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifier->source_location();
    }
};

class IntegerConstant final : public ConstantDefinition {
private:
    tl::optional<Token const&> m_sign;
    IntegerLiteral m_integer_literal;

public:
    [[nodiscard]] explicit IntegerConstant(
        Token const& identifier,
        tl::optional<Token const&> const& sign,
        IntegerLiteral const& integer_literal
    )
        : ConstantDefinition{ identifier }, m_sign{ sign }, m_integer_literal{ integer_literal } {}

    [[nodiscard]] IntegerLiteral const& integer_literal() const {
        return m_integer_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return ConstantDefinition::source_location().join(integer_literal().source_location());
    }

    void print(PrintContext& context) const override {
        if (m_sign.has_value()) {
            print_ast_node(context, "IntegerConstant", m_sign.value().lexeme());
        } else {
            print_ast_node(context, "IntegerConstant");
        }
        context.begin_children(true);
        integer_literal().print(context);
        context.end_children();
    }
};

class RealConstant final : public ConstantDefinition {
private:
    tl::optional<Token const&> m_sign;
    RealLiteral m_real_literal;

public:
    [[nodiscard]] explicit RealConstant(
        Token const& identifier,
        tl::optional<Token const&> const& sign,
        RealLiteral const& real_literal
    )
        : ConstantDefinition{ identifier }, m_sign{ sign }, m_real_literal{ real_literal } {}

    [[nodiscard]] RealLiteral const& real_literal() const {
        return m_real_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return ConstantDefinition::source_location().join(real_literal().source_location());
    }

    void print(PrintContext& context) const override {
        if (m_sign.has_value()) {
            print_ast_node(context, "RealConstant", m_sign.value().lexeme());
        } else {
            print_ast_node(context, "RealConstant");
        }
        context.begin_children(true);
        real_literal().print(context);
        context.end_children();
    }
};

class CharConstant final : public ConstantDefinition {
private:
    CharLiteral m_char_literal;

public:
    [[nodiscard]] explicit CharConstant(Token const& identifier, CharLiteral const& char_literal)
        : ConstantDefinition{ identifier }, m_char_literal{ char_literal } {}

    [[nodiscard]] CharLiteral const& char_literal() const {
        return m_char_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return ConstantDefinition::source_location().join(char_literal().source_location());
    }

    void print(PrintContext& context) const override {
        print_ast_node(context, "CharConstant");
        context.begin_children(true);
        char_literal().print(context);
        context.end_children();
    }
};

class StringConstant final : public ConstantDefinition {
private:
    StringLiteral m_string_literal;

public:
    [[nodiscard]] explicit StringConstant(Token const& identifier, StringLiteral const& string_literal)
        : ConstantDefinition{ identifier }, m_string_literal{ string_literal } {}

    [[nodiscard]] StringLiteral const& string_literal() const {
        return m_string_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return ConstantDefinition::source_location().join(string_literal().source_location());
    }

    void print(PrintContext& context) const override {
        print_ast_node(context, "StringConstant");
        context.begin_children(true);
        string_literal().print(context);
        context.end_children();
    }
};

class ConstantReference final : public ConstantDefinition {
private:
    tl::optional<Token const&> m_sign;
    Token const* m_referenced_constant;

public:
    [[nodiscard]] explicit ConstantReference(
        Token const& identifier,
        tl::optional<Token const&> const& sign,
        Token const& referenced_constant
    )
        : ConstantDefinition{ identifier }, m_sign{ sign }, m_referenced_constant{ &referenced_constant } {}

    [[nodiscard]] tl::optional<Token const&> const& sign() const {
        return m_sign;
    }

    [[nodiscard]] Token const& referenced_constant() const {
        return *m_referenced_constant;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return ConstantDefinition::source_location().join(referenced_constant().source_location());
    }

    void print(PrintContext& context) const override {
        if (m_sign.has_value()) {
            print_ast_node(context, "ConstantReference", m_sign.value().lexeme(), referenced_constant().lexeme());
        } else {
            print_ast_node(context, "ConstantReference", referenced_constant().lexeme());
        }
    }
};
