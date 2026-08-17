cmake_minimum_required(VERSION 3.10)
project(wcppcli VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

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
option(WCPPCLI_BUILD_TESTS "Build wcppcli unit tests" ${WCPPCLI_IS_TOP_LEVEL})
option(WCPPCLI_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# Library
add_library(wcppcli
    src/wstyle.cpp
    src/wcli.cpp
    src/wconf.cpp
    src/wui.cpp
    src/wlog.cpp
)

# C++17 표준 요구사항을 소비자에게 전파 (전역 CMAKE_CXX_STANDARD 에만 의존하지 않음).
target_compile_features(wcppcli PUBLIC cxx_std_17)

# 경고 플래그: 라이브러리와 테스트/예제/툴에 일관 적용.
function(wcppcli_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(WCPPCLI_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(WCPPCLI_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

wcppcli_enable_warnings(wcppcli)

if(WIN32)
    set_target_properties(wcppcli PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_include_directories(wcppcli PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(WCPPCLI_BUILD_TOOL)
    # CLI Generator Tool
    add_executable(wcli src/wcli_main.cpp)
    target_link_libraries(wcli PRIVATE wcppcli)
    wcppcli_enable_warnings(wcli)
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

foreach(example_target example_full example_formats example_nested example_ensure
                      example_priority example_schema example_interactive example_log
                      example_completion)
    wcppcli_enable_warnings(${example_target})
endforeach()

if(WCPPCLI_BUILD_TESTS)
    enable_testing()
    add_executable(wcppcli_tests
        tests/test_main.cpp
        tests/test_wconf.cpp
        tests/test_wcli.cpp
        tests/test_wstyle.cpp
        tests/test_wlog.cpp
    )
    target_include_directories(wcppcli_tests PRIVATE tests)
    target_link_libraries(wcppcli_tests PRIVATE wcppcli)
    wcppcli_enable_warnings(wcppcli_tests)
    add_test(NAME wcppcli_tests COMMAND wcppcli_tests)
    set_tests_properties(wcppcli_tests PROPERTIES LABELS "unit")
endif()

install(TARGETS wcppcli
    EXPORT wcppcliTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/wcppcliConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/wcppcliConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/wcppcliConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/wcppcli"
)

install(EXPORT wcppcliTargets
    FILE wcppcliTargets.cmake
    NAMESPACE wcppcli::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/wcppcli"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/wcppcliConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/wcppcliConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/wcppcli"
)
