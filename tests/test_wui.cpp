#include "wcppcli/wui.hpp"
#include "test_framework.hpp"
#include <iostream>
#include <sstream>
#include <vector>

using namespace wcppcli;

// ui 프롬프트는 stdout/stderr/stdin 을 직접 사용하므로 rdbuf 를 스왑해 캡처한다.
namespace {

    struct StreamSwap {
        std::ostringstream out_buf;
        std::ostringstream err_buf;
        std::istringstream in_buf;
        std::streambuf* old_out;
        std::streambuf* old_err;
        std::streambuf* old_in;

        explicit StreamSwap(const std::string& input = "") : in_buf(input) {
            old_out = std::cout.rdbuf(out_buf.rdbuf());
            old_err = std::cerr.rdbuf(err_buf.rdbuf());
            old_in = std::cin.rdbuf(in_buf.rdbuf());
        }

        ~StreamSwap() {
            std::cout.rdbuf(old_out);
            std::cerr.rdbuf(old_err);
            std::cin.rdbuf(old_in);
        }
    };

} // namespace

TEST_CASE("ui::confirm returns the default without consuming stdin when non-interactive") {
    ui::set_interactive_enabled(false);
    StreamSwap swap("yes\n");

    CHECK(ui::confirm("overwrite?", false) == false);
    CHECK(ui::confirm("proceed?", true) == true);

    // 비대화형에서는 stdin 을 읽지 않아야 한다(파이프 입력을 소비하지 않음).
    std::string remaining;
    std::getline(std::cin, remaining);
    CHECK_EQ(remaining, std::string("yes"));

    ui::reset_interactive_enabled();
}

TEST_CASE("ui::input returns the default when non-interactive") {
    ui::set_interactive_enabled(false);
    StreamSwap swap("typed\n");

    CHECK_EQ(ui::input("name", "fallback"), std::string("fallback"));

    ui::reset_interactive_enabled();
}

TEST_CASE("ui::select returns -1 when non-interactive") {
    ui::set_interactive_enabled(false);
    StreamSwap swap("1\n");

    CHECK_EQ(ui::select("pick", {"a", "b"}), -1);

    ui::reset_interactive_enabled();
}

TEST_CASE("ui prompts and errors go to stderr so stdout stays machine-readable") {
    ui::set_interactive_enabled(true);
    StreamSwap swap("\n");

    const bool answer = ui::confirm("overwrite?", false);
    const std::string picked = ui::input("name", "fallback");
    const int index = ui::select("pick", {"a", "b"});

    CHECK(answer == false);
    CHECK_EQ(picked, std::string("fallback"));
    // select 는 빈 줄을 받으면 -1 을 반환한다.
    CHECK_EQ(index, -1);

    CHECK(swap.out_buf.str().empty());
    CHECK(swap.err_buf.str().find("overwrite?") != std::string::npos);
    CHECK(swap.err_buf.str().find("pick") != std::string::npos);

    ui::reset_interactive_enabled();
}
