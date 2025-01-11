add_library(diagnostics
        include/diagnostics/diagnostics.hpp
        diagnostics.cpp
        colors.hpp
)

target_include_directories(diagnostics PUBLIC include)

target_link_libraries(diagnostics
        PUBLIC
        common
        lexer
        parser
)
