#pragma once

#include <lexer/token.hpp>
#include "ast_node.hpp"

class Identifier final : public AstNode {
private:
    Token const* m_token;

public:
    [[nodiscard]] explicit Identifier(std::same_as<Token const> auto& token)
        : m_token{ &token } {
        if (token.type() != TokenType::Identifier) {
            throw InternalCompilerError{ "Expected identifier token." };
        }
    }

    [[nodiscard]] Token const& token() const {
        return *m_token;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_token->source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "Identifier", m_token->lexeme());
    }
};
