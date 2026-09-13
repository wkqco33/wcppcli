#pragma once

#include <string>
#include <string_view>
#include "wcppcli/wstyle.hpp"

namespace wcppcli {

    enum class LogLevel {
        Debug,
        Info,
        Success,
        Warn,
        Error
    };

    class WLog {
        public:
            static void log(LogLevel level, std::string_view message);

            // 프로그램에서 최소 로그 레벨을 지정한다. 설정하면 WCPPCLI_LOG_LEVEL 보다
            // 우선하며, reset_min_level() 로 환경변수 기반 자동 판단으로 되돌린다.
            static void set_min_level(LogLevel level);
            static void reset_min_level();
            
            static void debug(std::string_view msg) { log(LogLevel::Debug, msg); }
            static void info(std::string_view msg) { log(LogLevel::Info, msg); }
            static void success(std::string_view msg) { log(LogLevel::Success, msg); }
            static void warn(std::string_view msg) { log(LogLevel::Warn, msg); }
            static void error(std::string_view msg) { log(LogLevel::Error, msg); }

        private:
            static Style get_level_style(LogLevel level);
            static std::string_view get_level_label(LogLevel level);
    };

} // namespace wcppcli
