cmake_minimum_required(VERSION 3.10)
project(NytrogenCompiler)

set(CMAKE_CXX_STANDARD 17)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Include headers
include_directories(include)

add_executable(Nytro ${SOURCES})

