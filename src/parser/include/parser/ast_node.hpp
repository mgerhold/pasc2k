#pragma once

#include <lexer/source_location.hpp>
#include <lib2k/types.hpp>
#include <print>
#include <vector>

template<typename T, typename Contained>
concept IsOptional = requires(T&& t) {
    { t.has_value() } -> std::same_as<bool>;
    { t.value() } -> std::convertible_to<Contained>;
};

class AstNode;

template<typename T>
concept PrintableChild = std::derived_from<T, AstNode> or IsOptional<T, AstNode const&>;

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
    private:
        std::vector<bool> indents;
        bool just_indented = false;
        bool is_last_child = false;

    public:
        template<typename... Ts>
        void print(AstNode const& node, std::string_view const name, Ts const&... args) {
            print_indentation();
            std::print("{} [{}, {}]", name, node.source_location(), node.source_location().end());
            if constexpr (sizeof...(args) > 0) {
                (std::print(" '{}'", args), ...);
            }
            std::print("\n");
        }

        void print_children(PrintableChild auto const&... children) {
            auto vector = std::vector<AstNode const*>{};

            // clang-format off
            ([&] {
                if constexpr (IsOptional<decltype(children), AstNode const&>) {
                    if (children.has_value()) {
                        vector.push_back(static_cast<AstNode const*>(&children.value()));
                    }
                } else {
                    vector.push_back(&children);
                }
            }(), ...);
            // clang-format on
            print_children(vector);
        }

        template<std::derived_from<AstNode> T>
        void print_children(std::vector<T> const& children) {
            using std::views::transform, std::views::filter, std::ranges::to;
            // clang-format off
            print_children(
                children
                    | transform([](auto const& child) { return static_cast<AstNode const*>(&child); })
                    | to<std::vector>()
            );
            // clang-format on
        }

        void print_children(std::vector<AstNode const*> const children) {
            if (children.empty()) {
                return;
            }

            begin_children(children.size() == 1);
            for (auto i = usize{ 0 }; i < children.size(); ++i) {
                if (i == children.size() - 1) {
                    is_last_child = true;
                }
                children[i]->print(*this);
            }
            end_children();
        }

    private:
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
            if (indents.back() or is_last_child) {
                std::print("`-");
                if (is_last_child) {
                    indents.back() = true;
                }
            } else {
                std::print("|-");
            }
            just_indented = false;
            is_last_child = false;
        }
    };

    virtual void print(PrintContext& context) const = 0;
};
