#include "wcppcli/wcli.hpp"
#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include "wcppcli/wlog.hpp"
#include <iostream>

namespace wcppcli {

    static bool is_flag(std::string_view arg) { return arg.size() > 1 && arg[0] == '-'; }

    // string*/int*/monostate 플래그에 값을 대입하고 WConf 오버라이드까지 반영. 실패 시 false.
    static bool assign_flag_value(Command* current, Flag& f, const std::string& val) {
        if (std::holds_alternative<std::string*>(f.value_ptr)) {
            *std::get<std::string*>(f.value_ptr) = val;
            if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, val);
        } else if (std::holds_alternative<int*>(f.value_ptr)) {
            try {
                int iv = std::stoi(val);
                *std::get<int*>(f.value_ptr) = iv;
                if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, iv);
            } catch (const std::exception&) {
                WLog::error("invalid integer value for --" + f.name + ": \"" + val + "\"");
                return false;
            }
        } else if (std::holds_alternative<std::monostate>(f.value_ptr)) {
            if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, val);
        }
        f.changed = true;
        return true;
    }

    int Command::execute(int argc, char** argv) {
      try {
        std::vector<std::string> raw_args;
        for (int i = 1; i < argc; ++i) raw_args.push_back(argv[i]);

        Command* current = this;
        size_t idx = 0;

        while (idx < raw_args.size()) {
            std::string_view arg = raw_args[idx];
            if (arg == "--help" || arg == "-h") { current->print_help(); return 0; }
            if (arg == "--version" && !current->version.empty()) { std::cout << current->version << std::endl; return 0; }

            if (is_flag(arg) && arg.substr(0, 2) == "--") {
                bool found = false;
                std::string name = std::string(arg.substr(2));
                std::string val;
                bool has_val = false;
                size_t pos = name.find('=');
                if (pos != std::string::npos) { val = name.substr(pos + 1); name = name.substr(0, pos); has_val = true; }

                for (auto& f : current->flags) {
                    if (f.name != name) continue;
                    found = true;
                    if (std::holds_alternative<bool*>(f.value_ptr)) {
                        *std::get<bool*>(f.value_ptr) = true;
                        f.changed = true;
                        if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, true);
                    } else {
                        if (!has_val && idx + 1 < raw_args.size()) { val = raw_args[++idx]; has_val = true; }
                        if (has_val) {
                            if (!assign_flag_value(current, f, val)) return 1;
                        } else if (std::holds_alternative<std::monostate>(f.value_ptr)) {
                            // 인자 없는 단독 플래그인 경우 (true로 간주하거나 그냥 changed만 표시)
                            f.changed = true;
                            if (current->conf_ptr && !f.config_key.empty()) current->conf_ptr->set_cli(f.config_key, true);
                        }
                    }
                    break;
                }
                if (!found) { WLog::error("unknown flag: " + std::string(arg)); return 1; }
            } else if (is_flag(arg)) {
                // 단일 '-' 플래그: shorthand 1개(-p) 또는 조합/인라인 값(-abc, -p8080) 형태를 모두 처리.
                // bool 플래그는 조합해서 이어붙일 수 있고, 값이 필요한 플래그를 만나면 그 뒤 남은
                // 문자 전체(없으면 다음 인자)를 값으로 소비하고 해당 인자 처리를 끝낸다.
                std::string chars = std::string(arg.substr(1));
                for (size_t ci = 0; ci < chars.size(); ++ci) {
                    char c = chars[ci];
                    Flag* matched = nullptr;
                    for (auto& f : current->flags) {
                        if (f.shorthand == c) { matched = &f; break; }
                    }
                    if (!matched) {
                        WLog::error("unknown flag: -" + std::string(1, c) + " (in " + std::string(arg) + ")");
                        return 1;
                    }
                    if (std::holds_alternative<bool*>(matched->value_ptr)) {
                        *std::get<bool*>(matched->value_ptr) = true;
                        matched->changed = true;
                        if (current->conf_ptr && !matched->config_key.empty()) current->conf_ptr->set_cli(matched->config_key, true);
                        continue;
                    }
                    std::string val = chars.substr(ci + 1);
                    bool has_val = !val.empty();
                    if (!has_val && idx + 1 < raw_args.size()) { val = raw_args[++idx]; has_val = true; }
                    if (has_val) {
                        if (!assign_flag_value(current, *matched, val)) return 1;
                    } else if (std::holds_alternative<std::monostate>(matched->value_ptr)) {
                        matched->changed = true;
                        if (current->conf_ptr && !matched->config_key.empty()) current->conf_ptr->set_cli(matched->config_key, true);
                    }
                    break; // 값을 소비했으므로 이 인자의 나머지 문자는 처리하지 않음
                }
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

        for (const auto& f : current->flags) {
            if (f.required && !f.changed) {
                WLog::error("required flag --" + f.name + " not set");
                return 1;
            }
        }

        if (current->handler) { return current->handler(*current); }
        else { current->print_help(); return 0; }
      } catch (const std::exception& e) {
        WLog::error(e.what());
        return 1;
      }
    }

    void Command::print_help() const {
        print(name, Style(Color::Yellow, Color::None, true));
        if (!description.empty()) std::cout << description << std::endl << std::endl;
        print("Usage:", Style(Color::Green, Color::None, true));
        std::cout << "  " << (usage.empty() ? (name + " [command] [flags] [args]") : usage) << std::endl << std::endl;
        if (!subcommands.empty()) {
            print("Available Commands:", Style(Color::Green, Color::None, true));
            for (const auto& cmd : subcommands) std::cout << "  " << pad_display(cmd->name, 15) << " " << cmd->description << std::endl;
            std::cout << std::endl;
        }
        if (!flags.empty()) {
            print("Flags:", Style(Color::Green, Color::None, true));
            for (const auto& f : flags) {
                std::string info = "  ";
                if (f.shorthand != 0) { info += "-"; info += f.shorthand; info += ", "; }
                info += "--"; info += f.name;
                std::cout << pad_display(info, 25) << " " << f.description << std::endl;
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
