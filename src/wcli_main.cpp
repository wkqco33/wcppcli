#include "wcppcli/wcli.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace wcppcli;

// 템플릿: main.cpp (src/ 폴더에 위치)
const char* MAIN_TEMPLATE = R"(#include "wcppcli/wcli.hpp"
#include <iostream>

using namespace wcppcli;

int main(int argc, char** argv) {
    Command root;
    root.name = "{0}";
    root.description = "{0} CLI Application";

    // TODO: Add subcommands here
    // #include "cmd_example.hpp"
    // root.add_command(create_example_cmd());

    return root.execute(argc, argv);
}
)";

// 템플릿: CMakeLists.txt
const char* CMAKE_TEMPLATE = R"(cmake_minimum_required(VERSION 3.10)
project({0} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include 디렉토리 설정
include_directories(include)

# 소스 파일 목록 (commands 폴더 내의 모든 cpp 포함)
file(GLOB_RECURSE COMMAND_SOURCES src/commands/*.cpp)
set(SOURCES src/main.cpp ${{COMMAND_SOURCES}})

# wcppcli 라이브러리 연동 (사용자 환경에 맞게 라이브러리 경로 설정 필요)
# find_package(wcppcli REQUIRED) 

add_executable({0} ${{SOURCES}})
# target_link_libraries({0} PRIVATE wcppcli)
)";

// 템플릿: cmd.hpp
const char* CMD_HPP_TEMPLATE = R"(#pragma once
#include "wcppcli/wcli.hpp"
#include <memory>

std::unique_ptr<wcppcli::Command> create_{0}_cmd();
)";

// 템플릿: cmd.cpp
const char* CMD_CPP_TEMPLATE = R"(#include "{1}/cmd_{0}.hpp"
#include <iostream>

using namespace wcppcli;

std::unique_ptr<Command> create_{0}_cmd() {
    auto cmd = std::make_unique<Command>();
    cmd->name = "{0}";
    cmd->description = "{0} command description";
    cmd->handler = [](const Command& c) {
        std::cout << "{0} command executed!" << std::endl;
    };
    return cmd;
}
)";

std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

int main(int argc, char** argv) {
    Command root;
    root.name = "wcli";
    root.description = "WCPP-CLI Structured Code Generator";

    // init command
    auto init_cmd = std::make_unique<Command>();
    init_cmd->name = "init";
    init_cmd->description = "Initialize a structured wcppcli project";
    init_cmd->handler = [](const Command& cmd) {
        std::string proj_name = "myapp";
        if (!cmd.args.empty()) proj_name = cmd.args[0];

        // 폴더 구조 생성
        fs::create_directories("src/commands");
        fs::create_directories("include/" + proj_name);

        // CMakeLists.txt
        std::ofstream cmake_file("CMakeLists.txt");
        cmake_file << replace_all(CMAKE_TEMPLATE, "{0}", proj_name);

        // src/main.cpp
        std::ofstream main_file("src/main.cpp");
        main_file << replace_all(MAIN_TEMPLATE, "{0}", proj_name);

        std::cout << "Successfully initialized structured project: " << proj_name << std::endl;
        std::cout << "Directories created: src/commands, include/" << proj_name << std::endl;
    };

    // add command
    auto add_cmd = std::make_unique<Command>();
    add_cmd->name = "add";
    add_cmd->description = "Add a new subcommand to the structured project";
    add_cmd->handler = [](const Command& cmd) {
        if (cmd.args.empty()) {
            std::cerr << "Error: command name is required." << std::endl;
            return;
        }
        std::string cmd_name = cmd.args[0];

        // 프로젝트 이름 찾기 (include 하위 폴더명으로 추측)
        std::string proj_name = "myapp";
        if (fs::exists("include")) {
            for (const auto& entry : fs::directory_iterator("include")) {
                if (entry.is_directory()) {
                    proj_name = entry.path().filename().string();
                    break;
                }
            }
        }

        std::string hpp_path = "include/" + proj_name + "/cmd_" + cmd_name + ".hpp";
        std::string cpp_path = "src/commands/cmd_" + cmd_name + ".cpp";

        if (fs::exists(hpp_path) || fs::exists(cpp_path)) {
            std::cerr << "Error: Command files already exist." << std::endl;
            return;
        }

        // 헤더 파일 생성
        std::ofstream hpp_file(hpp_path);
        hpp_file << replace_all(CMD_HPP_TEMPLATE, "{0}", cmd_name);

        // 소스 파일 생성
        std::ofstream cpp_file(cpp_path);
        std::string cpp_content = replace_all(CMD_CPP_TEMPLATE, "{0}", cmd_name);
        cpp_content = replace_all(cpp_content, "{1}", proj_name);
        cpp_file << cpp_content;

        std::cout << "Successfully added command: " << cmd_name << std::endl;
        std::cout << "Created: " << hpp_path << std::endl;
        std::cout << "Created: " << cpp_path << std::endl;
    };

    root.add_command(std::move(init_cmd));
    root.add_command(std::move(add_cmd));

    return root.execute(argc, argv);
}
