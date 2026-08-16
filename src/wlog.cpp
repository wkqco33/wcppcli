#include "wcppcli/wlog.hpp"
#include "environment.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

namespace wcppcli {

    namespace {

        int level_priority(LogLevel level) {
            switch (level) {
                case LogLevel::Debug:   return 10;
                case LogLevel::Info:    return 20;
                case LogLevel::Success: return 20;
                case LogLevel::Warn:    return 30;
                case LogLevel::Error:   return 40;
                default:                return 20;
            }
        }

        LogLevel parse_min_level_from_env() {
            auto raw = detail::read_environment_variable("WCPPCLI_LOG_LEVEL");
            if (!raw) {
                return LogLevel::Info;
            }

            std::string s = *raw;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });

            if (s == "debug") {
                return LogLevel::Debug;
            }
            if (s == "info") {
                return LogLevel::Info;
            }
            if (s == "success") {
                return LogLevel::Success;
            }
            if (s == "warn" || s == "warning") {
                return LogLevel::Warn;
            }
            if (s == "error") {
                return LogLevel::Error;
            }

            return LogLevel::Info;
        }

        // 호출마다 환경변수를 재평가한다 (캐싱하지 않음). 로그 호출은 핫패스가 아니므로
        // 비용이 무시할 만한 수준이며, 테스트/서브프로세스에서 WCPPCLI_LOG_LEVEL을
        // 바꿔가며 검증할 수 있어야 하기 때문.
        bool should_log(LogLevel level) {
            return level_priority(level) >= level_priority(parse_min_level_from_env());
        }

    } // namespace

    void WLog::log(LogLevel level, std::string_view message) {
        if (!should_log(level)) {
            return;
        }

        std::ostream& out =
            (level == LogLevel::Info || level == LogLevel::Success) ? std::cout : std::cerr;
        
        Style level_style = get_level_style(level);
        std::string label = "[" + std::string(get_level_label(level)) + "]";
        
        out << format(label, level_style) << " " << message << std::endl;
    }

    Style WLog::get_level_style(LogLevel level) {
        switch (level) {
            case LogLevel::Debug:   return Style(Color::Default, Color::None, false);
            case LogLevel::Info:    return Style(Color::Cyan, Color::None, true);
            case LogLevel::Success: return Style(Color::Green, Color::None, true);
            case LogLevel::Warn:    return Style(Color::Yellow, Color::None, true);
            case LogLevel::Error:   return Style(Color::Red, Color::None, true);
            default:                return Style();
        }
    }

    std::string_view WLog::get_level_label(LogLevel level) {
        switch (level) {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO ";
            case LogLevel::Success: return "SUCCESS";
            case LogLevel::Warn:    return "WARN ";
            case LogLevel::Error:   return "ERROR";
            default:                return "LOG  ";
        }
    }

} // namespace wcppcli
