#pragma once

#include <common/common.hpp>

#include "lexer/token.hpp"

[[nodiscard]] std::vector<Token> tokenize(std::string_view path, std::string_view source);
