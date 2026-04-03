#include "wcppcli/wui.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>

using namespace wcppcli;

int main() {
    print("=== Interactive UI Component Test ===", Style(Color::Cyan, Color::None, true));
    std::cout << std::endl;

    // 1. input 테스트
    std::string name = ui::input("What is your name?", "Guest");
    print("Hello, " + name + "!", Style(Color::Green));
    std::cout << std::endl;

    // 2. confirm 테스트
    bool proceed = ui::confirm("Do you want to see the options?", true);
    if (!proceed) {
        print("Goodbye!", Style(Color::Yellow));
        return 0;
    }
    std::cout << std::endl;

    // 3. select 테스트
    std::vector<std::string> themes = {"Dark Mode", "Light Mode", "System Default", "High Contrast"};
    int choice = ui::select("Choose your preferred theme:", themes);

    if (choice != -1) {
        print("You selected: " + themes[choice], Style(Color::Green, Color::None, true));
    } else {
        print("No selection made.", Style(Color::Red));
    }

    std::cout << std::endl;
    print("Test completed.", Style(Color::Cyan));

    return 0;
}
