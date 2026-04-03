#include "wcppcli/wlog.hpp"
#include <iostream>

namespace wcppcli {

    void WLog::log(LogLevel level, std::string_view message) {
        std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
        
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
