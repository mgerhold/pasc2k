#pragma once

#include <lexer/token.hpp>
#include <optional>
#include "ast_node.hpp"
#include "constant_definitions.hpp"
#include "label_declarations.hpp"
#include "type_definitions.hpp"

class Block final : public AstNode {
private:
    std::optional<LabelDeclarations> m_label_declarations;
    std::optional<ConstantDefinitions> m_constant_definitions;
    std::optional<TypeDefinitions> m_type_definitions;

public:
    [[nodiscard]] explicit Block(
        std::optional<LabelDeclarations> label_declarations,
        std::optional<ConstantDefinitions> constant_definitions,
        std::optional<TypeDefinitions> type_definitions
    )
        : m_label_declarations{ std::move(label_declarations) },
          m_constant_definitions{ std::move(constant_definitions) },
          m_type_definitions{ std::move(type_definitions) } {}

public:
    [[nodiscard]] std::optional<LabelDeclarations> const& label_declarations() const {
        return m_label_declarations;
    }

    [[nodiscard]] std::optional<ConstantDefinitions> const& constant_definitions() const {
        return m_constant_definitions;
    }

    [[nodiscard]] std::optional<TypeDefinitions> const& type_definitions() const {
        return m_type_definitions;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        auto source_location = std::optional<SourceLocation>{};
        if (m_label_declarations.has_value()) {
            if (not source_location.has_value()) {
                source_location = m_label_declarations.value().source_location();
            } else {
                source_location = source_location.value().join(m_label_declarations.value().source_location());
            }
        }
        if (m_constant_definitions.has_value()) {
            if (not source_location.has_value()) {
                source_location = m_constant_definitions.value().source_location();
            } else {
                source_location = source_location.value().join(m_constant_definitions.value().source_location());
            }
        }
        if (m_type_definitions.has_value()) {
            if (not source_location.has_value()) {
                source_location = m_type_definitions.value().source_location();
            } else {
                source_location = source_location.value().join(m_type_definitions.value().source_location());
            }
        }

        if (source_location.has_value()) {
            return source_location.value();
        }
        throw InternalCompilerError{ "Block has no source location." };
    }

    void print(PrintContext& context) const override {
        context.print(*this, "Block");
        /*auto children = std::vector<AstNode const*>{};
        if (m_label_declarations.has_value()) {
            children.push_back(&m_label_declarations.value());
        }
        if (m_constant_definitions.has_value()) {
            children.push_back(&m_constant_definitions.value());
        }
        if (m_type_definitions.has_value()) {
            children.push_back(&m_type_definitions.value());
        }
        context.print_children(children);*/
        context.print_children(m_label_declarations, m_constant_definitions, m_type_definitions);
    }
};
