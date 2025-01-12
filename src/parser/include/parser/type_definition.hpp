#pragma once

#include <lexer/token.hpp>
#include <memory>
#include "ast_node.hpp"

class TypeDefinition : public AstNode {
private:
    Token const* m_identifier;

protected:
    [[nodiscard]] explicit TypeDefinition(Token const& identifier)
        : m_identifier{ &identifier } {}

public:
    [[nodiscard]] Token const& identifier() const {
        return *m_identifier;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifier->source_location();
    }
};

class TypeAliasDefinition final : public TypeDefinition {
private:
    Token const* m_referenced_type;

public:
    [[nodiscard]] explicit TypeAliasDefinition(Token const& identifier, Token const& referenced_type)
        : TypeDefinition{ identifier }, m_referenced_type{ &referenced_type } {}

    [[nodiscard]] Token const* referenced_type() const {
        return m_referenced_type;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return TypeDefinition::source_location().join(m_referenced_type->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "TypeAliasDefinition", identifier().lexeme(), m_referenced_type->lexeme());
    }
};

class EnumeratedTypeDefinition final : public TypeDefinition {
private:
    Token const* m_left_parenthesis;
    std::vector<Token const*> m_identifiers;
    Token const* m_right_parenthesis;

public:
    [[nodiscard]] explicit EnumeratedTypeDefinition(
        Token const& identifier,
        Token const& left_parenthesis,
        std::vector<Token const*> identifiers,
        Token const& right_parenthesis
    )
        : TypeDefinition{ identifier },
          m_left_parenthesis{ &left_parenthesis },
          m_identifiers{ std::move(identifiers) },
          m_right_parenthesis{ &right_parenthesis } {
        if (m_identifiers.empty()) {
            throw InternalCompilerError{ "EnumeratedTypeDefinition must have at least one identifier." };
        }
    }

    [[nodiscard]] std::vector<Token const*> const& identifiers() const {
        return m_identifiers;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_left_parenthesis->source_location().join(m_right_parenthesis->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(
            *this,
            "EnumeratedTypeDefinition",
            identifier().lexeme(),
            std::format(
                "({})",
                c2k::join(
                    m_identifiers
                        | std::views::transform([](auto const* token) { return std::format("'{}'", token->lexeme()); }),
                    ", "
                )
            )
        );
    }
};
