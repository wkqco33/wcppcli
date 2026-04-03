#include "wcppcli/wui.hpp"
#include <iostream>
#include <algorithm>
#include <limits>

namespace wcppcli {

    namespace ui {

        bool confirm(const std::string& label, bool default_val) {
            std::string prompt = default_val ? "[Y/n]" : "[y/N]";
            std::cout << format(label + " ", Style(Color::White, Color::None, true)) 
                      << format(prompt, Style(Color::Cyan)) << ": ";
            
            std::string line;
            if (!std::getline(std::cin, line) || line.empty()) {
                return default_val;
            }

            std::transform(line.begin(), line.end(), line.begin(), ::tolower);
            if (line == "y" || line == "yes") return true;
            if (line == "n" || line == "no") return false;
            
            return default_val;
        }

        std::string input(const std::string& label, const std::string& default_val) {
            std::cout << format(label, Style(Color::White, Color::None, true));
            if (!default_val.empty()) {
                std::cout << " " << format("(" + default_val + ")", Style(Color::Cyan));
            }
            std::cout << ": ";

            std::string line;
            if (!std::getline(std::cin, line) || line.empty()) {
                return default_val;
            }
            return line;
        }

        int select(const std::string& label, const std::vector<std::string>& options) {
            if (options.empty()) return -1;

            std::cout << format(label, Style(Color::White, Color::None, true)) << std::endl;
            for (size_t i = 0; i < options.size(); ++i) {
                std::cout << "  " << format(std::to_string(i + 1) + ")", Style(Color::Cyan))
                          << " " << options[i] << std::endl;
            }

            while (true) {
                std::cout << format("Select [1-" + std::to_string(options.size()) + "]", Style(Color::Cyan)) << ": ";
                
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
                
                print("Invalid choice. Please try again.", Style(Color::Red));
            }
        }

    } // namespace ui

} // namespace wcppcli
