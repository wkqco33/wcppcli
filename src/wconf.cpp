#include "wcppcli/wconf.hpp"
#include "environment.hpp"
#include "wcppcli/wlog.hpp"
#include <cctype>
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

    // 따옴표로 감싸인 값은 문자열로, "true"/"false"는 bool로, 숫자는 int로 추론.
    // 파일에서 읽은 값도 CLI/코드로 직접 set()한 값과 동일한 타입을 갖도록 하기 위함.
    static WConf::ValueType parse_scalar(const std::string& raw) {
        auto start = raw.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return std::string("");
        auto end = raw.find_last_not_of(" \t\r\n,");
        std::string v = raw.substr(start, end - start + 1);

        if (v.size() >= 2 && v.front() == '"' && v.back() == '"') return v.substr(1, v.size() - 2);
        if (v == "true") return true;
        if (v == "false") return false;
        if (!v.empty()) {
            size_t i = (v[0] == '-' || v[0] == '+') ? 1 : 0;
            bool is_num = i < v.size();
            for (size_t j = i; j < v.size(); ++j) if (!std::isdigit((unsigned char)v[j])) { is_num = false; break; }
            if (is_num) {
                try { return std::stoi(v); } catch (...) {}
            }
        }
        return v;
    }

    // 값이 `[`로 시작하는 한 줄짜리 인라인 배열이면 문자열 벡터로 파싱하고, 아니면 parse_scalar로 위임.
    // 여러 줄에 걸친 배열은 지원하지 않는다(닫는 ']'가 같은 줄에 없으면 스칼라로 폴백).
    static WConf::ValueType parse_array_or_scalar(const std::string& raw) {
        auto start = raw.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return std::string("");
        if (raw[start] != '[') return parse_scalar(raw);

        size_t close = raw.find(']', start);
        if (close == std::string::npos) return parse_scalar(raw);

        std::string inner = raw.substr(start + 1, close - start - 1);
        std::vector<std::string> items;
        std::stringstream ss(inner);
        std::string item;
        while (std::getline(ss, item, ',')) {
            std::string trimmed = trim(item);
            if (!trimmed.empty()) items.push_back(trimmed);
        }
        return items;
    }

    static void write_array(std::ostream& f, const std::vector<std::string>& arr) {
        f << "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            f << "\"" << arr[i] << "\"";
            if (i + 1 < arr.size()) f << ", ";
        }
        f << "]";
    }

    bool WConf::read_file(const std::string& path) {
        size_t dot = path.find_last_of('.');
        if (dot == std::string::npos) return read_json(path);
        std::string ext = path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == "json") return read_json(path);
        if (ext == "toml" || ext == "ini" || ext == "env") return read_toml(path);
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
                else if (std::holds_alternative<std::vector<std::string>>(it->second)) write_array(f, std::get<std::vector<std::string>>(it->second));
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
                else if (std::holds_alternative<std::vector<std::string>>(v)) { write_array(f, std::get<std::vector<std::string>>(v)); f << "\n"; }
            }
        } else if (ext == "yaml" || ext == "yml") {
            for (const auto& [k, v] : values_) {
                f << k << ": ";
                if (std::holds_alternative<std::string>(v)) f << "\"" << std::get<std::string>(v) << "\"\n";
                else if (std::holds_alternative<int>(v)) f << std::get<int>(v) << "\n";
                else if (std::holds_alternative<bool>(v)) f << (std::get<bool>(v) ? "true" : "false") << "\n";
                else if (std::holds_alternative<std::vector<std::string>>(v)) { write_array(f, std::get<std::vector<std::string>>(v)); f << "\n"; }
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
                    values_[full_key] = parse_array_or_scalar(line.substr(colon + 1));
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
                    values_[full_key] = parse_array_or_scalar(line.substr(pos + 1));
                }
            }
        }
        return true;
    }

    bool WConf::read_yaml(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        std::vector<std::pair<size_t, std::string>> stack;
        std::string line;
        while (std::getline(file, line)) {
            size_t indent = line.find_first_not_of(" \t");
            if (indent == std::string::npos || line[indent] == '#') continue;
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string k = trim(line.substr(indent, colon - indent));
                std::string raw_v = line.substr(colon + 1);
                std::string v = trim(raw_v); // 빈 값이면 중첩 매핑 헤더로 간주
                while (!stack.empty() && stack.back().first >= indent) stack.pop_back();
                std::string full_key;
                for (const auto& p : stack) full_key += p.second + ".";
                full_key += k;
                if (v.empty()) stack.push_back({indent, k});
                else values_[full_key] = parse_array_or_scalar(raw_v);
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
        return detail::read_environment_variable(env_name.c_str());
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
            try { return std::stoi(std::get<std::string>(*val)); }
            catch (...) {
                WLog::warn("key '" + key + "' is not a valid integer, using 0");
                return 0;
            }
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

    std::vector<std::string> WConf::get_array(const std::string& key) const {
        auto val = get_raw_value(key);
        if (!val) return {};
        if (std::holds_alternative<std::vector<std::string>>(*val)) return std::get<std::vector<std::string>>(*val);
        return {}; // 타입 불일치 시 빈 벡터
    }

    void WConf::add_schema(const std::string& key, Validator validator, bool required) {
        schemas_[key] = {validator, required};
    }

    std::vector<WConf::ValidationError> WConf::validate_errors() const {
        std::vector<ValidationError> errors;
        for (const auto& [key, entry] : schemas_) {
            auto val = get_raw_value(key);
            if (!val) {
                if (entry.required) errors.push_back({key, "missing required key"});
                continue;
            }
            if (entry.validator && !entry.validator(*val)) {
                errors.push_back({key, "validator rejected value"});
            }
        }
        return errors;
    }

    bool WConf::validate() const {
        return validate_errors().empty();
    }

} // namespace wcppcli
