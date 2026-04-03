#pragma once

#include <string>
#include <vector>
#include "wcppcli/wstyle.hpp"

namespace wcppcli {

    namespace ui {

        // [y/N] 형태의 확인 프롬프트
        bool confirm(const std::string& label, bool default_val = true);

        // 텍스트 입력 프롬프트
        std::string input(const std::string& label, const std::string& default_val = "");

        // 목록 선택 프롬프트 (번호 입력 방식)
        int select(const std::string& label, const std::vector<std::string>& options);

    } // namespace ui

} // namespace wcppcli
