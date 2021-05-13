
cmake_minimum_required(VERSION 3.8)
project(dht)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-g)


include_directories(src)


add_library(bencoding src/bencode.cpp)

add_executable(dht 
    src/main.cpp
    src/network.cpp
    src/Krpc.cpp
    )

add_subdirectory(test)

target_link_libraries(dht Ws2_32 bencoding crypto)
