#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>
#include <cstdio> // remove

using namespace wcppcli;

int main() {
    std::string path = "generated_config.toml";
    std::remove(path.c_str()); // 테스트를 위해 기존 파일 삭제

    WConf conf;
    // 1. 기본값 설정
    conf.set("server.host", std::string("127.0.0.1"));
    conf.set("server.port", 8080);
    conf.set("debug", true);

    std::cout << "--- Ensure File (Auto-Create) Test ---" << std::endl;

    // 2. ensure_file 호출 (파일이 없으므로 생성됨)
    if (conf.ensure_file(path)) {
        print("File 'generated_config.toml' was created with defaults!", {.fg = Color::Green});
    }

    // 3. 다시 읽어보기 (이미 존재하므로 로드됨)
    WConf conf2;
    conf2.read_file(path);
    std::cout << "Loaded Host: " << conf2.get_string("server.host") << std::endl;
    std::cout << "Loaded Port: " << conf2.get_int("server.port") << std::endl;

    return 0;
}
