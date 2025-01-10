#pragma once

#include <lib2k/types.hpp>
#include <stdexcept>
#include <string_view>
#include <vector>

class InternalCompilerError final : public std::runtime_error {
public:
    [[nodiscard]] explicit InternalCompilerError(std::string const& message)
        : runtime_error{ message } {}
};
