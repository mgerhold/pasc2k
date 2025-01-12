#pragma once

#include <lexer/token.hpp>
#include "ast_node.hpp"

class Identifier final : public AstNode {
private:
    Token const* m_token;

public:
    [[nodiscard]] explicit Identifier(Token const& token)
        : m_token{ &token } {}

    [[nodiscard]] SourceLocation source_location() const override {
        return m_token->source_location();
    }

    void print(PrintContext& context) const override {
        print_ast_node(context, "Identifier", m_token->lexeme());
    }
};
