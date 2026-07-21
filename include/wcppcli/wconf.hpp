#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <optional>
#include <functional>

namespace wcppcli {

    class WConf {
        public:
            using ValueType = std::variant<std::string, int, bool, std::vector<std::string>>;
            using Validator = std::function<bool(const ValueType&)>;

            void set(const std::string& key, ValueType value);
            void set_env_prefix(const std::string& prefix);
            void bind_env(const std::string& key, const std::string& env_name = "");

            // 통합 파일 읽기 (확장자 자동 감지)
            bool read_file(const std::string& path);

            // 파일이 없으면 현재 값으로 생성, 있으면 읽기
            bool ensure_file(const std::string& path);

            // 현재 설정을 파일로 저장
            bool write_file(const std::string& path);

            // --- Schema Validation ---
            struct ValidationError {
                std::string key;
                std::string reason; // "missing required key" 또는 "validator rejected value"
            };

            void add_schema(const std::string& key, Validator validator = nullptr, bool required = false);
            bool validate() const; // validate_errors().empty() 와 동일
            std::vector<ValidationError> validate_errors() const;

            // 신규 지원 형식 (중첩 구조 지원 강화).
            // 인라인 객체/이스케이프 문자열은 지원하지 않는 경량 라인 기반 파서임에 유의.
            // 값은 따옴표 유무와 true/false/숫자 형태를 보고 string/bool/int로 추론되어 저장됨.
            // 배열은 `["a", "b"]` 형태의 한 줄짜리 인라인 문자열 배열만 지원하며(get_array),
            // 여러 줄에 걸친 YAML block-style 리스트(`- item`)는 지원하지 않음.
            bool read_json(const std::string& path);
            bool read_toml(const std::string& path);
            bool read_yaml(const std::string& path);

            // CLI 오버라이드 값 설정
            void set_cli(const std::string& key, ValueType value);

            std::string get_string(const std::string& key) const;
            int get_int(const std::string& key) const;
            bool get_bool(const std::string& key) const;
            std::vector<std::string> get_array(const std::string& key) const; // 값이 없거나 배열이 아니면 빈 벡터

        private:
            std::map<std::string, ValueType> values_;
            std::map<std::string, ValueType> cli_values_;
            std::map<std::string, std::string> env_bindings_;
            std::string env_prefix_;

            struct SchemaEntry {
                Validator validator;
                bool required;
            };
            std::map<std::string, SchemaEntry> schemas_;

            // 환경 변수 값은 항상 string 타입으로 반환됨 (파일 값과 달리 타입 추론을 하지 않음).
            // Validator에서 std::get<int>/std::get<bool>을 바로 쓰면 env로 채워진 값에 대해
            // bad_variant_access가 날 수 있으니 std::holds_alternative로 먼저 확인할 것.
            std::optional<std::string> get_env_value(const std::string& key) const;
            std::optional<ValueType> get_raw_value(const std::string& key) const;
    };

} // namespace wcppcli
