#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <variant>

namespace wcppcli {

    struct Command;
    // 0 = 성공, 0이 아니면 프로세스 종료 코드로 그대로 전달됨.
    using CommandHandler = std::function<int(const Command&)>;

    class WConf;

    struct Flag {
        std::string name;
        char shorthand = 0;
        std::string description;
        using ValueType = std::variant<std::monostate, std::string*, int*, bool*>;
        ValueType value_ptr = std::monostate{}; // 가리키는 변수는 Command::execute() 호출 동안 살아있어야 함 (댕글링 포인터 주의)
        std::string config_key; // WConf와 연동할 키
        bool changed = false;
        bool required = false; // true인데 파싱 후에도 changed가 false면 execute()가 에러로 처리
    };

    struct Command {
        std::string name;
        std::string description;
        std::string usage;
        std::string version; // 비어있지 않으면 --version 플래그를 자동으로 처리
        CommandHandler handler;
        WConf* conf_ptr = nullptr; // 연동할 설정 인스턴스

        std::vector<std::unique_ptr<Command>> subcommands;
        std::vector<Flag> flags;
        std::vector<std::string> args; // Positional arguments

        void add_command(std::unique_ptr<Command> cmd) { subcommands.push_back(std::move(cmd)); }
        void add_flag(Flag flag) { flags.push_back(std::move(flag)); }
        void bind_config(WConf* conf) { conf_ptr = conf; }

        int execute(int argc, char** argv);
        void print_help() const;
        std::string generate_bash_completion() const;
    };

} // namespace wcppcli
