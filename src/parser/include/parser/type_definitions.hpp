#pragma once

#include <lexer/token.hpp>
#include <memory>
#include <vector>
#include "ast_node.hpp"
#include "type_definition.hpp"

class TypeDefinitions final : public AstNode {
private:
    Token const* m_type_token;
    std::vector<std::unique_ptr<TypeDefinition>> m_type_definitions;

public:
    [[nodiscard]] explicit TypeDefinitions(
        Token const& type_token,
        std::vector<std::unique_ptr<TypeDefinition>> type_definitions
    )
        : m_type_token{ &type_token }, m_type_definitions{ std::move(type_definitions) } {
        if (m_type_definitions.empty()) {
            throw InternalCompilerError{ "Empty type definitions." };
        }
    }

    [[nodiscard]] std::vector<std::unique_ptr<TypeDefinition>> const& type_definitions() const {
        return m_type_definitions;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_type_token->source_location().join(m_type_definitions.back()->source_location());
    }

    void print(PrintContext& context) const override {
        print_ast_node(context, "TypeDefinitions");
        context.begin_children(m_type_definitions.size() == 1);
        for (auto const& definition : m_type_definitions) {
            definition->print(context);
        }
        context.end_children();
    }
};
