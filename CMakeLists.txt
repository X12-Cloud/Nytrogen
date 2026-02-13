cmake_minimum_required(VERSION 3.10)

# Version tree
set(NYTRO_VERSION_MAJOR 0)
set(NYTRO_VERSION_MINOR 1)
set(NYTRO_VERSION_SNAPSHOT 0)
set(NYTRO_VERSION_PATCH 0)
set(NYTRO_VERSION_SUFFIX "-beta")

# Final version
set(NYTRO_FULL_VERSION "${NYTRO_VERSION_MAJOR}.${NYTRO_VERSION_MINOR}${NYTRO_VERSION_SNAPSHOT}${NYTRO_VERSION_PATCH}${NYTRO_VERSION_SUFFIX}")

# Init
project(Nytrogen VERSION ${NYTRO_VERSION_MAJOR}.${NYTRO_VERSION_MINOR})
set(CMAKE_CXX_STANDARD 17)

# Set output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Global macro/var
add_compile_definitions(NYTRO_VERSION="\\"${NYTRO_FULL_VERSION}\\"")

# Components
add_subdirectory(Preprocessor)
add_subdirectory(bootstrap)
add_subdirectory(driver)
