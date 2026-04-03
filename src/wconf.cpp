#include "wcppcli/wconf.hpp"
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace wcppcli {

    void WConf::set(const std::string& key, ValueType value) { values_[key] = std::move(value); }
    void WConf::set_cli(const std::string& key, ValueType value) { cli_values_[key] = std::move(value); }
    void WConf::set_env_prefix(const std::string& prefix) { env_prefix_ = prefix; }
    void WConf::bind_env(const std::string& key, const std::string& env_name) { env_bindings_[key] = env_name; }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n\"");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\r\n\",");
        return s.substr(start, end - start + 1);
    }

    bool WConf::read_file(const std::string& path) {
        size_t dot = path.find_last_of('.');
        if (dot == std::string::npos) return read_json(path);
        std::string ext = path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == "json") return read_json(path);
        if (ext == "toml" || ext == "ini") return read_toml(path);
        if (ext == "yaml" || ext == "yml") return read_yaml(path);
        return read_json(path);
    }

    bool WConf::ensure_file(const std::string& path) {
        std::ifstream f(path);
        if (f.good()) return read_file(path);
        return write_file(path);
    }

    bool WConf::write_file(const std::string& path) {
        std::ofstream f(path);
        if (!f.is_open()) return false;

        size_t dot = path.find_last_of('.');
        std::string ext = (dot == std::string::npos) ? "json" : path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == "json") {
            f << "{\n";
            for (auto it = values_.begin(); it != values_.end(); ++it) {
                f << "  \"" << it->first << "\": ";
                if (std::holds_alternative<std::string>(it->second)) f << "\"" << std::get<std::string>(it->second) << "\"";
                else if (std::holds_alternative<int>(it->second)) f << std::get<int>(it->second);
                else if (std::holds_alternative<bool>(it->second)) f << (std::get<bool>(it->second) ? "true" : "false");
                if (std::next(it) != values_.end()) f << ",";
                f << "\n";
            }
            f << "}\n";
        } else if (ext == "toml" || ext == "ini") {
            for (const auto& [k, v] : values_) {
                f << k << " = ";
                if (std::holds_alternative<std::string>(v)) f << "\"" << std::get<std::string>(v) << "\"\n";
                else if (std::holds_alternative<int>(v)) f << std::get<int>(v) << "\n";
                else if (std::holds_alternative<bool>(v)) f << (std::get<bool>(v) ? "true" : "false") << "\n";
            }
        } else if (ext == "yaml" || ext == "yml") {
            for (const auto& [k, v] : values_) {
                f << k << ": ";
                if (std::holds_alternative<std::string>(v)) f << "\"" << std::get<std::string>(v) << "\"\n";
                else if (std::holds_alternative<int>(v)) f << std::get<int>(v) << "\n";
                else if (std::holds_alternative<bool>(v)) f << (std::get<bool>(v) ? "true" : "false") << "\n";
            }
        }
        return true;
    }


    bool WConf::read_json(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        std::vector<std::string> stack;
        std::string line;
        while (std::getline(file, line)) {
            size_t open_brace = line.find('{');
            size_t close_brace = line.find('}');
            size_t colon = line.find(':');

            if (open_brace != std::string::npos && colon != std::string::npos) {
                size_t s = line.find('\"');
                size_t e = line.find('\"', s + 1);
                if (s != std::string::npos && e < colon) stack.push_back(line.substr(s + 1, e - s - 1));
            } else if (close_brace != std::string::npos && !stack.empty()) {
                stack.pop_back();
            } else if (colon != std::string::npos) {
                size_t s = line.find('\"');
                size_t e = line.find('\"', s + 1);
                if (s != std::string::npos && e < colon) {
                    std::string k = line.substr(s + 1, e - s - 1);
                    std::string full_key;
                    for (const auto& p : stack) full_key += p + ".";
                    full_key += k;
                    values_[full_key] = trim(line.substr(colon + 1));
                }
            }
        }
        return true;
    }

    bool WConf::read_toml(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        std::string section, line;
        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t"));
            if (line.empty() || line[0] == '#') continue;
            if (line[0] == '[' && line.find(']') != std::string::npos) {
                section = line.substr(1, line.find(']') - 1);
            } else {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string k = trim(line.substr(0, pos));
                    std::string full_key = section.empty() ? k : section + "." + k;
                    values_[full_key] = trim(line.substr(pos + 1));
                }
            }
        }
        return true;
    }

    bool WConf::read_yaml(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        std::vector<std::pair<int, std::string>> stack;
        std::string line;
        while (std::getline(file, line)) {
            int indent = line.find_first_not_of(" \t");
            if (indent == (int)std::string::npos || line[indent] == '#') continue;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string k = trim(line.substr(indent, colon - indent));
                std::string v = trim(line.substr(colon + 1));
                while (!stack.empty() && stack.back().first >= indent) stack.pop_back();
                std::string full_key;
                for (const auto& p : stack) full_key += p.second + ".";
                full_key += k;
                if (v.empty()) stack.push_back({indent, k});
                else values_[full_key] = v;
            }
        }
        return true;
    }

    std::optional<std::string> WConf::get_env_value(const std::string& key) const {
        std::string env_name;
        if (env_bindings_.count(key)) env_name = env_bindings_.at(key);
        else {
            env_name = env_prefix_.empty() ? key : env_prefix_ + "_" + key;
            std::replace(env_name.begin(), env_name.end(), '.', '_');
            std::transform(env_name.begin(), env_name.end(), env_name.begin(), ::toupper);
        }
        const char* val = std::getenv(env_name.c_str());
        return val ? std::optional<std::string>(val) : std::nullopt;
    }

    std::optional<WConf::ValueType> WConf::get_raw_value(const std::string& key) const {
        if (cli_values_.count(key)) return cli_values_.at(key);
        if (auto env = get_env_value(key)) return *env;
        if (values_.count(key)) return values_.at(key);
        return std::nullopt;
    }

    std::string WConf::get_string(const std::string& key) const {
        auto val = get_raw_value(key);
        if (!val) return "";
        if (std::holds_alternative<std::string>(*val)) return std::get<std::string>(*val);
        return ""; // 타입 불일치 시 빈 문자열
    }

    int WConf::get_int(const std::string& key) const {
        auto val = get_raw_value(key);
        if (!val) return 0;
        if (std::holds_alternative<int>(*val)) return std::get<int>(*val);
        if (std::holds_alternative<std::string>(*val)) {
            try { return std::stoi(std::get<std::string>(*val)); } catch (...) { return 0; }
        }
        return 0;
    }

    bool WConf::get_bool(const std::string& key) const {
        auto val = get_raw_value(key);
        if (!val) return false;
        if (std::holds_alternative<bool>(*val)) return std::get<bool>(*val);
        
        std::string s;
        if (std::holds_alternative<std::string>(*val)) s = std::get<std::string>(*val);
        else return false;

        if (s.empty()) return false;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s == "true" || s == "1" || s == "yes";
    }

    void WConf::add_schema(const std::string& key, Validator validator, bool required) {
        schemas_[key] = {validator, required};
    }

    bool WConf::validate() const {
        for (const auto& [key, entry] : schemas_) {
            auto val = get_raw_value(key);
            if (!val) {
                if (entry.required) return false;
                continue;
            }
            if (entry.validator && !entry.validator(*val)) return false;
        }
        return true;
    }

} // namespace wcppcli
