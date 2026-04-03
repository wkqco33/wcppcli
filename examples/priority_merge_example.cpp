#include "wcppcli/wcli.hpp"
#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace wcppcli;

int main(int argc, char** argv) {
    WConf conf;
    
    // 1. 기본값 설정 (우선순위 4: 가장 낮음)
    conf.set("port", 8080);
    conf.set("debug", false);

    // 2. 설정 파일 생성 및 로드 (우선순위 3)
    {
        std::ofstream f("config.ini");
        f << "port = 9090\n";
    }
    conf.read_file("config.ini");

    // 3. 환경 변수 바인딩 (우선순위 2)
    // 실제 환경 변수 설정 (WCONF_APP_PORT)
    setenv("APP_PORT", "1010", 1);
    conf.set_env_prefix("APP");
    conf.bind_env("port", "APP_PORT");

    // 4. CLI 플래그 정의 및 바인딩 (우선순위 1: 가장 높음)
    Command root;
    root.name = "priority_test";
    root.bind_config(&conf);

    int cli_port = 0;
    Flag port_flag;
    port_flag.name = "port";
    port_flag.shorthand = 'p';
    port_flag.description = "Server port";
    port_flag.value_ptr = &cli_port;
    port_flag.config_key = "port"; // WConf의 "port" 키와 연동
    root.add_flag(port_flag);

    root.handler = [&](const Command&) {
        print("--- Configuration Priority Test ---", Style(Color::Cyan, Color::None, true));
        
        std::cout << "Final Port Value: " << conf.get_int("port") << std::endl;
        
        if (conf.get_int("port") == 1212) {
            print("SUCCESS: CLI Flag took priority!", Style(Color::Green));
        } else if (conf.get_int("port") == 1010) {
            print("SUCCESS: Env Var took priority (when no flag)!", Style(Color::Green));
        } else if (conf.get_int("port") == 9090) {
            print("SUCCESS: Config File took priority (when no env/flag)!", Style(Color::Green));
        } else {
            print("Using default value: 8080", Style(Color::Yellow));
        }
    };

    return root.execute(argc, argv);
}
