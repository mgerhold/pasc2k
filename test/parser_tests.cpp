#include <gtest/gtest.h>
#include <lexer/lexer.hpp>
#include <parser/block.hpp>
#include <parser/parser.hpp>

[[nodiscard]] static Block parse(std::string_view const source) {
    auto tokens = tokenize("test", source);
    auto block = parse(std::move(tokens));
    return block;
}

TEST(ParserTests, Test) {
    auto block = parse("");
}
