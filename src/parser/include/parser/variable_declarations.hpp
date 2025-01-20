#pragma once

#include <vector>
#include "ast_node.hpp"
#include "variable_declaration.hpp"

class VariableDeclarations final : public AstNode {
private:
    Token const* m_var;
    std::vector<VariableDeclaration> m_declarations;

public:
    [[nodiscard]] explicit VariableDeclarations(
        std::same_as<Token const> auto& var_token,
        std::vector<VariableDeclaration> declarations
    )
        : m_var{ &var_token }, m_declarations{ std::move(declarations) } {
        if (m_declarations.empty()) {
            throw std::invalid_argument{ "VariableDeclarations must have at least one declaration." };
        }
    }

    [[nodiscard]] std::vector<VariableDeclaration> const& declarations() const {
        return m_declarations;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_var->source_location().join(m_declarations.back().source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "VariableDeclarations");
        context.print_children(m_declarations);
    }
};
