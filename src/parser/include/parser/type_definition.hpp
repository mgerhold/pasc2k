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

class TypeAlias final : public TypeDefinition {
private:
    Token const* m_referenced_type;

public:
    [[nodiscard]] explicit TypeAlias(Token const& identifier, Token const& referenced_type)
        : TypeDefinition{ identifier }, m_referenced_type{ &referenced_type } {}

    [[nodiscard]] Token const* referenced_type() const {
        return m_referenced_type;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return TypeDefinition::source_location().join(m_referenced_type->source_location());
    }

    void print(PrintContext& context) const override {
        print_ast_node(
            context,
            "TypeAlias",
            std::format("'{}'", identifier().lexeme()),
            std::format("'{}'", referenced_type()->lexeme())
        );
    }
};
