#include "wcppcli/wcli.hpp"
#include <iostream>

using namespace wcppcli;

int main(int argc, char** argv) {
    Command root;
    root.name = "myapp";
    root.description = "Example app with auto-completion support";

    // 1. 자동 완성 스크립트 출력을 위한 플래그 추가
    bool show_completion = false;
    Flag comp_flag;
    comp_flag.name = "completion";
    comp_flag.description = "Generate bash completion script";
    comp_flag.value_ptr = &show_completion;
    root.add_flag(comp_flag);

    // 2. 더미 커맨드 및 플래그 추가
    auto start_cmd = std::make_unique<Command>();
    start_cmd->name = "start";
    start_cmd->description = "Start the service";
    root.add_command(std::move(start_cmd));

    auto stop_cmd = std::make_unique<Command>();
    stop_cmd->name = "stop";
    stop_cmd->description = "Stop the service";
    root.add_command(std::move(stop_cmd));

    // 3. 실행 및 결과 처리
    int result = root.execute(argc, argv);
    if (result == 0 && show_completion) {
        std::cout << root.generate_bash_completion() << std::endl;
    } else {
        std::cout << "Try: myapp --completion" << std::endl;
    }

    return 0;
}
