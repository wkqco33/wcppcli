#include "wcppcli/wlog.hpp"
#include "test_framework.hpp"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>

using namespace wcppcli;

namespace {
    // WLog 는 std::cout/std::cerr 에 직접 쓰므로, rdbuf 를 스왑해 캡처한다.
    // (isatty(fileno(stdout))는 C 레벨 stdout FILE*을 보므로 rdbuf 스왑엔 영향받지 않는다.)
    std::string capture_log_output(const std::function<void()>& fn) {
        std::ostringstream buf;
        auto* old_cout = std::cout.rdbuf(buf.rdbuf());
        auto* old_cerr = std::cerr.rdbuf(buf.rdbuf());
        fn();
        std::cout.rdbuf(old_cout);
        std::cerr.rdbuf(old_cerr);
        return buf.str();
    }
} // namespace

TEST_CASE("wlog WCPPCLI_LOG_LEVEL=warn suppresses debug/info but keeps warn/error") {
    wtest::set_env("WCPPCLI_LOG_LEVEL", "warn");
    std::string out = capture_log_output([] {
        WLog::debug("d-msg");
        WLog::info("i-msg");
        WLog::warn("w-msg");
        WLog::error("e-msg");
    });
    wtest::unset_env("WCPPCLI_LOG_LEVEL");

    CHECK(out.find("d-msg") == std::string::npos);
    CHECK(out.find("i-msg") == std::string::npos);
    CHECK(out.find("w-msg") != std::string::npos);
    CHECK(out.find("e-msg") != std::string::npos);
}

TEST_CASE("wlog re-reads WCPPCLI_LOG_LEVEL on every call (no first-call caching)") {
    // 이 테스트가 위 테스트 뒤에 실행되어도 새 레벨이 반영되어야 한다.
    // (should_log()가 과거처럼 최초 호출 시점의 static 값을 캐싱한다면 이 테스트는 실패한다.)
    wtest::set_env("WCPPCLI_LOG_LEVEL", "error");
    std::string out = capture_log_output([] {
        WLog::warn("w-msg2");
        WLog::error("e-msg2");
    });
    wtest::unset_env("WCPPCLI_LOG_LEVEL");

    CHECK(out.find("w-msg2") == std::string::npos);
    CHECK(out.find("e-msg2") != std::string::npos);
}

TEST_CASE("wlog defaults to info level when WCPPCLI_LOG_LEVEL is unset") {
    wtest::unset_env("WCPPCLI_LOG_LEVEL");
    std::string out = capture_log_output([] {
        WLog::debug("d-msg3");
        WLog::info("i-msg3");
    });

    CHECK(out.find("d-msg3") == std::string::npos);
    CHECK(out.find("i-msg3") != std::string::npos);
}
