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
