add_library(lexer
        include/lexer/lexer.hpp
        lexer.cpp
        include/lexer/token_type.hpp
        include/lexer/source_location.hpp
        include/lexer/token.hpp
        token_type.cpp
        include/lexer/lexer_error.hpp
)

target_include_directories(lexer
        PUBLIC
        include
)

target_link_libraries(lexer
        PUBLIC
        common
)
