include(${PROJECT_SOURCE_DIR}/cmake/warnings.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/sanitizers.cmake)

# the following function was taken from:
# https://github.com/cpp-best-practices/cmake_template/blob/main/ProjectOptions.cmake
macro(check_sanitizer_support)
    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
        set(supports_ubsan ON)
    else ()
        set(supports_ubsan OFF)
    endif ()

    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
        set(supports_asan OFF)
    else ()
        set(supports_asan ON)
    endif ()
endmacro()

if (PROJECT_IS_TOP_LEVEL)
    option(pasc2k_warnings_as_errors "Treat warnings as errors" ON)
    option(pasc2k_enable_undefined_behavior_sanitizer "Enable undefined behavior sanitizer" ${supports_ubsan})
    option(pasc2k_enable_address_sanitizer "Enable address sanitizer" ${supports_asan})
    option(pasc2k_build_tests "Build unit tests" ON)
else ()
    option(pasc2k_warnings_as_errors "Treat warnings as errors" OFF)
    option(pasc2k_enable_undefined_behavior_sanitizer "Enable undefined behavior sanitizer" OFF)
    option(pasc2k_enable_address_sanitizer "Enable address sanitizer" OFF)
    option(pasc2k_build_tests "Build unit tests" OFF)
endif ()

add_library(pasc2k_warnings INTERFACE)
pasc2k_set_warnings(pasc2k_warnings ${pasc2k_warnings_as_errors})

add_library(pasc2k_sanitizers INTERFACE)
pasc2k_enable_sanitizers(pasc2k_sanitizers ${pasc2k_enable_address_sanitizer} ${pasc2k_enable_undefined_behavior_sanitizer})

add_library(pasc2k_project_options INTERFACE)
target_link_libraries(pasc2k_project_options
        INTERFACE pasc2k_warnings
        INTERFACE pasc2k_sanitizers
)
