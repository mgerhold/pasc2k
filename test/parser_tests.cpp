#include <gtest/gtest.h>
#include <lexer/lexer.hpp>
#include <parser/block.hpp>
#include <parser/parser.hpp>

[[nodiscard]] static Ast parse(std::string_view const source) {
    auto tokens = tokenize("test", source);
    auto ast = parse(std::move(tokens));
    return ast;
}

TEST(ParserTests, Test) {
    auto ast = parse("");
}
