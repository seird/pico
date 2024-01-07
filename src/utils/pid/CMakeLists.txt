add_library(pid INTERFACE)

target_sources(pid INTERFACE
    pid.c
)

target_link_libraries(pid INTERFACE
    pico_stdlib
)

target_include_directories(pid INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
