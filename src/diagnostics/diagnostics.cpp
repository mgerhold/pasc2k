#include <diagnostics/diagnostics.hpp>
#include <lexer/lexer_error.hpp>
#include <lexer/source_location.hpp>
#include <magic_enum.hpp>
#include <parser/parser_error.hpp>
#include <print>
#include "colors.hpp"

enum class DiagnosticsType {
    Error,
    Note,
};

static void format_source_location_to(std::ostream& stream, SourceLocation const& source_location) {
    auto const [line, column] = source_location.position();
    std::print(stream, "{}:{}:{}: ", source_location.path(), line, column);
}

static void format_line_to(std::ostream& stream, SourceLocation const& source_location, bool const use_color) {
    auto const [line, column] = source_location.position();
    if (use_color) {
        set_text_color(TextColor::White);
    }
    std::print(stream, "{:5}", line);
    if (use_color) {
        reset_colors();
    }
    std::println(stream, " | {}", source_location.surrounding_line());
    std::print(stream, "      |");
    if (use_color) {
        set_text_color(TextColor::Green);
    }
    std::print(stream, "{:>{}}^", "", column);
    if (source_location.length() > 1) {
        std::print(stream, "{:~>{}}", "", source_location.length() - 1);
    }
    if (use_color) {
        reset_colors();
    }
    std::println();
}

[[nodiscard]] static TextColor color(DiagnosticsType const type) {
    switch (type) {
        case DiagnosticsType::Error:
            return TextColor::Red;
        case DiagnosticsType::Note:
            return TextColor::Blue;
    }
    throw InternalCompilerError{ "Unknown diagnostics type" };
}

static void format_to_with_source_location(
    std::ostream& stream,
    std::string const& error_message,
    SourceLocation const& source_location,
    DiagnosticsType const type,
    bool const use_color
) {
    format_source_location_to(stream, source_location);
    if (use_color) {
        set_text_color(color(type));
    }
    std::print(stream, "{}: ", magic_enum::enum_name(type));
    if (use_color) {
        reset_colors();
    }
    std::println(stream, "{}", error_message);
    format_line_to(stream, source_location, use_color);
}

void format_to_without_source_location(std::ostream& stream, std::string const& error_message, bool const use_color) {
    if (use_color) {
        set_text_color(TextColor::Red);
    }
    std::print(stream, "Error: ");
    if (use_color) {
        reset_colors();
    }
    std::println(stream, "{}", error_message);
}

void format_error_to(std::ostream& stream, std::exception const& error, bool const use_color) {
    if (auto const lexer_error = dynamic_cast<LexerError const*>(&error); lexer_error != nullptr) {
        format_to_with_source_location(
            stream,
            error.what(),
            lexer_error->source_location(),
            DiagnosticsType::Error,
            use_color
        );
    } else if (auto const parser_error = dynamic_cast<ParserError const*>(&error); parser_error != nullptr) {
        format_to_with_source_location(
            stream,
            error.what(),
            parser_error->source_location(),
            DiagnosticsType::Error,
            use_color
        );
        for (auto const& note : parser_error->notes()) {
            format_to_with_source_location(stream, note.message(), note.source_location(), DiagnosticsType::Note, use_color);
        }
    } else {
        format_to_without_source_location(stream, error.what(), use_color);
    }
}
