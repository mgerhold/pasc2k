#pragma once

#include <lexer/source_location.hpp>
#include <stdexcept>
#include <vector>
#include "parser_note.hpp"

class ParserError final : public std::runtime_error {
private:
    SourceLocation m_source_location;
    std::vector<ParserNote> m_notes;

public:
    [[nodiscard]] ParserError(
        std::string const& error_message,
        SourceLocation const& source_location,
        std::vector<ParserNote> notes = {}
    )
        : runtime_error{ error_message }, m_source_location{ source_location }, m_notes{ std::move(notes) } {}

    [[nodiscard]] SourceLocation const& source_location() const {
        return m_source_location;
    }

    [[nodiscard]] std::vector<ParserNote> const& notes() const {
        return m_notes;
    }
};
