#include <diagnostics/diagnostics.hpp>
#include <filesystem>
#include <fstream>
#include <lexer/lexer.hpp>
#include <parser/parser.hpp>
#include <print>
#include <sstream>
#include <string_view>

[[nodiscard]] std::string read_file(std::filesystem::path const& path) {
    auto file = std::ifstream{ path };
    auto stream = std::ostringstream{};
    stream << file.rdbuf();
    if (not file) {
        throw std::runtime_error{ "Failed to read file" };
    }
    return std::move(stream).str();
}

int main() {
    using namespace std::string_view_literals;
    static constexpr auto path = "test/block.pas"sv;
    auto const source = read_file(path);
    try {
        auto tokens = tokenize(path, source);
        for (auto const& token : tokens) {
            std::println("{}, {}", token, token.source_location());
        }
        auto const block = parse(std::move(tokens));
    } catch (std::exception const& e) {
        format_error_to(std::cout, e);
        return EXIT_FAILURE;
    }
}
