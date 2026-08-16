#include "wcppcli/wstyle.hpp"
#include "test_framework.hpp"
#include <cstdlib>

using namespace wcppcli;

TEST_CASE("display_width counts ascii as 1 column per character") {
    CHECK_EQ(display_width("abc"), static_cast<size_t>(3));
}

TEST_CASE("display_width counts hangul as 2 columns per character") {
    CHECK_EQ(display_width("\xea\xb0\x80\xeb\x82\x98"), static_cast<size_t>(4)); // "가나" (UTF-8), 2글자 x 2칸
}

TEST_CASE("pad_display pads based on display width, not byte length") {
    std::string padded = pad_display("\xea\xb0\x80", 4); // "가" (표시폭 2) + 공백 2칸 = 표시폭 4
    CHECK_EQ(display_width(padded), static_cast<size_t>(4));
}

TEST_CASE("format emits plain text (no ANSI codes) when NO_COLOR is set") {
    wtest::set_env("NO_COLOR", "1");
    std::string out = format("hello", Style(Color::Red, Color::None, true));
    CHECK_EQ(out, std::string("hello"));
    wtest::unset_env("NO_COLOR");
}
