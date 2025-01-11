#pragma once

#include <lib2k/types.hpp>

class IntegerLiteral final {
private:
    i64 m_value;

public:
    [[nodiscard]] explicit IntegerLiteral(i64 const value)
        : m_value{ value } {}

    [[nodiscard]] i64 value() const {
        return m_value;
    }
};
