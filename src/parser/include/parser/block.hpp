#pragma once

#include <lexer/token.hpp>
#include <optional>
#include "ast_node.hpp"
#include "constant_definitions.hpp"
#include "label_declarations.hpp"

class Block final : public AstNode {
private:
    std::optional<LabelDeclarations> m_label_declarations;
    std::optional<ConstantDefinitions> m_constant_definitions;

public:
    [[nodiscard]] explicit Block(
        std::optional<LabelDeclarations> label_declarations,
        std::optional<ConstantDefinitions> constant_definitions
    )
        : m_label_declarations{ std::move(label_declarations) },
          m_constant_definitions{ std::move(constant_definitions) } {}

public:
    [[nodiscard]] std::optional<LabelDeclarations> const& label_declarations() const {
        return m_label_declarations;
    }

    [[nodiscard]] std::optional<ConstantDefinitions> const& constant_definitions() const {
        return m_constant_definitions;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        if (m_label_declarations.has_value()) {
            return m_label_declarations.value().source_location();
        }
        /*if (m_constant_definitions.has_value()) {
            return m_constant_definitions.value().source_location();
        }*/
        throw InternalCompilerError{ "Block has no source location." };
    }

    void print(PrintContext& context) const override {
        print_ast_node(context, "Block");
        context.begin_children(false);
        if (m_label_declarations.has_value()) {
            m_label_declarations.value().print(context);
        }
        if (m_constant_definitions.has_value()) {
            m_constant_definitions.value().print(context);
        }
        context.end_children();
    }
};
