#pragma once

#include <lexer/source_location.hpp>
#include <lib2k/types.hpp>
#include <print>
#include <vector>

class AstNode {
public:
    AstNode() = default;
    AstNode(AstNode const& other) = default;
    AstNode(AstNode&& other) noexcept = default;
    AstNode& operator=(AstNode const& other) = default;
    AstNode& operator=(AstNode&& other) noexcept = default;
    virtual ~AstNode() = default;

    [[nodiscard]] virtual SourceLocation source_location() const = 0;

    struct PrintContext {
        std::vector<bool> indents;
        bool just_indented = false;

        void indent(bool const only_one_child) {
            indents.push_back(only_one_child);
            just_indented = true;
        }

        void dedent() {
            indents.pop_back();
            just_indented = false;
        }

        void begin_children(bool const only_one_child) {
            indent(only_one_child);
        }

        void end_children() {
            dedent();
        }

        void print_indentation() {
            if (indents.empty()) {
                return;
            }
            for (auto i = usize{ 0 }; i < indents.size() - 1; ++i) {
                if (indents.at(i)) {
                    std::print("  ");
                } else {
                    std::print("| ");
                }
            }
            if (indents.back()) {
                std::print("`-");
            } else {
                std::print("|-");
            }
            just_indented = false;
        }
    };

    virtual void print(PrintContext& context) const = 0;

    template<typename... Ts>
    void print_ast_node(PrintContext& context, std::string_view const name, Ts const&... args) const {
        context.print_indentation();
        std::print("{} [{}, {}]", name, source_location(), source_location().end());
        if constexpr (sizeof...(args) > 0) {
            (std::print(" {}", args), ...);
        }
        std::print("\n");
    }
};
