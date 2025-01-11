#pragma once

#include <lexer/token.hpp>
#include <memory>
#include <vector>
#include "block.hpp"
#include <print>

class Ast final {
private:
    std::vector<Token> m_tokens;
    Block m_block;

public:
    [[nodiscard]] explicit Ast(std::vector<Token>&& tokens, Block block)
        : m_tokens{ std::move(tokens) }, m_block{ std::move(block) } {}

    [[nodiscard]] Block const& block() const {
        return m_block;
    }

    void print() const {
        auto context = AstNode::PrintContext{};
        m_block.print(context);
    }
};
