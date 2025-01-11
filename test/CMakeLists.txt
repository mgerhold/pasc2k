CPMAddPackage(
        NAME GTEST
        GITHUB_REPOSITORY google/googletest
        VERSION 1.15.2
        OPTIONS
        "INSTALL_GTEST OFF"
        "BUILD_SHARED_LIBS OFF"
)

add_executable(
        lexer_tests
        lexer_tests.cpp
)
target_link_libraries(
        lexer_tests
        PRIVATE
        lexer
)
target_link_system_libraries(lexer_tests
        PRIVATE
        gtest_main
        gmock_main
)

add_executable(
        parser_tests
        parser_tests.cpp
)
target_link_libraries(
        parser_tests
        PRIVATE
        parser
)
target_link_system_libraries(parser_tests
        PRIVATE
        gtest_main
        gmock_main
)

include(GoogleTest)
gtest_discover_tests(lexer_tests)
gtest_discover_tests(parser_tests)
