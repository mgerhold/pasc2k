#pragma once

#include <format>
#include <stdexcept>
#include <string>
#include "source_location.hpp"

class LexerError : public std::runtime_error {
private:
    SourceLocation m_source_location;

protected:
    [[nodiscard]] explicit LexerError(std::string const& message, SourceLocation const& source_location)
        : std::runtime_error{ message }, m_source_location{ source_location } {}

public:
    [[nodiscard]] SourceLocation const& source_location() const {
        return m_source_location;
    }
};

class NonAsciiCharacter final : public LexerError {
public:
    [[nodiscard]] explicit NonAsciiCharacter(SourceLocation const& source_location)
        : LexerError{ "Non-ASCII character", source_location } {}
};

class UnexpectedCharacter final : public LexerError {
public:
    [[nodiscard]] explicit UnexpectedCharacter(
        SourceLocation const& source_location,
        char const actual,
        char const* const expected
    )
        : LexerError{
              std::isprint(static_cast<unsigned char>(actual))
                  ? std::format("Unexpected character: Got '{}', expected {}", actual, expected)
                  : std::format(
                        "Unexpected character: Got non-printable character #{}, expected {}",
                        static_cast<int>(actual),
                        expected
                    ),
              source_location,
          } {}
};

class UnterminatedCharacterString final : public LexerError {
public:
    [[nodiscard]] explicit UnterminatedCharacterString(SourceLocation const& source_location)
        : LexerError{ "Unterminated character string", source_location } {}
};

class UnterminatedComment final : public LexerError {
public:
    [[nodiscard]] explicit UnterminatedComment(SourceLocation const& source_location)
        : LexerError{ "Unterminated comment", source_location } {}
};
