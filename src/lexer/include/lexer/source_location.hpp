#pragma once

#include <common/common.hpp>

class SourceLocation final {
    friend class Lexer;
public:
    struct Position final {
        usize line;
        usize column;

        [[nodiscard]] constexpr Position(usize const line, usize const column)
            : line{ line }, column{ column } {}

        [[nodiscard]] constexpr bool operator==(Position const& other) const {
            return line == other.line and column == other.column;
        }
    };

private:
    std::string_view m_path;
    std::string_view m_source;
    usize m_offset;
    usize m_length;

public:
    [[nodiscard]] explicit constexpr SourceLocation(
        std::string_view const path,
        std::string_view const source,
        usize const offset,
        usize const length
    )
        : m_path{ path }, m_source{ source }, m_offset{ offset }, m_length{ length } {}

    [[nodiscard]] constexpr Position position() const {
        auto line = usize{ 1 };
        auto column = usize{ 1 };
        for (auto offset = usize{ 0 }; offset < m_offset; ++offset) {
            if (m_source.at(offset) == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }
        return Position{ line, column };
    }

    [[nodiscard]] constexpr std::string_view text() const {
        using Difference = decltype(m_source)::difference_type;
        auto const begin = m_source.cbegin() + static_cast<Difference>(m_offset);
        auto const end = begin + static_cast<Difference>(m_length);
        return std::string_view{ begin, end };
    }

    [[nodiscard]] constexpr std::string_view const& path() const {
        return m_path;
    }

    [[nodiscard]] constexpr usize length() const {
        return m_length;
    }
};
