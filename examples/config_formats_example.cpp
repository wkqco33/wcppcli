#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>
#include <fstream>

using namespace wcppcli;

int main() {
    WConf conf;

    // 1. JSON 테스트 파일 생성
    {
        std::ofstream f("config.json");
        f << "{\n  \"app_name\": \"JSON-App\",\n  \"port\": 1234,\n  \"debug\": true\n}\n";
    }

    // 2. TOML 테스트 파일 생성
    {
        std::ofstream f("config.toml");
        f << "# This is a TOML file\n[server]\nhost = \"localhost\"\ntimeout = 30\n";
    }

    // 3. YAML 테스트 파일 생성
    {
        std::ofstream f("config.yaml");
        f << "# YAML format\nversion: 2.0\nauthor: \"seoyc\"\n";
    }

    std::cout << "--- Config Formats Test ---" << std::endl;

    if (conf.read_json("config.json")) {
        print("JSON Loaded:", Style(Color::Green, Color::None, true));
        std::cout << "  App Name: " << conf.get_string("app_name") << std::endl;
        std::cout << "  Port: " << conf.get_int("port") << std::endl;
    }

    if (conf.read_toml("config.toml")) {
        print("TOML Loaded:", Style(Color::Green, Color::None, true));
        // config.toml의 [server] 섹션 아래 키는 "server.host"처럼 점 표기법으로 저장된다.
        std::cout << "  Host: " << conf.get_string("server.host") << std::endl;
        std::cout << "  Timeout: " << conf.get_int("server.timeout") << std::endl;
    }

    if (conf.read_yaml("config.yaml")) {
        print("YAML Loaded:", Style(Color::Green, Color::None, true));
        std::cout << "  Version: " << conf.get_string("version") << std::endl;
        std::cout << "  Author: " << conf.get_string("author") << std::endl;
    }

    return 0;
}
