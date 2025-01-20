#pragma once

#include <memory>
#include "ast_node.hpp"
#include "identifier_list.hpp"
#include "type_definition.hpp"

class VariableDeclaration final : public AstNode {
private:
    IdentifierList m_identifiers;
    std::unique_ptr<Type> m_type;

public:
    [[nodiscard]] explicit VariableDeclaration(IdentifierList&& identifiers, std::unique_ptr<Type>&& type)
        : m_identifiers{ std::move(identifiers) }, m_type{ std::move(type) } {}

    [[nodiscard]] IdentifierList const& identifiers() const {
        return m_identifiers;
    }

    [[nodiscard]] Type const& type() const {
        return *m_type;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifiers.source_location().join(m_type->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "VariableDeclaration");
        context.print_children(m_identifiers, *m_type);
    }
};
