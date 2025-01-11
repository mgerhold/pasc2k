#pragma once

#include <lexer/source_location.hpp>
#include <string>

class ParserNote final {
private:
    SourceLocation m_source_location;
    std::string m_message;

public:
    [[nodiscard]] ParserNote(SourceLocation const& source_location, std::string message)
        : m_source_location{ source_location }, m_message{ std::move(message) } {}

    [[nodiscard]] SourceLocation const& source_location() const {
        return m_source_location;
    }

    [[nodiscard]] std::string const& message() const {
        return m_message;
    }
};
