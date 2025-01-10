add_library(common
        INTERFACE
        include/common/common.hpp
)

target_include_directories(common
        INTERFACE
        include
)

target_link_libraries(common
        INTERFACE
        pasc2k_project_options
)

target_link_system_libraries(common
        INTERFACE
        lib2k
        magic_enum::magic_enum
)
