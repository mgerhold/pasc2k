#pragma once

#include <lib2k/types.hpp>
#include "integer_literal.hpp"

class LabelDeclaration final {
private:
    i64 m_value;

public:
    [[nodiscard]] explicit LabelDeclaration(i64 const value)
        : m_value{ value } {}

    [[nodiscard]] i64 value() const {
        return m_value;
    }
};
