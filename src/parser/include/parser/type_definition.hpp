#pragma once

#include <lexer/token.hpp>
#include <memory>
#include "ast_node.hpp"
#include "constant_definition.hpp"

class Type : public AstNode {};

class TypeDefinition final : public AstNode {
private:
    Identifier m_identifier;
    std::unique_ptr<Type> m_type;

public:
    [[nodiscard]] explicit TypeDefinition(Identifier const& identifier, std::unique_ptr<Type> type)
        : m_identifier{ identifier }, m_type{ std::move(type) } {}

    [[nodiscard]] Identifier const& identifier() const {
        return m_identifier;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifier.source_location().join(m_type->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "TypeDefinition");
        context.print_children(m_identifier, *m_type);
    }
};

class TypeAliasDefinition final : public Type {
private:
    Identifier m_referenced_type;

public:
    [[nodiscard]] explicit TypeAliasDefinition(Identifier const& referenced_type)
        : m_referenced_type{ referenced_type } {}

    [[nodiscard]] Identifier const& referenced_type() const {
        return m_referenced_type;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_referenced_type.source_location();
    }

    void print(PrintContext& context) const override {
        context.print(*this, "TypeAliasDefinition");
        context.print_children(m_referenced_type);
    }
};

class EnumeratedTypeDefinition final : public Type {
private:
    Token const* m_left_parenthesis;
    std::vector<Identifier> m_identifiers;
    Token const* m_right_parenthesis;

public:
    [[nodiscard]] explicit EnumeratedTypeDefinition(
        Token const& left_parenthesis,
        std::vector<Identifier> identifiers,
        Token const& right_parenthesis
    )
        : m_left_parenthesis{ &left_parenthesis },
          m_identifiers{ std::move(identifiers) },
          m_right_parenthesis{ &right_parenthesis } {
        if (m_identifiers.empty()) {
            throw InternalCompilerError{ "EnumeratedTypeDefinition must have at least one identifier." };
        }
    }

    [[nodiscard]] std::vector<Identifier> const& identifiers() const {
        return m_identifiers;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_left_parenthesis->source_location().join(m_right_parenthesis->source_location());
    }

    void print(PrintContext& context) const override {
        using std::views::transform, std::ranges::to;
        context.print(*this, "EnumeratedTypeDefinition");
        // clang-format off
        context.print_children(
            m_identifiers
                | transform([](auto const& identifier) { return static_cast<AstNode const*>(&identifier); })
                | to<std::vector>()
        );
        // clang-format on
    }
};

class SubrangeTypeDefinition final : public Type {
private:
    std::unique_ptr<Constant> m_from;
    std::unique_ptr<Constant> m_to;

public:
    [[nodiscard]] explicit SubrangeTypeDefinition(std::unique_ptr<Constant> from, std::unique_ptr<Constant> to)
        : m_from{ std::move(from) }, m_to{ std::move(to) } {}

    [[nodiscard]] std::unique_ptr<Constant> const& from() const {
        return m_from;
    }

    [[nodiscard]] std::unique_ptr<Constant> const& to() const {
        return m_to;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_from->source_location().join(m_to->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "SubrangeTypeDefinition");
        context.print_children(*m_from, *m_to);
    }
};
