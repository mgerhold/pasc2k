#pragma once

#include <algorithm>
#include <lib2k/types.hpp>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <vector>

class InternalCompilerError final : public std::runtime_error {
public:
    [[nodiscard]] explicit InternalCompilerError(std::string const& message)
        : runtime_error{ message } {}
};

[[nodiscard]] inline bool equals_case_insensitive(std::string_view const lhs, std::string_view const rhs) {
    if (lhs.length() != rhs.length()) {
        return false;
    }
    return std::ranges::all_of(std::views::zip(lhs, rhs), [](auto const& pair) {
        auto const [left, right] = pair;
        auto const left_upper = static_cast<char>(std::toupper(left));
        auto const right_upper = static_cast<char>(std::toupper(right));
        return left_upper == right_upper;
    });
}
