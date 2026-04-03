#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>
#include <fstream>

using namespace wcppcli;

int main() {
    WConf conf;

    // 1. 중첩 JSON 생성
    {
        std::ofstream f("nested.json");
        f << "{\n  \"server\": {\n    \"host\": \"127.0.0.1\",\n    \"port\": 8080\n  }\n}\n";
    }

    // 2. 중첩 TOML 생성
    {
        std::ofstream f("nested.toml");
        f << "[database]\nuser = \"admin\"\npass = \"1234\"\n";
    }

    // 3. 중첩 YAML 생성
    {
        std::ofstream f("nested.yaml");
        f << "auth:\n  jwt:\n    secret: \"top-secret\"\n    expiry: 3600\n";
    }

    std::cout << "--- Nested Config & Auto Detection Test ---" << std::endl;

    // read_file 하나로 모든 포맷 처리
    conf.read_file("nested.json");
    conf.read_file("nested.toml");
    conf.read_file("nested.yaml");

    print("Server Config (JSON Nested):", Style(Color::Cyan, Color::None, true));
    std::cout << "  Host: " << conf.get_string("server.host") << std::endl;
    std::cout << "  Port: " << conf.get_int("server.port") << std::endl;

    print("\nDatabase Config (TOML Section):", Style(Color::Magenta, Color::None, true));
    std::cout << "  User: " << conf.get_string("database.user") << std::endl;

    print("\nAuth Config (YAML Indent):", Style(Color::Yellow, Color::None, true));
    std::cout << "  JWT Secret: " << conf.get_string("auth.jwt.secret") << std::endl;
    std::cout << "  JWT Expiry: " << conf.get_int("auth.jwt.expiry") << std::endl;

    return 0;
}
