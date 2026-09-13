#pragma once

#include <string>
#include <vector>
#include "wcppcli/wstyle.hpp"

namespace wcppcli {

    namespace ui {

        // 대화형 프롬프트 활성화 여부. 기본은 stdin 이 tty 인지로 자동 판단하며,
        // set_interactive_enabled() 로 명시 지정할 수 있다(지정 시 자동 판단보다 우선).
        // 비활성 상태에서는 프롬프트를 출력하지 않고 stdin 도 읽지 않으며 기본값을 반환한다.
        bool interactive_enabled();
        void set_interactive_enabled(bool enabled);
        void reset_interactive_enabled();

        // [y/N] 형태의 확인 프롬프트
        bool confirm(const std::string& label, bool default_val = true);

        // 텍스트 입력 프롬프트
        std::string input(const std::string& label, const std::string& default_val = "");

        // 목록 선택 프롬프트 (번호 입력 방식)
        int select(const std::string& label, const std::vector<std::string>& options);

    } // namespace ui

} // namespace wcppcli
