#pragma once

#include <lexer/token.hpp>
#include <vector>
#include "ast_node.hpp"
#include "label_declaration.hpp"

class LabelDeclarations final : public AstNode {
private:
    Token const* m_label_token;
    std::vector<LabelDeclaration> m_label_declarations;

public:
    [[nodiscard]] explicit LabelDeclarations(
        std::same_as<Token const> auto& label_token,
        std::vector<LabelDeclaration> label_declarations
    ) : m_label_token{ &label_token }, m_label_declarations{ std::move(label_declarations) } {
        if (m_label_declarations.empty()) {
            throw InternalCompilerError{ "Empty label declarations." };
        }
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_label_token->source_location().join(m_label_declarations.back().source_location());
    }

    [[nodiscard]] std::vector<LabelDeclaration> const& label_declarations() const {
        return m_label_declarations;
    }

    void print(PrintContext& context) const override {
        using std::views::transform, std::ranges::to;
        context.print(*this, "LabelDeclarations");
        // clang-format off
        context.print_children(
            m_label_declarations
            | transform([](auto const& e) { return static_cast<AstNode const*>(&e); })
            | to<std::vector>()
        );
        // clang-format on
    }
};
