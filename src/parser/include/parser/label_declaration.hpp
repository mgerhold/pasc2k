#pragma once

#include <lib2k/types.hpp>

class LabelDeclaration final : public AstNode {
private:
    IntegerLiteral m_integer_literal;

public:
    [[nodiscard]] explicit LabelDeclaration(IntegerLiteral const integer_literal)
        : m_integer_literal{ integer_literal } {}

    [[nodiscard]] IntegerLiteral const& integer_literal() const {
        return m_integer_literal;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_integer_literal.source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "LabelDeclaration");
        context.print_children(m_integer_literal);
    }
};
