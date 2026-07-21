#pragma once
// wcppcli 의 "의존성 제로" 원칙을 테스트 도구에도 유지하기 위한 초경량 테스트 하네스.
// 외부 프레임워크(Catch2/doctest 등) 없이 TEST_CASE/CHECK/CHECK_EQ 만 제공한다.

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace wtest {

    struct TestCase {
        std::string name;
        std::function<void()> fn;
    };

    inline std::vector<TestCase>& registry() {
        static std::vector<TestCase> tests;
        return tests;
    }

    inline int& failure_count() {
        static int count = 0;
        return count;
    }

    struct Registrar {
        Registrar(std::string name, std::function<void()> fn) {
            registry().push_back({std::move(name), std::move(fn)});
        }
    };

    inline void check_failed(const std::string& expr, const char* file, int line) {
        std::cerr << "  CHECK failed: " << expr << " (" << file << ":" << line << ")\n";
        ++failure_count();
    }

    inline int run_all() {
        int total = 0, before_all = failure_count();
        for (auto& t : registry()) {
            int before = failure_count();
            std::cout << "[ RUN  ] " << t.name << "\n";
            t.fn();
            std::cout << (failure_count() == before ? "[  OK  ] " : "[ FAIL ] ") << t.name << "\n";
            ++total;
        }
        int failed = failure_count() - before_all;
        std::cout << total << " test case(s) run, " << failed << " check failure(s)\n";
        return failed == 0 ? 0 : 1;
    }

} // namespace wtest

#define WTEST_CONCAT_(a, b) a##b
#define WTEST_CONCAT(a, b) WTEST_CONCAT_(a, b)

#define TEST_CASE(name)                                                                          \
    static void WTEST_CONCAT(wtest_fn_, __LINE__)();                                             \
    static ::wtest::Registrar WTEST_CONCAT(wtest_reg_, __LINE__)(name, WTEST_CONCAT(wtest_fn_, __LINE__)); \
    static void WTEST_CONCAT(wtest_fn_, __LINE__)()

#define CHECK(cond)                                                                               \
    do { if (!(cond)) ::wtest::check_failed(#cond, __FILE__, __LINE__); } while (0)

#define CHECK_EQ(a, b)                                                                            \
    do {                                                                                          \
        if (!((a) == (b))) {                                                                      \
            std::ostringstream wtest_oss_;                                                        \
            wtest_oss_ << #a << " == " << #b << " (got '" << (a) << "' vs '" << (b) << "')";       \
            ::wtest::check_failed(wtest_oss_.str(), __FILE__, __LINE__);                          \
        }                                                                                          \
    } while (0)
