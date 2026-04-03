cmake_minimum_required(VERSION 3.10)
project(wcppcli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Library
add_library(wcppcli
    src/wstyle.cpp
    src/wcli.cpp
    src/wconf.cpp
)

target_include_directories(wcppcli PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Example - 모든 기능을 통합한 예제
add_executable(example_full examples/full_example.cpp)
target_link_libraries(example_full PRIVATE wcppcli)

add_executable(example_formats examples/config_formats_example.cpp)
target_link_libraries(example_formats PRIVATE wcppcli)

add_executable(example_nested examples/nested_config_example.cpp)
target_link_libraries(example_nested PRIVATE wcppcli)

add_executable(example_ensure examples/ensure_file_example.cpp)
target_link_libraries(example_ensure PRIVATE wcppcli)
