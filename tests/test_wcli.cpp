#include "wcppcli/wcli.hpp"
#include "test_framework.hpp"
#include <memory>
#include <vector>

using namespace wcppcli;

namespace {
    // argc/argv 를 흉내내기 위한 헬퍼 (첫 인자는 관례상 프로그램 이름).
    int run_cli(Command& root, std::vector<std::string> args) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>("prog"));
        for (auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
        return root.execute(static_cast<int>(argv.size()), argv.data());
    }
} // namespace

TEST_CASE("wcli parses bool/int/string flags via shorthand and --flag=value") {
    Command root;
    bool verbose = false;
    std::string name;
    int count = 0;
    Flag vf; vf.name = "verbose"; vf.shorthand = 'v'; vf.value_ptr = &verbose; root.add_flag(vf);
    Flag nf; nf.name = "name"; nf.value_ptr = &name; root.add_flag(nf);
    Flag cf; cf.name = "count"; cf.shorthand = 'c'; cf.value_ptr = &count; root.add_flag(cf);
    root.handler = [](const Command&) { return 0; };

    CHECK_EQ(run_cli(root, {"-v", "--name=alice", "-c", "3"}), 0);
    CHECK(verbose);
    CHECK_EQ(name, std::string("alice"));
    CHECK_EQ(count, 3);
}

TEST_CASE("wcli combines short bool flags and accepts inline short value (-p8080)") {
    Command root;
    bool a = false, b = false;
    int port = 0;
    Flag af; af.name = "aa"; af.shorthand = 'a'; af.value_ptr = &a; root.add_flag(af);
    Flag bf; bf.name = "bb"; bf.shorthand = 'b'; bf.value_ptr = &b; root.add_flag(bf);
    Flag pf; pf.name = "port"; pf.shorthand = 'p'; pf.value_ptr = &port; root.add_flag(pf);
    root.handler = [](const Command&) { return 0; };

    CHECK_EQ(run_cli(root, {"-ab", "-p8080"}), 0);
    CHECK(a);
    CHECK(b);
    CHECK_EQ(port, 8080);
}

TEST_CASE("wcli unknown flag returns exit code 1") {
    Command root;
    root.handler = [](const Command&) { return 0; };
    CHECK_EQ(run_cli(root, {"--nope"}), 1);
}

TEST_CASE("wcli invalid integer flag value returns exit code 1") {
    Command root;
    int n = 0;
    Flag nf; nf.name = "n"; nf.value_ptr = &n; root.add_flag(nf);
    root.handler = [](const Command&) { return 0; };
    CHECK_EQ(run_cli(root, {"--n", "notanumber"}), 1);
}

TEST_CASE("wcli required flag missing returns exit code 1, present returns handler result") {
    Command root;
    std::string val;
    Flag rf; rf.name = "req"; rf.value_ptr = &val; rf.required = true; root.add_flag(rf);
    root.handler = [](const Command&) { return 0; };

    CHECK_EQ(run_cli(root, {}), 1);
    CHECK_EQ(run_cli(root, {"--req", "x"}), 0);
}

TEST_CASE("wcli routes subcommands and collects positional args") {
    Command root;
    auto sub = std::make_unique<Command>();
    sub->name = "sub";
    std::vector<std::string> captured;
    sub->handler = [&captured](const Command& cmd) { captured = cmd.args; return 0; };
    root.add_command(std::move(sub));

    CHECK_EQ(run_cli(root, {"sub", "x", "y"}), 0);
    CHECK_EQ(captured.size(), static_cast<size_t>(2));
    CHECK_EQ(captured[0], std::string("x"));
    CHECK_EQ(captured[1], std::string("y"));
}

TEST_CASE("wcli propagates handler return value as process exit code") {
    Command root;
    root.handler = [](const Command&) { return 42; };
    CHECK_EQ(run_cli(root, {}), 42);
}

TEST_CASE("wcli --version prints and exits 0 without invoking the handler") {
    Command root;
    root.version = "v1.0.0";
    bool handler_called = false;
    root.handler = [&handler_called](const Command&) { handler_called = true; return 0; };

    CHECK_EQ(run_cli(root, {"--version"}), 0);
    CHECK(!handler_called);
}

TEST_CASE("wcli catches exceptions thrown from handlers and returns exit code 1") {
    Command root;
    root.handler = [](const Command&) -> int { throw std::runtime_error("boom"); };
    CHECK_EQ(run_cli(root, {}), 1);
}
