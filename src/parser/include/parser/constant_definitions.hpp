#pragma once

#include <lexer/token.hpp>
#include <vector>
#include "ast_node.hpp"
#include "constant_definition.hpp"

class ConstantDefinitions final : public AstNode {
private:
    Token const* m_const_token;
    std::vector<std::unique_ptr<ConstantDefinition>> m_constant_definitions;

public:
    [[nodiscard]] explicit ConstantDefinitions(
        Token const& const_token,
        std::vector<std::unique_ptr<ConstantDefinition>> constant_definitions
    )
        : m_const_token{ &const_token }, m_constant_definitions{ std::move(constant_definitions) } {
        if (m_constant_definitions.empty()) {
            throw InternalCompilerError{ "Empty constant definitions." };
        }
    }

    [[nodiscard]] std::vector<std::unique_ptr<ConstantDefinition>> const& constant_definitions() const {
        return m_constant_definitions;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_const_token->source_location().join(m_constant_definitions.back()->source_location());
    }

    void print(PrintContext& context) const override {
        print_ast_node(context, "ConstantDefinitions");
        context.begin_children(m_constant_definitions.size() == 1);
        for (auto const& definition : m_constant_definitions) {
            definition->print(context);
        }
        context.end_children();
    }
};
