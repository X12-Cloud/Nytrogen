cmake_minimum_required(VERSION 3.20)
project(Nytrogen VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Nytro
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
)