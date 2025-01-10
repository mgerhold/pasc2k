#include <cctype>
#include <lexer/token_type.hpp>
#include <magic_enum.hpp>

std::ostream& operator<<(std::ostream& os, TokenType const type) {
    for (auto const c : magic_enum::enum_name(type)) {
        os << std::toupper(static_cast<unsigned char>(c));
    }
    return os;
}
