

project(dht)


set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_definitions(-g -std=c99)
include_directories(src)


add_executable(dht 
	src/dht.c
	src/krpc.c
	)

target_link_libraries(dht crypto pthread)
