add_library(parser
        include/parser/parser.hpp
        parser.cpp
        include/parser/block.hpp
        include/parser/label_declaration.hpp
        include/parser/parser_error.hpp
        include/parser/parser_note.hpp
        include/parser/constant_definition.hpp
        include/parser/type_definition.hpp
        include/parser/literals.hpp
        include/parser/ast_node.hpp
        include/parser/ast.hpp
        include/parser/label_declarations.hpp
        include/parser/constant_definitions.hpp
        include/parser/type_definitions.hpp
        include/parser/identifier.hpp
        type_definition.cpp
        include/parser/variable_declarations.hpp
        include/parser/variable_declaration.hpp
        include/parser/identifier_list.hpp
)

target_include_directories(parser PUBLIC include)

target_link_libraries(parser
        PUBLIC
        lexer
        common
)
