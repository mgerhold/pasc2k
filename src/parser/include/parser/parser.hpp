#pragma once

#include <lexer/token.hpp>
#include <parser/parser_error.hpp>
#include <vector>
#include "block.hpp"

[[nodiscard]] Block parse(std::vector<Token>&& tokens);
