#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <variant>

namespace wcppcli {

    struct Command;
    using CommandHandler = std::function<void(const Command&)>;

    class WConf;

    struct Flag {
        std::string name;
        char shorthand = 0;
        std::string description;
        using ValueType = std::variant<std::monostate, std::string*, int*, bool*>;
        ValueType value_ptr = std::monostate{};
        std::string config_key; // WConf와 연동할 키
        bool changed = false;
    };

    struct Command {
        std::string name;
        std::string description;
        std::string usage;
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
