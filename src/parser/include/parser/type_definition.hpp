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
        [[nodiscard]] explicit BuiltInType(std::same_as<Token const> auto& token, std::string_view const ast_node_name)
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

class IdentifierList final : public AstNode {
private:
    std::vector<Identifier> m_identifiers;

public:
    [[nodiscard]] explicit IdentifierList(std::vector<Identifier> identifiers)
        : m_identifiers{ std::move(identifiers) } {
        if (m_identifiers.empty()) {
            throw InternalCompilerError{ "IdentifierList must have at least one identifier." };
        }
    }

    [[nodiscard]] std::vector<Identifier> const& identifiers() const {
        return m_identifiers;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifiers.front().source_location().join(m_identifiers.back().source_location());
    }

    void print(PrintContext& context) const override {
        using std::views::transform, std::ranges::to;
        context.print(*this, "IdentifierList");
        // clang-format off
        context.print_children(
            m_identifiers
                | transform([](auto const& identifier) { return static_cast<AstNode const*>(&identifier); })
                | to<std::vector>()
        );
        // clang-format on
    }
};

class EnumeratedTypeDefinition final : public OrdinalType {
private:
    Token const* m_left_parenthesis;
    IdentifierList m_identifiers;
    Token const* m_right_parenthesis;

public:
    [[nodiscard]] explicit EnumeratedTypeDefinition(
        std::same_as<Token const> auto& left_parenthesis,
        IdentifierList identifiers,
        std::same_as<Token const> auto& right_parenthesis
    )
        : m_left_parenthesis{ &left_parenthesis },
          m_identifiers{ std::move(identifiers) },
          m_right_parenthesis{ &right_parenthesis } {}

    [[nodiscard]] IdentifierList const& identifiers() const {
        return m_identifiers;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_left_parenthesis->source_location().join(m_right_parenthesis->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "EnumeratedTypeDefinition");
        context.print_children(m_identifiers);
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
    template<std::same_as<Token const> T>
    [[nodiscard]] explicit StructuredTypeDefinition(
        tl::optional<T&> const& packed,
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
        std::same_as<Token const> auto& array,
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

class RecordSection final : public AstNode {
private:
    IdentifierList m_identifiers;
    std::unique_ptr<Type> m_type;

public:
    [[nodiscard]] explicit RecordSection(IdentifierList identifiers, std::unique_ptr<Type> type)
        : m_identifiers{ std::move(identifiers) }, m_type{ std::move(type) } {}

    [[nodiscard]] IdentifierList const& identifiers() const {
        return m_identifiers;
    }

    [[nodiscard]] Type const& type() const {
        return *m_type;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_identifiers.source_location().join(m_type->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "RecordSection");
        context.print_children(m_identifiers, *m_type);
    }
};

class RecordFixedPart final : public AstNode {
private:
    std::vector<RecordSection> m_record_sections;

public:
    [[nodiscard]] explicit RecordFixedPart(std::vector<RecordSection> record_sections)
        : m_record_sections{ std::move(record_sections) } {
        if (m_record_sections.empty()) {
            throw InternalCompilerError{ "RecordFixedPart must have at least one record section." };
        }
    }

    [[nodiscard]] std::vector<RecordSection> const& record_sections() const {
        return m_record_sections;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_record_sections.front().source_location().join(m_record_sections.back().source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "RecordFixedPart");
        context.print_children(m_record_sections);
    }
};

class FieldList final : public AstNode {
private:
    tl::optional<RecordFixedPart> m_fixed_part;
    // TODO: Variant part.

public:
    [[nodiscard]] explicit FieldList(tl::optional<RecordFixedPart> fixed_part)
        : m_fixed_part{ std::move(fixed_part) } {}

    [[nodiscard]] tl::optional<RecordFixedPart> const& fixed_part() const {
        return m_fixed_part;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return join_source_locations(m_fixed_part);
    }

    void print(PrintContext& context) const override {
        context.print(*this, "FieldList");
        context.print_children(m_fixed_part);
    }
};

class RecordTypeDefinition final : public UnpackedStructuredTypeDefinition {
private:
    Token const* m_record;
    tl::optional<FieldList> m_field_list;
    Token const* m_end;

public:
    [[nodiscard]] explicit RecordTypeDefinition(
        std::same_as<Token const> auto& record,
        tl::optional<FieldList> field_list,
        std::same_as<Token const> auto& end
    )
        : m_record{ &record }, m_field_list{ std::move(field_list) }, m_end{ &end } {}

    [[nodiscard]] Token const& record() const {
        return *m_record;
    }

    [[nodiscard]] tl::optional<FieldList> const& field_list() const {
        return m_field_list;
    }

    [[nodiscard]] SourceLocation source_location() const override {
        return m_record->source_location().join(m_end->source_location());
    }

    void print(PrintContext& context) const override {
        context.print(*this, "RecordTypeDefinition");
        context.print_children(m_field_list);
    }
};

/*class SetTypeDefinition final : public UnpackedStructuredTypeDefinition {};

class FileTypeDefinition final : public UnpackedStructuredTypeDefinition {};
*/
