#pragma once

#include <vector>
#include "label_declaration.hpp"
#include <lexer/token.hpp>

class Block final {
private:
    std::vector<Token> m_tokens;
    std::vector<LabelDeclaration> m_label_declarations;

public:
    Block(std::vector<Token> tokens, std::vector<LabelDeclaration> label_declarations)
        : m_tokens{ std::move(tokens) }, m_label_declarations{ std::move(label_declarations) } {}
};
