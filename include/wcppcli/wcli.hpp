#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <variant>

namespace wcppcli {

    struct Command;
    using CommandHandler = std::function<void(const Command&)>;

    struct Flag {
        std::string name;
        char shorthand = 0;
        std::string description;
        using ValueType = std::variant<std::string*, int*, bool*>;
        ValueType value_ptr;
        bool changed = false;
    };

    struct Command {
        std::string name;
        std::string description;
        std::string usage;
        CommandHandler handler;

        std::vector<std::unique_ptr<Command>> subcommands;
        std::vector<Flag> flags;
        std::vector<std::string> args; // Positional arguments

        void add_command(std::unique_ptr<Command> cmd) { subcommands.push_back(std::move(cmd)); }
        void add_flag(Flag flag) { flags.push_back(std::move(flag)); }

        int execute(int argc, char** argv);
        void print_help() const;
    };

} // namespace wcppcli
