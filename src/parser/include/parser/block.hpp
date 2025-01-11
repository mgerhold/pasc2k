#pragma once

#include <lexer/token.hpp>
#include <vector>
#include "constant_definition.hpp"
#include "label_declaration.hpp"

class Block final {
private:
    std::vector<Token> m_tokens;
    std::vector<LabelDeclaration> m_label_declarations;
    std::vector<ConstantDefinition> m_constant_definitions;

public:
    [[nodiscard]] explicit Block(
        std::vector<Token> tokens,
        std::vector<LabelDeclaration> label_declarations,
        std::vector<ConstantDefinition> constant_definitions
    )
        : m_tokens{ std::move(tokens) },
          m_label_declarations{ std::move(label_declarations) },
          m_constant_definitions{ std::move(constant_definitions) } {}
};
