include("${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/system_link.cmake")

function(obpf_simulator_setup_dependencies)
    CPMAddPackage(
            NAME SPDLOG
            GITHUB_REPOSITORY gabime/spdlog
            VERSION 1.14.1
            OPTIONS
            "SPDLOG_BUILD_EXAMPLE OFF"
            "SPDLOG_BUILD_TESTS OFF"
            "BUILD_SHARED_LIBS OFF"
    )
    CPMAddPackage(
            NAME MAGIC_ENUM
            GITHUB_REPOSITORY Neargye/magic_enum
            VERSION 0.9.6
    )
    CPMAddPackage(
            NAME LIB2K
            GITHUB_REPOSITORY mgerhold/lib2k
            VERSION 0.1.2
    )
endfunction()
