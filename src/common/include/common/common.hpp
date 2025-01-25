#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <lib2k/types.hpp>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <vector>

[[nodiscard]] inline bool is_ascii(char const c) {
    // If `char` is signed and therefore has a max value of 127,
    // GCC issues a warning because the comparison is always true.
    // Therefore, we suppress the warning here.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    return c >= 0 and c <= 127;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

template<typename T, typename Contained>
concept IsOptional = requires(T&& t) {
    { t.has_value() } -> std::same_as<bool>;
    { t.value() } -> std::convertible_to<Contained>;
};

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
        assert(is_ascii(left));
        assert(is_ascii(right));
        auto const left_upper = static_cast<char>(std::toupper(left));
        auto const right_upper = static_cast<char>(std::toupper(right));
        return left_upper == right_upper;
    });
}
