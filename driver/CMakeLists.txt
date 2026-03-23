cmake_minimum_required(VERSION 3.10)
project(NytroDriver)

find_package(Lua REQUIRED) # load lua

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "src/*.cpp")

include_directories(include)
include_directories(src)

add_executable(nytro-driver ${SOURCES})

target_include_directories(nytro-driver PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${LUA_INCLUDE_DIR}
)

target_link_libraries(nytro-driver ${LUA_LIBRARIES})
