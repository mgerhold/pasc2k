#pragma once

#include <lexer/token.hpp>
#include <memory>

class Type {
public:
    Type() = default;
    Type(Type const& other) = default;
    Type(Type&& other) noexcept = default;
    Type& operator=(Type const& other) = default;
    Type& operator=(Type&& other) noexcept = default;
    virtual ~Type() = default;
};

class IntegerType final : public Type {};

class RealType final : public Type {};

class BooleanType final : public Type {};

class CharType final : public Type {};

class EnumeratedType final : public Type {
private:
    std::vector<Token const*> m_identifiers;

public:
    [[nodiscard]] explicit EnumeratedType(std::vector<Token const*> identifiers)
        : m_identifiers{ std::move(identifiers) } {}
};

class IntegerSubrangeType final : public Type {
private:
    i64 m_from;
    i64 m_to;

public:
    [[nodiscard]] explicit IntegerSubrangeType(i64 const from, i64 const to)
        : m_from{ from }, m_to{ to } {}

    [[nodiscard]] i64 from() const {
        return m_from;
    }

    [[nodiscard]] i64 to() const {
        return m_to;
    }
};

class BooleanSubrangeType final : public Type {
private:
    bool m_from;
    bool m_to;

public:
    [[nodiscard]] explicit BooleanSubrangeType(bool const from, bool const to)
        : m_from{ from }, m_to{ to } {}

    [[nodiscard]] bool from() const {
        return m_from;
    }

    [[nodiscard]] bool to() const {
        return m_to;
    }
};

class CharSubrangeType final : public Type {
private:
    char m_from;
    char m_to;

public:
    [[nodiscard]] explicit CharSubrangeType(char const from, char const to)
        : m_from{ from }, m_to{ to } {}

    [[nodiscard]] char from() const {
        return m_from;
    }

    [[nodiscard]] char to() const {
        return m_to;
    }
};

class EnumeratedSubrangeType final : public Type {
private:
    Token const* m_from;
    Token const* m_to;

public:
    [[nodiscard]] explicit EnumeratedSubrangeType(Token const& from, Token const& to)
        : m_from{ &from }, m_to{ &to } {}

    [[nodiscard]] Token const& from() const {
        return *m_from;
    }

    [[nodiscard]] Token const& to() const {
        return *m_to;
    }
};

class TypeIdentifierType final : public Type {
private:
    Token const* m_identifier;

public:
    [[nodiscard]] explicit TypeIdentifierType(Token const& identifier)
        : m_identifier{ &identifier } {}

    [[nodiscard]] Token const& identifier() const {
        return *m_identifier;
    }
};

// TODO: Other types...

class TypeDefinition final {
private:
    Token const* m_identifier;
    std::unique_ptr<Type> m_type;

public:
    [[nodiscard]] TypeDefinition(Token const& identifier, std::unique_ptr<Type> type)
        : m_identifier{ &identifier }, m_type{ std::move(type) } {}

    [[nodiscard]] Token const& identifier() const {
        return *m_identifier;
    }

    [[nodiscard]] std::unique_ptr<Type> const& type() const {
        return m_type;
    }
};
