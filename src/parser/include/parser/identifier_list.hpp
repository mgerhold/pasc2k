#pragma once

#include <ranges>
#include <vector>
#include "ast_node.hpp"
#include "identifier.hpp"

class IdentifierList final : public AstNode {
private:
    std::vector<Identifier> m_identifiers;

public:
    [[nodiscard]] explicit IdentifierList(std::vector<Identifier> identifiers)
        : m_identifiers{ std::move(identifiers) } {
        if (m_identifiers.empty()) {
            throw InternalCompilerError{ "IdentifierList must have at least one identifier." };
        }
    }

    [[nodiscard]] std::vector<Identifier> const& identifiers() const {
        return m_identifiers;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifiers.front().source_location().join(m_identifiers.back().source_location());
    }

    void print(PrintContext& context) const override {
        using std::views::transform, std::ranges::to;
        context.print(*this, "IdentifierList");
        // clang-format off
        context.print_children(
            m_identifiers
                | transform([](auto const& identifier) { return static_cast<AstNode const*>(&identifier); })
                | to<std::vector>()
        );
        // clang-format on
    }
};
