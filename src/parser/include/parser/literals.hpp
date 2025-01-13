#pragma once

#include <lib2k/string_utils.hpp>
#include <lib2k/types.hpp>
#include <sstream>
#include "parser_error.hpp"

class IntegerLiteral final : public AstNode {
private:
    Token const* m_integer_token;
    i64 m_value;

public:
    [[nodiscard]] explicit IntegerLiteral(std::same_as<Token const> auto& integer_token)
        : m_integer_token{ &integer_token } {
        auto const parsed = c2k::parse<i64>(integer_token.lexeme());
        if (not parsed.has_value()) {
            throw ParserError{ "Integer literal out of range.", integer_token.source_location() };
        }
        m_value = parsed.value();
    }

    [[nodiscard]] i64 value() const {
        return m_value;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_integer_token->source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "IntegerLiteral", value());
    }
};

class RealLiteral final : public AstNode {
private:
    Token const* m_real_token;
    double m_value;

public:
    [[nodiscard]] explicit RealLiteral(std::same_as<Token const> auto& real_token)
        : m_real_token{ &real_token } {
        auto stream = std::istringstream{ std::string{ real_token.lexeme() } };
        auto result = 0.0;
        stream >> result;
        if (not stream.eof()) {
            throw InternalCompilerError{ "Failed to parse real number." };
        }
        m_value = result;
    }

    [[nodiscard]] double value() const {
        return m_value;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_real_token->source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "RealLiteral", value());
    }
};

class CharLiteral final : public AstNode {
private:
    Token const* m_char_token;
    char m_value;

public:
    [[nodiscard]] explicit CharLiteral(std::same_as<Token const> auto& char_token)
        : m_char_token{ &char_token } {
        if (char_token.lexeme().length() > 3) {
            if (char_token.lexeme().at(1) != '\'' or char_token.lexeme().at(2) != '\''
                or char_token.lexeme().length() != 4) {
                throw InternalCompilerError{ "Invalid character literal." };
            }
            m_value = '\'';
            return;
        }
        if (char_token.lexeme().length() != 3) {
            throw InternalCompilerError{ "Invalid character literal." };
        }
        m_value = char_token.lexeme().at(1);
    }

    [[nodiscard]] char value() const {
        return m_value;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_char_token->source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "CharLiteral", value());
    }
};

class StringLiteral final : public AstNode {
private:
    Token const* m_string_token;
    std::string m_value;

public:
    explicit StringLiteral(std::same_as<Token const> auto& string_token)
        : m_string_token{ &string_token } {
        auto result = std::string{};
        auto const lexeme = string_token.lexeme();
        for (auto i = usize{ 1 }; i < lexeme.length() - 1;) {
            if (lexeme.at(i) == '\'' and lexeme.at(i + 1) == '\'') {
                result += '\'';
                i += 2;
            } else {
                result += lexeme.at(i);
                ++i;
            }
        }
        m_value = std::move(result);
    }

    [[nodiscard]] std::string const& value() const {
        return m_value;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_string_token->source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "StringLiteral", value());
    }
};
