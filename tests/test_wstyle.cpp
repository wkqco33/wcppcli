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

TEST_CASE("color_enabled honors an explicit override until reset") {
    set_color_enabled(false);
    CHECK(!color_enabled());
    set_color_enabled(true);
    CHECK(color_enabled());
    reset_color_enabled();
}

TEST_CASE("set_color_enabled(true) emits ANSI codes without a tty") {
    set_color_enabled(true);
    std::string out = format("hello", Style(Color::Red, Color::None, true));
    reset_color_enabled();

    CHECK(out.find("\033[") != std::string::npos);
}

TEST_CASE("format without a stream never colors a non-console target") {
    // 스트림을 넘기면 그 스트림 기준으로 tty 여부를 판단해야 한다(로그를 stderr 로
    // 분리했을 때 stdout 이 tty 라도 리다이렉트된 stderr 에 ANSI 가 새지 않도록).
    std::ostringstream oss;
    std::string out = format("hello", Style(Color::Red, Color::None, true), oss);
    CHECK_EQ(out, std::string("hello"));
}

TEST_CASE("reset_color_enabled restores automatic detection (NO_COLOR applies again)") {
    set_color_enabled(true);
    wtest::set_env("NO_COLOR", "1");
    // 명시적 강제가 NO_COLOR 보다 우선한다.
    CHECK(color_enabled());

    reset_color_enabled();
    // auto 로 돌아오면 NO_COLOR 가 적용된다(tty 여부와 무관하게 결정적).
    CHECK(!color_enabled());
    wtest::unset_env("NO_COLOR");
}
