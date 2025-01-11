#include <diagnostics/diagnostics.hpp>

#include <lexer/lexer_error.hpp>
#include <lexer/source_location.hpp>
#include <print>
#include "colors.hpp"

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

void format_error_to(std::ostream& stream, std::exception const& error, bool const use_color) {
    auto const lexer_error = dynamic_cast<LexerError const*>(&error);
    if (lexer_error != nullptr) {
        format_source_location_to(stream, lexer_error->source_location());
    }
    if (use_color) {
        set_text_color(TextColor::Red);
    }
    std::print(stream, "Error: ");
    if (use_color) {
        reset_colors();
    }
    std::println(stream, "{}", error.what());
    if (lexer_error != nullptr) {
        format_line_to(stream, lexer_error->source_location(), use_color);
    }
}
