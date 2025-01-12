#pragma once

#include <lexer/token.hpp>
#include <vector>
#include "ast_node.hpp"
#include "constant_definition.hpp"

class ConstantDefinitions final : public AstNode {
private:
    Token const* m_const_token;
    std::vector<ConstantDefinition> m_constant_definitions;

public:
    [[nodiscard]] explicit ConstantDefinitions(Token const& const_token, std::vector<ConstantDefinition> constant_definitions)
        : m_const_token{ &const_token }, m_constant_definitions{ std::move(constant_definitions) } {
        if (m_constant_definitions.empty()) {
            throw InternalCompilerError{ "Empty constant definitions." };
        }
    }

    [[nodiscard]] std::vector<ConstantDefinition> const& constant_definitions() const {
        return m_constant_definitions;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_const_token->source_location().join(m_constant_definitions.back().source_location());
    }

    void print(PrintContext& context) const override {
        using std::views::transform, std::ranges::to;
        context.print(*this, "ConstantDefinitions");
        // clang-format off
        context.print_children(
            m_constant_definitions
            | transform([](auto const& e) { return static_cast<AstNode const*>(&e); })
            | to<std::vector>()
        );
        // clang-format on
    }
};
