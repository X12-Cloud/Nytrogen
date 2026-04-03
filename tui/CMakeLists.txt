find_package(ftxui REQUIRED)

file(GLOB SOURCES "src/*.cpp")

include_directories(include)
include_directories(src)

add_executable(nytro-tui ${SOURCES})

target_link_libraries(nytro-tui 
    PRIVATE ftxui::screen 
    PRIVATE ftxui::dom 
    PRIVATE ftxui::component
)
