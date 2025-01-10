#include <gtest/gtest.h>
#include <lexer/lexer.hpp>
#include <lexer/lexer_error.hpp>

using namespace std::string_view_literals;

static std::vector<Token> tokenize(std::string_view const source) {
    return tokenize("test.pas", source);
}

TEST(LexerTests, EmptySource_ReturnsOnlyEndOfFileToken) {
    auto const tokens = tokenize("");
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens.at(0).type(), TokenType::EndOfFile);
}

TEST(LexerTests, OnlyWhitespace_ReturnsOnlyEndOfFileToken) {
    auto const tokens = tokenize(" \f\n\r\t\v");
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens.at(0).type(), TokenType::EndOfFile);
}

TEST(LexerTests, AllSpecialSymbols_TokenizesCorrectly) {
    static constexpr auto source = "+ - * / = < > [ ] . , : ; ^ ( ) <> <= >= := .. +-*/=<>[].,:;^()<><=>=:=.."sv;
    auto const tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 42);
    EXPECT_EQ(tokens.at(0).type(), TokenType::Plus);
    EXPECT_EQ(tokens.at(1).type(), TokenType::Minus);
    EXPECT_EQ(tokens.at(2).type(), TokenType::Asterisk);
    EXPECT_EQ(tokens.at(3).type(), TokenType::Slash);
    EXPECT_EQ(tokens.at(4).type(), TokenType::Equals);
    EXPECT_EQ(tokens.at(5).type(), TokenType::LessThan);
    EXPECT_EQ(tokens.at(6).type(), TokenType::GreaterThan);
    EXPECT_EQ(tokens.at(7).type(), TokenType::LeftSquareBracket);
    EXPECT_EQ(tokens.at(8).type(), TokenType::RightSquareBracket);
    EXPECT_EQ(tokens.at(9).type(), TokenType::Dot);
    EXPECT_EQ(tokens.at(10).type(), TokenType::Comma);
    EXPECT_EQ(tokens.at(11).type(), TokenType::Colon);
    EXPECT_EQ(tokens.at(12).type(), TokenType::Semicolon);
    EXPECT_EQ(tokens.at(13).type(), TokenType::UpArrow);
    EXPECT_EQ(tokens.at(14).type(), TokenType::LeftParenthesis);
    EXPECT_EQ(tokens.at(15).type(), TokenType::RightParenthesis);
    EXPECT_EQ(tokens.at(16).type(), TokenType::LessThanGreaterThan);
    EXPECT_EQ(tokens.at(17).type(), TokenType::LessThanEquals);
    EXPECT_EQ(tokens.at(18).type(), TokenType::GreaterThanEquals);
    EXPECT_EQ(tokens.at(19).type(), TokenType::ColonEquals);
    EXPECT_EQ(tokens.at(20).type(), TokenType::DotDot);
    EXPECT_EQ(tokens.at(21).type(), TokenType::Plus);
    EXPECT_EQ(tokens.at(22).type(), TokenType::Minus);
    EXPECT_EQ(tokens.at(23).type(), TokenType::Asterisk);
    EXPECT_EQ(tokens.at(24).type(), TokenType::Slash);
    EXPECT_EQ(tokens.at(25).type(), TokenType::Equals);
    EXPECT_EQ(tokens.at(26).type(), TokenType::LessThanGreaterThan);
    EXPECT_EQ(tokens.at(27).type(), TokenType::LeftSquareBracket);
    EXPECT_EQ(tokens.at(28).type(), TokenType::RightSquareBracket);
    EXPECT_EQ(tokens.at(29).type(), TokenType::Dot);
    EXPECT_EQ(tokens.at(30).type(), TokenType::Comma);
    EXPECT_EQ(tokens.at(31).type(), TokenType::Colon);
    EXPECT_EQ(tokens.at(32).type(), TokenType::Semicolon);
    EXPECT_EQ(tokens.at(33).type(), TokenType::UpArrow);
    EXPECT_EQ(tokens.at(34).type(), TokenType::LeftParenthesis);
    EXPECT_EQ(tokens.at(35).type(), TokenType::RightParenthesis);
    EXPECT_EQ(tokens.at(36).type(), TokenType::LessThanGreaterThan);
    EXPECT_EQ(tokens.at(37).type(), TokenType::LessThanEquals);
    EXPECT_EQ(tokens.at(38).type(), TokenType::GreaterThanEquals);
    EXPECT_EQ(tokens.at(39).type(), TokenType::ColonEquals);
    EXPECT_EQ(tokens.at(40).type(), TokenType::DotDot);

    EXPECT_EQ(tokens.at(41).type(), TokenType::EndOfFile);
}

TEST(LexerTests, AllWordSymbols_TokenizesCorrectly) {
    static constexpr auto source = R"(and array begin case const div
do downto else end file for function goto if in label mod nil not of
or packed procedure program record repeat set then to type until var
while with
AND ARRAY BEGIN CASE CONST DIV DO DOWNTO ELSE END FILE FOR FUNCTION
GOTO IF IN LABEL MOD NIL NOT OF OR PACKED PROCEDURE PROGRAM RECORD
REPEAT SET THEN TO TYPE UNTIL VAR WHILE WITH
AnD aRrAy BeGiN cAsE cOnSt DiV dO dOwNtO eLsE eNd FiLe FoR fUnCtIoN
GoTo If In LaBeL mOd NiL nOt Of Or PaCkEd PrOcEdUrE pRoGrAm ReCoRd
RePeAt SeT tHeN tO tYpE uNtIl VaR wHiLe WiTh)"sv;
    auto const tokens = tokenize(source);

    EXPECT_EQ(tokens.size(), 106);

    EXPECT_EQ(tokens.at(0).type(), TokenType::And);
    EXPECT_EQ(tokens.at(1).type(), TokenType::Array);
    EXPECT_EQ(tokens.at(2).type(), TokenType::Begin);
    EXPECT_EQ(tokens.at(3).type(), TokenType::Case);
    EXPECT_EQ(tokens.at(4).type(), TokenType::Const);
    EXPECT_EQ(tokens.at(5).type(), TokenType::Div);
    EXPECT_EQ(tokens.at(6).type(), TokenType::Do);
    EXPECT_EQ(tokens.at(7).type(), TokenType::DownTo);
    EXPECT_EQ(tokens.at(8).type(), TokenType::Else);
    EXPECT_EQ(tokens.at(9).type(), TokenType::End);
    EXPECT_EQ(tokens.at(10).type(), TokenType::File);
    EXPECT_EQ(tokens.at(11).type(), TokenType::For);
    EXPECT_EQ(tokens.at(12).type(), TokenType::Function);
    EXPECT_EQ(tokens.at(13).type(), TokenType::Goto);
    EXPECT_EQ(tokens.at(14).type(), TokenType::If);
    EXPECT_EQ(tokens.at(15).type(), TokenType::In);
    EXPECT_EQ(tokens.at(16).type(), TokenType::Label);
    EXPECT_EQ(tokens.at(17).type(), TokenType::Mod);
    EXPECT_EQ(tokens.at(18).type(), TokenType::Nil);
    EXPECT_EQ(tokens.at(19).type(), TokenType::Not);
    EXPECT_EQ(tokens.at(20).type(), TokenType::Of);
    EXPECT_EQ(tokens.at(21).type(), TokenType::Or);
    EXPECT_EQ(tokens.at(22).type(), TokenType::Packed);
    EXPECT_EQ(tokens.at(23).type(), TokenType::Procedure);
    EXPECT_EQ(tokens.at(24).type(), TokenType::Program);
    EXPECT_EQ(tokens.at(25).type(), TokenType::Record);
    EXPECT_EQ(tokens.at(26).type(), TokenType::Repeat);
    EXPECT_EQ(tokens.at(27).type(), TokenType::Set);
    EXPECT_EQ(tokens.at(28).type(), TokenType::Then);
    EXPECT_EQ(tokens.at(29).type(), TokenType::To);
    EXPECT_EQ(tokens.at(30).type(), TokenType::Type);
    EXPECT_EQ(tokens.at(31).type(), TokenType::Until);
    EXPECT_EQ(tokens.at(32).type(), TokenType::Var);
    EXPECT_EQ(tokens.at(33).type(), TokenType::While);
    EXPECT_EQ(tokens.at(34).type(), TokenType::With);

    EXPECT_EQ(tokens.at(35).type(), TokenType::And);
    EXPECT_EQ(tokens.at(36).type(), TokenType::Array);
    EXPECT_EQ(tokens.at(37).type(), TokenType::Begin);
    EXPECT_EQ(tokens.at(38).type(), TokenType::Case);
    EXPECT_EQ(tokens.at(39).type(), TokenType::Const);
    EXPECT_EQ(tokens.at(40).type(), TokenType::Div);
    EXPECT_EQ(tokens.at(41).type(), TokenType::Do);
    EXPECT_EQ(tokens.at(42).type(), TokenType::DownTo);
    EXPECT_EQ(tokens.at(43).type(), TokenType::Else);
    EXPECT_EQ(tokens.at(44).type(), TokenType::End);
    EXPECT_EQ(tokens.at(45).type(), TokenType::File);
    EXPECT_EQ(tokens.at(46).type(), TokenType::For);
    EXPECT_EQ(tokens.at(47).type(), TokenType::Function);
    EXPECT_EQ(tokens.at(48).type(), TokenType::Goto);
    EXPECT_EQ(tokens.at(49).type(), TokenType::If);
    EXPECT_EQ(tokens.at(50).type(), TokenType::In);
    EXPECT_EQ(tokens.at(51).type(), TokenType::Label);
    EXPECT_EQ(tokens.at(52).type(), TokenType::Mod);
    EXPECT_EQ(tokens.at(53).type(), TokenType::Nil);
    EXPECT_EQ(tokens.at(54).type(), TokenType::Not);
    EXPECT_EQ(tokens.at(55).type(), TokenType::Of);
    EXPECT_EQ(tokens.at(56).type(), TokenType::Or);
    EXPECT_EQ(tokens.at(57).type(), TokenType::Packed);
    EXPECT_EQ(tokens.at(58).type(), TokenType::Procedure);
    EXPECT_EQ(tokens.at(59).type(), TokenType::Program);
    EXPECT_EQ(tokens.at(60).type(), TokenType::Record);
    EXPECT_EQ(tokens.at(61).type(), TokenType::Repeat);
    EXPECT_EQ(tokens.at(62).type(), TokenType::Set);
    EXPECT_EQ(tokens.at(63).type(), TokenType::Then);
    EXPECT_EQ(tokens.at(64).type(), TokenType::To);
    EXPECT_EQ(tokens.at(65).type(), TokenType::Type);
    EXPECT_EQ(tokens.at(66).type(), TokenType::Until);
    EXPECT_EQ(tokens.at(67).type(), TokenType::Var);
    EXPECT_EQ(tokens.at(68).type(), TokenType::While);
    EXPECT_EQ(tokens.at(69).type(), TokenType::With);

    EXPECT_EQ(tokens.at(70).type(), TokenType::And);
    EXPECT_EQ(tokens.at(71).type(), TokenType::Array);
    EXPECT_EQ(tokens.at(72).type(), TokenType::Begin);
    EXPECT_EQ(tokens.at(73).type(), TokenType::Case);
    EXPECT_EQ(tokens.at(74).type(), TokenType::Const);
    EXPECT_EQ(tokens.at(75).type(), TokenType::Div);
    EXPECT_EQ(tokens.at(76).type(), TokenType::Do);
    EXPECT_EQ(tokens.at(77).type(), TokenType::DownTo);
    EXPECT_EQ(tokens.at(78).type(), TokenType::Else);
    EXPECT_EQ(tokens.at(79).type(), TokenType::End);
    EXPECT_EQ(tokens.at(80).type(), TokenType::File);
    EXPECT_EQ(tokens.at(81).type(), TokenType::For);
    EXPECT_EQ(tokens.at(82).type(), TokenType::Function);
    EXPECT_EQ(tokens.at(83).type(), TokenType::Goto);
    EXPECT_EQ(tokens.at(84).type(), TokenType::If);
    EXPECT_EQ(tokens.at(85).type(), TokenType::In);
    EXPECT_EQ(tokens.at(86).type(), TokenType::Label);
    EXPECT_EQ(tokens.at(87).type(), TokenType::Mod);
    EXPECT_EQ(tokens.at(88).type(), TokenType::Nil);
    EXPECT_EQ(tokens.at(89).type(), TokenType::Not);
    EXPECT_EQ(tokens.at(90).type(), TokenType::Of);
    EXPECT_EQ(tokens.at(91).type(), TokenType::Or);
    EXPECT_EQ(tokens.at(92).type(), TokenType::Packed);
    EXPECT_EQ(tokens.at(93).type(), TokenType::Procedure);
    EXPECT_EQ(tokens.at(94).type(), TokenType::Program);
    EXPECT_EQ(tokens.at(95).type(), TokenType::Record);
    EXPECT_EQ(tokens.at(96).type(), TokenType::Repeat);
    EXPECT_EQ(tokens.at(97).type(), TokenType::Set);
    EXPECT_EQ(tokens.at(98).type(), TokenType::Then);
    EXPECT_EQ(tokens.at(99).type(), TokenType::To);
    EXPECT_EQ(tokens.at(100).type(), TokenType::Type);
    EXPECT_EQ(tokens.at(101).type(), TokenType::Until);
    EXPECT_EQ(tokens.at(102).type(), TokenType::Var);
    EXPECT_EQ(tokens.at(103).type(), TokenType::While);
    EXPECT_EQ(tokens.at(104).type(), TokenType::With);

    EXPECT_EQ(tokens.at(105).type(), TokenType::EndOfFile);
}

TEST(LexerTests, Identifiers_TokenizesCorrectly) {
    static constexpr auto source = R"(X time readinteger WG4 AlterHeatSetting
InquireWorkstationTransformation InquireWorkstationIdentification)"sv;
    auto const tokens = tokenize(source);

    EXPECT_EQ(tokens.size(), 8);

    EXPECT_EQ(tokens.at(0).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(0).lexeme(), "X");
    EXPECT_EQ(tokens.at(1).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(1).lexeme(), "time");
    EXPECT_EQ(tokens.at(2).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(2).lexeme(), "readinteger");
    EXPECT_EQ(tokens.at(3).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(3).lexeme(), "WG4");
    EXPECT_EQ(tokens.at(4).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(4).lexeme(), "AlterHeatSetting");
    EXPECT_EQ(tokens.at(5).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(5).lexeme(), "InquireWorkstationTransformation");
    EXPECT_EQ(tokens.at(6).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(6).lexeme(), "InquireWorkstationIdentification");

    EXPECT_EQ(tokens.at(7).type(), TokenType::EndOfFile);
}

TEST(LexerTests, NonAsciiCharacter_Throws) {
    static constexpr auto source = "🦀"sv;
    EXPECT_THROW(
        {
            try {
                tokenize(source);
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Non-ASCII character");
                throw;
            }
        },
        NonAsciiCharacter
    );
}

TEST(LexerTests, InvalidCharacter_Throws) {
    static constexpr auto source = "!"sv;
    EXPECT_THROW(
        {
            try {
                tokenize(source);
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Invalid character: Got '!', expected letter (a-z|A-Z)");
                throw;
            }
        },
        InvalidCharacter
    );
}

TEST(LexerTests, Numbers_TokenizesCorrectly) {
    static constexpr auto source = "1e10 1 +100 -0.1 5e-3 87.35E+8"sv;
    auto const tokens = tokenize(source);

    EXPECT_EQ(tokens.size(), 7);

    EXPECT_EQ(tokens.at(0).type(), TokenType::RealNumber);
    EXPECT_EQ(tokens.at(0).lexeme(), "1e10");
    EXPECT_EQ(tokens.at(1).type(), TokenType::IntegerNumber);
    EXPECT_EQ(tokens.at(1).lexeme(), "1");
    EXPECT_EQ(tokens.at(2).type(), TokenType::IntegerNumber);
    EXPECT_EQ(tokens.at(2).lexeme(), "+100");
    EXPECT_EQ(tokens.at(3).type(), TokenType::RealNumber);
    EXPECT_EQ(tokens.at(3).lexeme(), "-0.1");
    EXPECT_EQ(tokens.at(4).type(), TokenType::RealNumber);
    EXPECT_EQ(tokens.at(4).lexeme(), "5e-3");
    EXPECT_EQ(tokens.at(5).type(), TokenType::RealNumber);
    EXPECT_EQ(tokens.at(5).lexeme(), "87.35E+8");

    EXPECT_EQ(tokens.at(6).type(), TokenType::EndOfFile);
}

TEST(LexerTests, InvalidNumbers_Throws) {
    EXPECT_THROW(
        {
            try {
                tokenize("1e");
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Invalid character: Got non-printable character #0, expected digit");
                throw;
            }
        },
        InvalidCharacter
    );
    EXPECT_THROW(
        {
            try {
                tokenize("1.");
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Invalid character: Got non-printable character #0, expected digit");
                throw;
            }
        },
        InvalidCharacter
    );
    EXPECT_THROW(
        {
            try {
                tokenize("1.!");
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Invalid character: Got '!', expected digit");
                throw;
            }
        },
        InvalidCharacter
    );
}

TEST(LexerTests, CharLiteral_TokenizesCorrectly) {
    auto const tokens = tokenize("'a' 'b' '!' '_' ' ' '@' ''''");
    EXPECT_EQ(tokens.size(), 8);

    EXPECT_EQ(tokens.at(0).type(), TokenType::Char);
    EXPECT_EQ(tokens.at(0).lexeme(), "'a'");
    EXPECT_EQ(tokens.at(1).type(), TokenType::Char);
    EXPECT_EQ(tokens.at(1).lexeme(), "'b'");
    EXPECT_EQ(tokens.at(2).type(), TokenType::Char);
    EXPECT_EQ(tokens.at(2).lexeme(), "'!'");
    EXPECT_EQ(tokens.at(3).type(), TokenType::Char);
    EXPECT_EQ(tokens.at(3).lexeme(), "'_'");
    EXPECT_EQ(tokens.at(4).type(), TokenType::Char);
    EXPECT_EQ(tokens.at(4).lexeme(), "' '");
    EXPECT_EQ(tokens.at(5).type(), TokenType::Char);
    EXPECT_EQ(tokens.at(5).lexeme(), "'@'");
    EXPECT_EQ(tokens.at(6).type(), TokenType::Char);
    EXPECT_EQ(tokens.at(6).lexeme(), "''''");

    EXPECT_EQ(tokens.at(7).type(), TokenType::EndOfFile);
}

TEST(LexerTests, InvalidCharLiteral_Throws) {
    EXPECT_THROW(
        {
            try {
                tokenize("'\t'");
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Invalid character: Got non-printable character #9, expected string character");
                throw;
            }
        },
        InvalidCharacter
    );
}

TEST(LexerTests, StringLiteral_TokenizesCorrectly) {
    auto const tokens = tokenize("'Abc' 'Pascal' 'THIS IS A STRING' 'The name is ''Pascal''!'");
    EXPECT_EQ(tokens.size(), 5);

    EXPECT_EQ(tokens.at(0).type(), TokenType::String);
    EXPECT_EQ(tokens.at(0).lexeme(), "'Abc'");
    EXPECT_EQ(tokens.at(1).type(), TokenType::String);
    EXPECT_EQ(tokens.at(1).lexeme(), "'Pascal'");
    EXPECT_EQ(tokens.at(2).type(), TokenType::String);
    EXPECT_EQ(tokens.at(2).lexeme(), "'THIS IS A STRING'");
    EXPECT_EQ(tokens.at(3).type(), TokenType::String);
    EXPECT_EQ(tokens.at(3).lexeme(), "'The name is ''Pascal''!'");

    EXPECT_EQ(tokens.at(4).type(), TokenType::EndOfFile);
}

TEST(LexerTests, StringLiteral_Throws) {
    EXPECT_THROW(
        {
            try {
                tokenize("'Abc\t'");
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Invalid character: Got non-printable character #9, expected string character");
                throw;
            }
        },
        InvalidCharacter
    );
}

TEST(LexerTests, Comments_GetIgnored) {
    static constexpr auto source = R"({This is a comment and everything is allowed, even ferris 🦀
and linebreaks.} (*a comment can also start with '(*', but end with a curly brace} {or the
other way around*) (*or use parentheses on both ends*))"sv;

    auto const tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens.at(0).type(), TokenType::EndOfFile);
}

TEST(LexerTests, MissingTokenSeparator_Throws) {
    EXPECT_THROW(
        {
            try {
                tokenize("3abc");
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Invalid character: Got 'a', expected token separator");
                throw;
            }
        },
        InvalidCharacter
    );
}

TEST(LexerTests, AlternativeTokens_TokenizesCorrectly) {
    auto const tokens = tokenize("^@(..)");
    EXPECT_EQ(tokens.size(), 5);

    EXPECT_EQ(tokens.at(0).type(), TokenType::UpArrow);
    EXPECT_EQ(tokens.at(1).type(), TokenType::UpArrow);
    EXPECT_EQ(tokens.at(2).type(), TokenType::LeftSquareBracket);
    EXPECT_EQ(tokens.at(3).type(), TokenType::RightSquareBracket);
    EXPECT_EQ(tokens.at(4).type(), TokenType::EndOfFile);
}

TEST(LexerTests, UnterminatedComment_Throws) {
    EXPECT_THROW(
        {
            try {
                tokenize("(*");
            } catch (std::runtime_error const& e) {
                EXPECT_STREQ(e.what(), "Unterminated comment");
                throw;
            }
        },
        UnterminatedComment
    );
}

TEST(LexerTests, ValidTokens_CorrectSourceLocations) {
    static constexpr auto source = R"(begin
    writeln('Hello, world!');
end.)"sv;

    auto const tokens = tokenize(source);
    EXPECT_EQ(tokens.size(), 9);

    // No uniform initialization for SourceLocation::Position, because it won't work inside the macro.
    EXPECT_EQ(tokens.at(0).type(), TokenType::Begin);
    EXPECT_EQ(tokens.at(0).source_location().position(), SourceLocation::Position(1, 1));
    EXPECT_EQ(tokens.at(0).source_location().length(), 5);

    EXPECT_EQ(tokens.at(1).type(), TokenType::Identifier);
    EXPECT_EQ(tokens.at(1).lexeme(), "writeln");
    EXPECT_EQ(tokens.at(1).source_location().position(), SourceLocation::Position(2, 5));
    EXPECT_EQ(tokens.at(1).source_location().length(), 7);

    EXPECT_EQ(tokens.at(2).type(), TokenType::LeftParenthesis);
    EXPECT_EQ(tokens.at(2).source_location().position(), SourceLocation::Position(2, 12));
    EXPECT_EQ(tokens.at(2).source_location().length(), 1);

    EXPECT_EQ(tokens.at(3).type(), TokenType::String);
    EXPECT_EQ(tokens.at(3).source_location().position(), SourceLocation::Position(2, 13));
    EXPECT_EQ(tokens.at(3).source_location().length(), 15);

    EXPECT_EQ(tokens.at(4).type(), TokenType::RightParenthesis);
    EXPECT_EQ(tokens.at(4).source_location().position(), SourceLocation::Position(2, 28));
    EXPECT_EQ(tokens.at(4).source_location().length(), 1);

    EXPECT_EQ(tokens.at(5).type(), TokenType::Semicolon);
    EXPECT_EQ(tokens.at(5).source_location().position(), SourceLocation::Position(2, 29));
    EXPECT_EQ(tokens.at(5).source_location().length(), 1);

    EXPECT_EQ(tokens.at(6).type(), TokenType::End);
    EXPECT_EQ(tokens.at(6).source_location().position(), SourceLocation::Position(3, 1));
    EXPECT_EQ(tokens.at(6).source_location().length(), 3);

    EXPECT_EQ(tokens.at(7).type(), TokenType::Dot);
    EXPECT_EQ(tokens.at(7).source_location().position(), SourceLocation::Position(3, 4));
    EXPECT_EQ(tokens.at(7).source_location().length(), 1);
}
