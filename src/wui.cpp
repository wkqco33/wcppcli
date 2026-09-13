#include "wcppcli/wui.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <limits>
#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

namespace wcppcli {

    namespace {

        enum class InteractiveOverride { Auto, On, Off };

        InteractiveOverride& interactive_override() {
            static InteractiveOverride mode = InteractiveOverride::Auto;
            return mode;
        }

        bool stdin_is_tty() {
            #ifdef _WIN32
                return _isatty(_fileno(stdin)) != 0;
            #else
                return isatty(fileno(stdin)) != 0;
            #endif
        }

        // 프롬프트는 stderr 로 보낸다. stdout 은 명령의 결과 전용으로 남겨 파이프/CI에서
        // 대화형 흐름과 무관하게 결과를 그대로 쓸 수 있게 한다.
        void prompt_out(const std::string& text, const Style& style) {
            std::cerr << format(text, style, std::cerr);
        }

    } // namespace

    namespace ui {

        bool interactive_enabled() {
            switch (interactive_override()) {
                case InteractiveOverride::On:  return true;
                case InteractiveOverride::Off: return false;
                case InteractiveOverride::Auto: break;
            }
            return stdin_is_tty();
        }

        void set_interactive_enabled(bool enabled) {
            interactive_override() = enabled ? InteractiveOverride::On : InteractiveOverride::Off;
        }

        void reset_interactive_enabled() { interactive_override() = InteractiveOverride::Auto; }

        bool confirm(const std::string& label, bool default_val) {
            if (!interactive_enabled()) return default_val;

            const std::string prompt = default_val ? "[Y/n]" : "[y/N]";
            prompt_out(label + " ", Style(Color::White, Color::None, true));
            prompt_out(prompt, Style(Color::Cyan));
            std::cerr << ": " << std::flush;

            std::string line;
            if (!std::getline(std::cin, line) || line.empty()) {
                return default_val;
            }

            std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (line == "y" || line == "yes") return true;
            if (line == "n" || line == "no") return false;
            
            return default_val;
        }

        std::string input(const std::string& label, const std::string& default_val) {
            if (!interactive_enabled()) return default_val;

            prompt_out(label, Style(Color::White, Color::None, true));
            if (!default_val.empty()) {
                std::cerr << " ";
                prompt_out("(" + default_val + ")", Style(Color::Cyan));
            }
            std::cerr << ": " << std::flush;

            std::string line;
            if (!std::getline(std::cin, line) || line.empty()) {
                return default_val;
            }
            return line;
        }

        int select(const std::string& label, const std::vector<std::string>& options) {
            if (options.empty()) return -1;
            if (!interactive_enabled()) return -1;

            prompt_out(label, Style(Color::White, Color::None, true));
            std::cerr << std::endl;
            for (size_t i = 0; i < options.size(); ++i) {
                std::cerr << "  ";
                prompt_out(std::to_string(i + 1) + ")", Style(Color::Cyan));
                std::cerr << " " << options[i] << std::endl;
            }

            while (true) {
                prompt_out("Select [1-" + std::to_string(options.size()) + "]", Style(Color::Cyan));
                std::cerr << ": " << std::flush;

                std::string line;
                if (!std::getline(std::cin, line)) return -1;
                
                try {
                    int choice = std::stoi(line);
                    if (choice >= 1 && choice <= static_cast<int>(options.size())) {
                        return choice - 1; // 0-based index 반환
                    }
                } catch (...) {
                    // Invalid input, retry
                }
                
                std::cerr << format("Invalid choice. Please try again.", Style(Color::Red), std::cerr) << std::endl;
            }
        }

    } // namespace ui

} // namespace wcppcli
