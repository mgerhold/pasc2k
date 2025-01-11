#pragma once

#include <common/common.hpp>

class SourceLocation final {
    friend class Lexer;

public:
    struct Position final {
        usize start_line;
        usize start_column;
        usize end_line;
        usize end_column;

        [[nodiscard]] explicit Position(
            usize const start_line,
            usize const start_column,
            usize const end_line,
            usize const end_column
        )
            : start_line{ start_line }, start_column{ start_column }, end_line{ end_line }, end_column{ end_column } {}

        [[nodiscard]] constexpr bool operator==(Position const& other) const = default;
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

    [[nodiscard]] Position position() const {
        auto start_line = usize{ 1 };
        auto start_column = usize{ 1 };
        for (auto offset = usize{ 0 }; offset < m_offset; ++offset) {
            if (m_source.at(offset) == '\n') {
                ++start_line;
                start_column = 1;
            } else {
                ++start_column;
            }
        }
        auto end_line = start_line;
        auto end_column = start_column;
        for (auto offset = m_offset; offset < std::min(m_offset + m_length, m_source.length()); ++offset) {
            if (m_source.at(offset) == '\n') {
                ++end_line;
                end_column = 1;
            } else {
                ++end_column;
            }
        }
        return Position{ start_line, start_column, end_line, end_column };
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

    [[nodiscard]] constexpr SourceLocation end() const {
        return SourceLocation{ m_path, m_source, m_offset + m_length, 0 };
    }

    [[nodiscard]] constexpr SourceLocation join(SourceLocation const& other) const {
        auto const begin = std::min(m_offset, other.m_offset);
        auto const end = std::max(m_offset + m_length, other.m_offset + other.m_length);
        return SourceLocation{ m_path, m_source, begin, end - begin };
    }

    [[nodiscard]] std::vector<std::string_view> surrounding_lines() const {
        /*auto line_start = usize{ 0 };
        for (auto offset = usize{ 0 }; offset < m_offset; ++offset) {
            if (m_source.at(offset) == '\n') {
                line_start = offset + 1;
            }
        }*/
        auto line_start = m_offset;
        while (line_start > 0 and m_source.at(line_start - 1) != '\n') {
            --line_start;
        }

        auto lines = std::vector<std::string_view>{};

        for (auto offset = m_offset; offset < std::min(m_source.size(), m_offset + m_length); ++offset) {
            if (m_source.at(offset) == '\n') {
                lines.emplace_back(m_source.cbegin() + line_start, m_source.cbegin() + offset);
                line_start = offset + 1;
            }
        }
        if (line_start < m_source.size()) {
            lines.emplace_back(m_source.cbegin() + line_start, m_source.cend());
        }
        return lines;
    }
};

template<>
struct std::formatter<SourceLocation> : std::formatter<std::string> {
    auto format(SourceLocation const& source_location, std::format_context& context) const {
        auto const [start_line, start_column, end_line, end_column] = source_location.position();
        return std::formatter<std::string>::format(
            std::format("{}:{}:{}", source_location.path(), start_line, start_column),
            context
        );
    }
};
