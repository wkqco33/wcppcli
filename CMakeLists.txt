cmake_minimum_required(VERSION 3.10)
project(wcppcli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(WCPPCLI_IS_TOP_LEVEL ON)
else()
    set(WCPPCLI_IS_TOP_LEVEL OFF)
endif()

# Consumers pulling wcppcli in via FetchContent/add_subdirectory only want the
# library target; these options keep the examples and the wcli tool from
# being built (and their target names from leaking) into the parent project.
option(WCPPCLI_BUILD_EXAMPLES "Build wcppcli example executables" ${WCPPCLI_IS_TOP_LEVEL})
option(WCPPCLI_BUILD_TOOL "Build the wcli code-gen tool executable" ${WCPPCLI_IS_TOP_LEVEL})

# Library
add_library(wcppcli
    src/wstyle.cpp
    src/wcli.cpp
    src/wconf.cpp
    src/wui.cpp
    src/wlog.cpp
)

target_include_directories(wcppcli PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WCPPCLI_BUILD_TOOL)
    # CLI Generator Tool
    add_executable(wcli src/wcli_main.cpp)
    target_link_libraries(wcli PRIVATE wcppcli)
endif()

if(WCPPCLI_BUILD_EXAMPLES)
    # Example - 모든 기능을 통합한 예제
    add_executable(example_full examples/full_example.cpp)
    target_link_libraries(example_full PRIVATE wcppcli)

    add_executable(example_formats examples/config_formats_example.cpp)
    target_link_libraries(example_formats PRIVATE wcppcli)

    add_executable(example_nested examples/nested_config_example.cpp)
    target_link_libraries(example_nested PRIVATE wcppcli)

    add_executable(example_ensure examples/ensure_file_example.cpp)
    target_link_libraries(example_ensure PRIVATE wcppcli)

    add_executable(example_priority examples/priority_merge_example.cpp)
    target_link_libraries(example_priority PRIVATE wcppcli)

    add_executable(example_schema examples/schema_validation_example.cpp)
    target_link_libraries(example_schema PRIVATE wcppcli)

    add_executable(example_interactive examples/interactive_example.cpp)
    target_link_libraries(example_interactive PRIVATE wcppcli)

    add_executable(example_log examples/log_example.cpp)
    target_link_libraries(example_log PRIVATE wcppcli)

    add_executable(example_completion examples/completion_example.cpp)
    target_link_libraries(example_completion PRIVATE wcppcli)
endif()
