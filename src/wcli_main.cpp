#include "wcppcli/wcli.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <regex>

namespace fs = std::filesystem;
using namespace wcppcli;

// ─── 프로젝트 컨텍스트 ────────────────────────────────────────────────────────

struct ProjectContext {
    std::string proj_name;
    std::string src_cmd_dir;      // 커맨드 소스 위치 (e.g., "src/commands")
    std::string hdr_cmd_dir;      // 커맨드 헤더 위치 (e.g., "include/myapp")
    std::string hdr_include_path; // #include "..." 에 사용할 경로 (e.g., "myapp/commands")
    std::string file_prefix;      // 파일명 앞 (e.g., "cmd_")
    std::string file_suffix;      // 파일명 뒤 (e.g., "" or "_cmd")
    bool headers_with_source = false;
};

static std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

static std::string detect_proj_name() {
    // CMakeLists.txt 의 project() 에서 추출
    if (fs::exists("CMakeLists.txt")) {
        std::ifstream f("CMakeLists.txt");
        std::string line;
        std::regex re(R"(project\s*\(\s*(\w+))");
        while (std::getline(f, line)) {
            std::smatch m;
            if (std::regex_search(line, m, re)) return m[1].str();
        }
    }
    // include/ 첫 번째 서브폴더
    if (fs::exists("include")) {
        for (const auto& e : fs::directory_iterator("include"))
            if (e.is_directory()) return e.path().filename().string();
    }
    return "myapp";
}

static std::string detect_src_cmd_dir() {
    // 존재하는 디렉토리 중 우선순위 순으로 반환
    for (const char* dir : {"src/commands", "src/cmds", "commands", "cmds"})
        if (fs::exists(dir) && fs::is_directory(dir)) return dir;
    return "src/commands";
}

// 기존 .cpp 파일의 #include 디렉티브에서 헤더 경로 추출
static std::string detect_include_path_from_sources(const std::string& src_dir) {
    if (!fs::exists(src_dir)) return "";
    std::regex re(R"re(#include\s+"([^"]+\.hpp)")re");
    for (const auto& e : fs::directory_iterator(src_dir)) {
        if (e.path().extension() != ".cpp") continue;
        std::ifstream f(e.path());
        std::string line;
        while (std::getline(f, line)) {
            std::smatch m;
            if (std::regex_search(line, m, re)) {
                std::string inc = m[1].str();
                auto slash = inc.rfind('/');
                if (slash != std::string::npos) return inc.substr(0, slash);
                // 슬래시 없음 → 헤더가 소스와 같은 위치
                return "";
            }
        }
    }
    return "";
}

// 기존 파일명에서 prefix/suffix 감지
static std::pair<std::string, std::string> detect_naming(const std::string& src_dir) {
    if (!fs::exists(src_dir)) return {"cmd_", ""};
    for (const auto& e : fs::directory_iterator(src_dir)) {
        if (e.path().extension() != ".cpp") continue;
        std::string stem = e.path().stem().string();
        if (stem.rfind("cmd_", 0) == 0)     return {"cmd_", ""};
        if (stem.rfind("command_", 0) == 0)  return {"command_", ""};
        if (stem.size() > 4 && stem.substr(stem.size() - 4) == "_cmd")     return {"", "_cmd"};
        if (stem.size() > 8 && stem.substr(stem.size() - 8) == "_command") return {"", "_command"};
        // 알 수 없는 패턴 → prefix/suffix 없음
        return {"", ""};
    }
    return {"cmd_", ""};
}

static ProjectContext detect_project_context() {
    ProjectContext ctx;
    ctx.proj_name   = detect_proj_name();
    ctx.src_cmd_dir = detect_src_cmd_dir();

    auto [prefix, suffix] = detect_naming(ctx.src_cmd_dir);
    ctx.file_prefix = prefix;
    ctx.file_suffix = suffix;

    // 기존 소스 파일에서 include path 파싱
    std::string detected_inc = detect_include_path_from_sources(ctx.src_cmd_dir);

    if (!detected_inc.empty()) {
        ctx.hdr_include_path = detected_inc;
        ctx.hdr_cmd_dir = "include/" + detected_inc;
    } else {
        // 존재하는 헤더 디렉토리 탐색
        std::vector<std::string> hdr_candidates = {
            "include/" + ctx.proj_name + "/commands",
            "include/" + ctx.proj_name + "/cmds",
            "include/" + ctx.proj_name,
        };
        bool found = false;
        for (const auto& dir : hdr_candidates) {
            if (fs::exists(dir)) {
                ctx.hdr_cmd_dir      = dir;
                ctx.hdr_include_path = dir.substr(8); // "include/" 제거
                found = true;
                break;
            }
        }
        if (!found) {
            // 소스 디렉토리에 .hpp 파일이 있으면 headers_with_source
            bool has_hpp = false;
            if (fs::exists(ctx.src_cmd_dir)) {
                for (const auto& e : fs::directory_iterator(ctx.src_cmd_dir))
                    if (e.path().extension() == ".hpp") { has_hpp = true; break; }
            }
            if (has_hpp) {
                ctx.headers_with_source = true;
                ctx.hdr_cmd_dir         = ctx.src_cmd_dir;
                ctx.hdr_include_path    = "";
            } else {
                ctx.hdr_cmd_dir      = "include/" + ctx.proj_name;
                ctx.hdr_include_path = ctx.proj_name;
            }
        }
    }

    return ctx;
}

// ─── 템플릿 ──────────────────────────────────────────────────────────────────

const char* MAIN_TEMPLATE = R"(#include "wcppcli/wcli.hpp"
#include <iostream>

using namespace wcppcli;

int main(int argc, char** argv) {
    Command root;
    root.name = "{proj}";
    root.description = "{proj} CLI Application";

    // TODO: Add subcommands here
    // #include "{proj}/cmd_example.hpp"
    // root.add_command(create_example_cmd());

    return root.execute(argc, argv);
}
)";

const char* CMAKE_TEMPLATE = R"(cmake_minimum_required(VERSION 3.10)
project({proj} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE COMMAND_SOURCES src/commands/*.cpp)
set(SOURCES src/main.cpp ${COMMAND_SOURCES})

add_executable({proj} ${SOURCES})
# target_link_libraries({proj} PRIVATE wcppcli)
)";

// {hdr}  = 헤더 include 전체 경로 (e.g., "myapp/cmd_foo.hpp" or "cmd_foo.hpp")
// {file} = 파일명 stem (e.g., "cmd_foo")
// {fn}   = 함수명 (e.g., "create_foo_cmd")
// {cmd}  = 커맨드명
const char* CMD_HPP_TEMPLATE = R"(#pragma once
#include "wcppcli/wcli.hpp"
#include <memory>

std::unique_ptr<wcppcli::Command> {fn}();
)";

const char* CMD_CPP_TEMPLATE = R"(#include "{hdr}"
#include <iostream>

using namespace wcppcli;

std::unique_ptr<Command> {fn}() {
    auto cmd = std::make_unique<Command>();
    cmd->name = "{cmd}";
    cmd->description = "{cmd} command description";
    cmd->handler = [](const Command& c) {
        std::cout << "{cmd} command executed!" << std::endl;
    };
    return cmd;
}
)";

// ─── 헬퍼 ────────────────────────────────────────────────────────────────────

static std::string apply_cmd_template(const char* tmpl,
                                       const std::string& hdr,
                                       const std::string& file,
                                       const std::string& fn,
                                       const std::string& cmd) {
    std::string s = tmpl;
    s = replace_all(s, "{hdr}",  hdr);
    s = replace_all(s, "{file}", file);
    s = replace_all(s, "{fn}",   fn);
    s = replace_all(s, "{cmd}",  cmd);
    return s;
}

static void write_or_preview(const std::string& path, const std::string& content, bool dry_run) {
    if (dry_run) {
        std::cout << "\n[preview] " << path << ":\n";
        std::cout << std::string(44, '-') << "\n" << content << std::string(44, '-') << "\n";
    } else {
        std::ofstream f(path);
        f << content;
        std::cout << "  Created: " << path << "\n";
    }
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    Command root;
    root.name = "wcli";
    root.description = "WCPP-CLI Structured Code Generator";

    // ── init ──────────────────────────────────────────────────────────────────
    auto init_cmd = std::make_unique<Command>();
    init_cmd->name = "init";
    init_cmd->description = "Initialize a structured wcppcli project";
    init_cmd->handler = [](const Command& cmd) {
        std::string proj = cmd.args.empty() ? "myapp" : cmd.args[0];

        fs::create_directories("src/commands");
        fs::create_directories("include/" + proj);

        {
            std::ofstream f("CMakeLists.txt");
            f << replace_all(CMAKE_TEMPLATE, "{proj}", proj);
        }
        {
            std::ofstream f("src/main.cpp");
            f << replace_all(MAIN_TEMPLATE, "{proj}", proj);
        }

        std::cout << "Initialized: " << proj << "\n";
        std::cout << "  src/commands/\n";
        std::cout << "  include/" << proj << "/\n";
        std::cout << "  CMakeLists.txt\n";
        std::cout << "  src/main.cpp\n";
    };

    // ── add ───────────────────────────────────────────────────────────────────
    auto add_cmd = std::make_unique<Command>();
    add_cmd->name = "add";
    add_cmd->description = "Add a new subcommand (auto-detects project structure)";

    bool dry_run = false;
    Flag dry_flag;
    dry_flag.name        = "dry-run";
    dry_flag.shorthand   = 'n';
    dry_flag.description = "Show what would be created without making any changes";
    dry_flag.value_ptr   = &dry_run;
    add_cmd->add_flag(dry_flag);

    add_cmd->handler = [&dry_run](const Command& cmd) {
        if (cmd.args.empty()) {
            std::cerr << "Error: command name required.\n";
            return;
        }
        const std::string cmd_name = cmd.args[0];
        ProjectContext ctx = detect_project_context();

        std::string file_stem = ctx.file_prefix + cmd_name + ctx.file_suffix;
        std::string fn_name   = "create_" + cmd_name + "_cmd";
        std::string hdr_inc   = ctx.headers_with_source
                                    ? (file_stem + ".hpp")
                                    : (ctx.hdr_include_path + "/" + file_stem + ".hpp");

        std::string hpp_path = ctx.hdr_cmd_dir + "/" + file_stem + ".hpp";
        std::string cpp_path = ctx.src_cmd_dir + "/" + file_stem + ".cpp";

        std::cout << "Project : " << ctx.proj_name << "\n";
        std::cout << "src dir : " << ctx.src_cmd_dir << "\n";
        std::cout << "hdr dir : " << ctx.hdr_cmd_dir << "\n";
        std::cout << "naming  : " << file_stem << ".{hpp,cpp}\n";
        std::cout << "include : #include \"" << hdr_inc << "\"\n";
        if (dry_run) std::cout << "[dry-run mode - no files will be created]\n";
        std::cout << "\n";

        if (!dry_run) {
            if (fs::exists(hpp_path) || fs::exists(cpp_path)) {
                std::cerr << "Error: files already exist.\n";
                return;
            }
            fs::create_directories(ctx.src_cmd_dir);
            if (!ctx.headers_with_source) fs::create_directories(ctx.hdr_cmd_dir);
        }

        std::string hpp_content = apply_cmd_template(CMD_HPP_TEMPLATE, hdr_inc, file_stem, fn_name, cmd_name);
        std::string cpp_content = apply_cmd_template(CMD_CPP_TEMPLATE, hdr_inc, file_stem, fn_name, cmd_name);

        write_or_preview(hpp_path, hpp_content, dry_run);
        write_or_preview(cpp_path, cpp_content, dry_run);

        if (!dry_run) {
            std::cout << "\nAdd to main.cpp:\n";
            std::cout << "  #include \"" << hdr_inc << "\"\n";
            std::cout << "  root.add_command(" << fn_name << "());\n";
        }
    };

    root.add_command(std::move(init_cmd));
    root.add_command(std::move(add_cmd));

    return root.execute(argc, argv);
}
