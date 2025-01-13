#pragma once

#include <lexer/token.hpp>
#include <memory>
#include "ast_node.hpp"
#include "constant_definition.hpp"

class Type : public AstNode {};

namespace detail {
    template<typename Parent, TokenType token_type>
    class BuiltInType : public Parent {
    private:
        Token const* m_token;
        std::string_view m_ast_node_name;

    public:
        [[nodiscard]] explicit BuiltInType(Token const& token, std::string_view const ast_node_name)
            : m_token{ &token }, m_ast_node_name{ ast_node_name } {
            if (m_token->type() != token_type) {
                throw InternalCompilerError{ "Invalid token type for built-in type." };
            }
        }

        [[nodiscard]] SourceLocation source_location() const override {
            return m_token->source_location();
        }

        void print(AstNode::PrintContext& context) const override {
            context.print(*this, m_ast_node_name);
        }
    };
}  // namespace detail

class RealType final : public detail::BuiltInType<Type, TokenType::Real> {
public:
    [[nodiscard]] explicit RealType(Token const& token)
        : BuiltInType{ token, "RealType" } {}
};

class OrdinalType : public Type {};

class BooleanType final : public detail::BuiltInType<OrdinalType, TokenType::Boolean> {
public:
    [[nodiscard]] explicit BooleanType(Token const& token)
        : BuiltInType{ token, "BooleanType" } {}
};

class IntegerType final : public detail::BuiltInType<OrdinalType, TokenType::Integer> {
public:
    [[nodiscard]] explicit IntegerType(Token const& token)
        : BuiltInType{ token, "IntegerType" } {}
};

class CharType final : public detail::BuiltInType<OrdinalType, TokenType::Char> {
public:
    [[nodiscard]] explicit CharType(Token const& token)
        : BuiltInType{ token, "CharType" } {}
};

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

class TypeAliasDefinition final : public OrdinalType {
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

class EnumeratedTypeDefinition final : public OrdinalType {
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

class SubrangeTypeDefinition final : public OrdinalType {
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

class UnpackedStructuredTypeDefinition : public Type {};

class StructuredTypeDefinition final : public Type {
private:
    tl::optional<Token const&> m_packed;
    std::unique_ptr<UnpackedStructuredTypeDefinition> m_unpacked_structured_type_definition;

public:
    [[nodiscard]] explicit StructuredTypeDefinition(
        tl::optional<Token const&> const& packed,
        std::unique_ptr<UnpackedStructuredTypeDefinition> unpacked_structured_type_definition
    )
        : m_packed{ packed }, m_unpacked_structured_type_definition{ std::move(unpacked_structured_type_definition) } {}

    [[nodiscard]] SourceLocation source_location() const override {
        if (m_packed.has_value()) {
            return m_packed.value().source_location().join(m_unpacked_structured_type_definition->source_location());
        }
        return m_unpacked_structured_type_definition->source_location();
    }

    void print(PrintContext& context) const override {
        if (m_packed) {
            context.print(*this, "StructuredTypeDefinition", m_packed->lexeme());
        } else {
            context.print(*this, "StructuredTypeDefinition");
        }
        context.print_children(*m_unpacked_structured_type_definition);
    }
};

class ArrayTypeDefinition final : public UnpackedStructuredTypeDefinition {
private:
    Token const* m_array;
    std::vector<std::unique_ptr<OrdinalType>> m_index_types;
    std::unique_ptr<Type> m_component_type;

public:
    [[nodiscard]] ArrayTypeDefinition(
        Token const& array,
        std::vector<std::unique_ptr<OrdinalType>> index_types,
        std::unique_ptr<Type> component_type
    )
        : m_array{ &array }, m_index_types{ std::move(index_types) }, m_component_type{ std::move(component_type) } {}

    [[nodiscard]] std::vector<std::unique_ptr<OrdinalType>> const& index_types() const {
        return m_index_types;
    }

    [[nodiscard]] Type const& component_type() const {
        return *m_component_type;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_array->source_location().join(component_type().source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "ArrayTypeDefinition");
        auto children = std::vector<AstNode const*>{};
        for (auto const& index_type : m_index_types) {
            children.push_back(index_type.get());
        }
        children.push_back(m_component_type.get());
        context.print_children(children);
    }
};

/*class RecordTypeDefinition final : public UnpackedStructuredTypeDefinition {};

class SetTypeDefinition final : public UnpackedStructuredTypeDefinition {};

class FileTypeDefinition final : public UnpackedStructuredTypeDefinition {};
*/
