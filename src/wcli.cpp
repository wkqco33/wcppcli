#include "wcppcli/wcli.hpp"
#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>
#include <iomanip>

namespace wcppcli {

    static bool is_flag(std::string_view arg) { return arg.size() > 1 && arg[0] == '-'; }

    int Command::execute(int argc, char** argv) {
        std::vector<std::string> raw_args;
        for (int i = 1; i < argc; ++i) raw_args.push_back(argv[i]);

        Command* current = this;
        size_t idx = 0;

        while (idx < raw_args.size()) {
            std::string_view arg = raw_args[idx];
            if (arg == "--help" || arg == "-h") { current->print_help(); return 0; }

            if (is_flag(arg)) {
                bool found = false;
                std::string name, val;
                bool has_val = false;

                if (arg.substr(0, 2) == "--") {
                    name = arg.substr(2);
                    size_t pos = name.find('=');
                    if (pos != std::string::npos) { val = name.substr(pos + 1); name = name.substr(0, pos); has_val = true; }
                } else { name = arg.substr(1); }

                for (auto& f : current->flags) {
                    if (f.name == name || (f.shorthand != 0 && std::string(1, f.shorthand) == name)) {
                        found = true;
                        if (std::holds_alternative<bool*>(f.value_ptr)) {
                            *std::get<bool*>(f.value_ptr) = true;
                            f.changed = true;
                            if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, true);
                        } else {
                            if (!has_val && idx + 1 < raw_args.size()) { val = raw_args[++idx]; has_val = true; }
                            if (has_val) {
                                if (std::holds_alternative<std::string*>(f.value_ptr)) {
                                    *std::get<std::string*>(f.value_ptr) = val;
                                    if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, val);
                                } else if (std::holds_alternative<int*>(f.value_ptr)) {
                                    int iv = std::stoi(val);
                                    *std::get<int*>(f.value_ptr) = iv;
                                    if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, iv);
                                } else if (std::holds_alternative<std::monostate>(f.value_ptr)) {
                                     if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, val);
                                }
                                f.changed = true;
                            } else if (std::holds_alternative<std::monostate>(f.value_ptr)) {
                                // 인자 없는 단독 플래그인 경우 (true로 간주하거나 그냥 changed만 표시)
                                f.changed = true;
                                if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, true);
                            }
                        }
                        break;
                    }
                }
                if (!found) { std::cerr << "Unknown flag: " << arg << std::endl; return 1; }
            } else {
                bool sub_found = false;
                for (auto& cmd : current->subcommands) {
                    if (cmd->name == arg) {
                        if (!cmd->conf_ptr) cmd->conf_ptr = current->conf_ptr; // 전파
                        current = cmd.get();
                        sub_found = true;
                        break;
                    }
                }
                if (!sub_found) { current->args.push_back(std::string(arg)); }
            }
            idx++;
        }

        if (current->handler) { current->handler(*current); return 0; }
        else { current->print_help(); return 0; }
    }

    void Command::print_help() const {
        print(name, Style(Color::Yellow, Color::None, true));
        if (!description.empty()) std::cout << description << std::endl << std::endl;
        print("Usage:", Style(Color::Green, Color::None, true));
        std::cout << "  " << (usage.empty() ? (name + " [command] [flags] [args]") : usage) << std::endl << std::endl;
        if (!subcommands.empty()) {
            print("Available Commands:", Style(Color::Green, Color::None, true));
            for (const auto& cmd : subcommands) std::cout << "  " << std::left << std::setw(15) << cmd->name << " " << cmd->description << std::endl;
            std::cout << std::endl;
        }
        if (!flags.empty()) {
            print("Flags:", Style(Color::Green, Color::None, true));
            for (const auto& f : flags) {
                std::string info = "  ";
                if (f.shorthand != 0) { info += "-"; info += f.shorthand; info += ", "; }
                info += "--"; info += f.name;
                std::cout << std::left << std::setw(25) << info << " " << f.description << std::endl;
            }
            std::cout << std::endl;
        }
    }

    std::string Command::generate_bash_completion() const {
        std::string script = "_" + name + "_completion() {\n";
        script += "    local cur opts\n";
        script += "    COMPREPLY=()\n";
        script += "    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n";
        
        std::string opts;
        for (const auto& cmd : subcommands) opts += cmd->name + " ";
        for (const auto& f : flags) {
            opts += "--" + f.name + " ";
            if (f.shorthand != 0) {
                opts += "-";
                opts += f.shorthand;
                opts += " ";
            }
        }
        
        script += "    opts=\"" + opts + "\"\n";
        script += "    COMPREPLY=( $(compgen -W \"${opts}\" -- \"${cur}\") )\n";
        script += "    return 0\n";
        script += "}\n";
        script += "complete -F _" + name + "_completion " + name + "\n";
        return script;
    }

} // namespace wcppcli
