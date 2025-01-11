#pragma once

#include <format>
#include <iostream>
#include <magic_enum.hpp>

// 6.1.2
enum class TokenType {
    // Spacial symbols.
    Plus,
    Minus,
    Asterisk,
    Slash,
    Equals,
    LessThan,
    GreaterThan,
    LeftSquareBracket,  // `[` or `(.` (alternative token)
    RightSquareBracket,  // `]` or `.)` (alternative token)
    Dot,
    Comma,
    Colon,
    Semicolon,
    UpArrow,  // `↑` (not supported) or `^` or `@` (alternative token)
    LeftParenthesis,
    RightParenthesis,
    LessThanGreaterThan,
    LessThanEquals,
    GreaterThanEquals,
    ColonEquals,
    DotDot,

    // Word symbols.
    And,
    Array,
    Begin,
    Case,
    Const,
    Div,
    Do,
    DownTo,
    Else,
    End,
    File,
    For,
    Function,
    Goto,
    If,
    In,
    Label,
    Mod,
    Nil,
    Not,
    Of,
    Or,
    Packed,
    Procedure,
    Program,
    Record,
    Repeat,
    Set,
    Then,
    To,
    Type,
    Until,
    Var,
    While,
    With,

    // 6.1.3
    Identifier,

    // 6.1.4
    Directive,

    // 6.1.5 Numbers.
    IntegerNumber,
    RealNumber,

    // 6.1.7 Character Strings.
    Char,
    String,

    EndOfFile,
};

std::ostream& operator<<(std::ostream& os, TokenType type);

template<>
struct std::formatter<TokenType> : std::formatter<std::string> {
    auto format(TokenType const type, std::format_context& context) const {
        return std::formatter<std::string>::format(std::format("{}", magic_enum::enum_name(type)), context);
    }
};
