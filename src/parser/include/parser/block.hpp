#pragma once

#include <lexer/token.hpp>
#include <tl/optional.hpp>
#include "ast_node.hpp"
#include "constant_definitions.hpp"
#include "label_declarations.hpp"
#include "type_definitions.hpp"
#include "variable_declarations.hpp"

class Block final : public AstNode {
private:
    tl::optional<LabelDeclarations> m_label_declarations;
    tl::optional<ConstantDefinitions> m_constant_definitions;
    tl::optional<TypeDefinitions> m_type_definitions;
    tl::optional<VariableDeclarations> m_variable_declarations;

public:
    [[nodiscard]] explicit Block(
        tl::optional<LabelDeclarations>&& label_declarations,
        tl::optional<ConstantDefinitions>&& constant_definitions,
        tl::optional<TypeDefinitions>&& type_definitions,
        tl::optional<VariableDeclarations>&& variable_declarations
    )
        : m_label_declarations{ std::move(label_declarations) },
          m_constant_definitions{ std::move(constant_definitions) },
          m_type_definitions{ std::move(type_definitions) },
          m_variable_declarations{ std::move(variable_declarations) } {}

public:
    [[nodiscard]] tl::optional<LabelDeclarations const&> label_declarations() const {
        return m_label_declarations.map([](auto const& value) -> LabelDeclarations const& { return value; });
    }

    [[nodiscard]] tl::optional<ConstantDefinitions const&> constant_definitions() const {
        return m_constant_definitions.map([](auto const& value) -> ConstantDefinitions const& { return value; });
    }

    [[nodiscard]] tl::optional<TypeDefinitions const&> type_definitions() const {
        return m_type_definitions.map([](auto const& value) -> TypeDefinitions const& { return value; });
    }

    [[nodiscard]] tl::optional<VariableDeclarations const&> variable_declarations() const {
        return m_variable_declarations.map([](auto const& value) -> VariableDeclarations const& { return value; });
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
        if (m_variable_declarations.has_value()) {
            if (not source_location.has_value()) {
                source_location = m_variable_declarations.value().source_location();
            } else {
                source_location = source_location.value().join(m_variable_declarations.value().source_location());
            }
        }

        if (source_location.has_value()) {
            return source_location.value();
        }
        throw InternalCompilerError{ "Block has no source location." };
    }

    void print(PrintContext& context) const override {
        context.print(*this, "Block");
        context.print_children(m_label_declarations, m_constant_definitions, m_type_definitions, m_variable_declarations);
    }
};
