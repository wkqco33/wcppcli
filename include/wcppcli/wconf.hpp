#pragma once

#include <string>
#include <map>
#include <variant>
#include <optional>

namespace wcppcli {

    class WConf {
        public:
            using ValueType = std::variant<std::string, int, bool>;

            void set(const std::string& key, ValueType value);
            void set_env_prefix(const std::string& prefix);
            void bind_env(const std::string& key, const std::string& env_name = "");

            // 통합 파일 읽기 (확장자 자동 감지)
            bool read_file(const std::string& path);

            // 파일이 없으면 현재 값으로 생성, 있으면 읽기
            bool ensure_file(const std::string& path);

            // 현재 설정을 파일로 저장
            bool write_file(const std::string& path);

            // 신규 지원 형식 (중첩 구조 지원 강화)
            bool read_json(const std::string& path);
            bool read_toml(const std::string& path);
            bool read_yaml(const std::string& path);

            std::string get_string(const std::string& key) const;
            int get_int(const std::string& key) const;
            bool get_bool(const std::string& key) const;

        private:
            std::map<std::string, ValueType> values_;
            std::map<std::string, std::string> env_bindings_;
            std::string env_prefix_;

            std::optional<std::string> get_env_value(const std::string& key) const;
    };

} // namespace wcppcli
