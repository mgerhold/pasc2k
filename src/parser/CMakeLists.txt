add_library(parser
        include/parser/parser.hpp
        parser.cpp
        include/parser/block.hpp
        include/parser/label_declaration.hpp
        include/parser/integer_literal.hpp
        include/parser/parser_error.hpp
        include/parser/parser_note.hpp
)

target_include_directories(parser PUBLIC include)

target_link_libraries(parser
        PUBLIC
        lexer
        common
)
