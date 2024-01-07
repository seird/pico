add_library(crypto INTERFACE)

target_sources(crypto INTERFACE
    sts.cpp
    mac.c
    aes.c
    ecdh.c
    utils.c
)

target_link_libraries(crypto INTERFACE
    pico_rand
)

target_include_directories(crypto INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
