cmake_minimum_required(VERSION 3.10)
project(NytrogenCompiler)

set(CMAKE_CXX_STANDARD 17)

# Source files
set(COMPILER_SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantic_analyzer.cpp
    src/code_generator.cpp
    src/instruction_set/basic.cpp
    src/instruction_set/advanced.cpp
    src/instruction_set/logical.cpp
    src/register_allocator.cpp
)

add_executable(nytro-c ${COMPILER_SOURCES})

# Include headers
target_include_directories(nytro-c PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# for pacman/aur
install(TARGETS nytro-c DESTINATION bin)
