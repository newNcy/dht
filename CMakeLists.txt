
cmake_minimum_required(VERSION 3.8)
project(dht)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-g)


add_executable(dht 
    src/main.cpp
    src/network.cpp
    src/bencode.cpp
    )

target_link_libraries(dht Ws2_32)
