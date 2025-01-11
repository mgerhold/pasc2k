#pragma once

#include <lexer/token.hpp>
#include <parser/parser_error.hpp>
#include <vector>
#include "ast.hpp"

[[nodiscard]] Ast parse(std::vector<Token>&& tokens);
