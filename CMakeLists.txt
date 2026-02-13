cmake_minimum_required(VERSION 3.10)
project(Nytrogen)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Componenets
add_subdirectory(Preprocessor)
add_subdirectory(bootstrap)
add_subdirectory(driver)
