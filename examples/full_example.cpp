#include "wcppcli/wcli.hpp"
#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>
#include <fstream>

using namespace wcppcli;

int main(int argc, char** argv) {
    WConf conf;
    conf.set("app_name", std::string("WCPP-CLI-APP"));
    conf.set("version", 1);

    // 1. 설정 파일 시뮬레이션
    {
        std::ofstream f("config.ini");
        f << "app_name=Renamed-App\n# This is a comment\nport=9000\n";
    }
    conf.read_file("config.ini");

    Command root;
    root.name = "wapp";
    root.description = "Full example of wcppcli library";

    auto info_cmd = std::make_unique<Command>();
    info_cmd->name = "info";
    info_cmd->description = "Show application information";
    info_cmd->handler = [&](const Command& cmd) {
        Panel p;
        p.title = "App Information";
        p.content = "Name: " + conf.get_string("app_name") + "\nVersion: v" + std::to_string(conf.get_int("version"));
        p.render();

        if (!cmd.args.empty()) {
            print("\nExtra Arguments provided:", Style(Color::Yellow));
            for (const auto& arg : cmd.args) std::cout << " - " << arg << std::endl;
        }
    };

    auto table_cmd = std::make_unique<Command>();
    table_cmd->name = "list";
    table_cmd->description = "Display a stylish table";
    table_cmd->handler = [](const Command&) {
        Table t;
        t.add_column("Key", Style(Color::Cyan, Color::None, true));
        t.add_column("Value", Style(Color::White));
        t.add_row({"OS", "Linux"});
        t.add_row({"Lang", "C++17"});
        t.add_row({"Library", "wcppcli"});
        t.render();
    };

    root.add_command(std::move(info_cmd));
    root.add_command(std::move(table_cmd));

    return root.execute(argc, argv);
}
