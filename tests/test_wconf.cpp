#include "wcppcli/wconf.hpp"
#include "test_framework.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>

using namespace wcppcli;

TEST_CASE("wconf read_json parses nested dot-notation keys") {
    { std::ofstream f("test_wconf_nested.json"); f << "{\n  \"server\": {\n    \"host\": \"127.0.0.1\",\n    \"port\": 8080\n  }\n}\n"; }
    WConf conf;
    CHECK(conf.read_json("test_wconf_nested.json"));
    CHECK_EQ(conf.get_string("server.host"), std::string("127.0.0.1"));
    CHECK_EQ(conf.get_int("server.port"), 8080);
    std::remove("test_wconf_nested.json");
}

TEST_CASE("wconf read_toml parses sections into dot-notation keys") {
    { std::ofstream f("test_wconf.toml"); f << "[server]\nhost = \"localhost\"\nport = 9090\n"; }
    WConf conf;
    CHECK(conf.read_toml("test_wconf.toml"));
    CHECK_EQ(conf.get_string("server.host"), std::string("localhost"));
    CHECK_EQ(conf.get_int("server.port"), 9090);
    std::remove("test_wconf.toml");
}

TEST_CASE("wconf read_yaml tracks nested mappings via indentation") {
    { std::ofstream f("test_wconf.yaml"); f << "server:\n  host: localhost\n  port: 6060\n"; }
    WConf conf;
    CHECK(conf.read_yaml("test_wconf.yaml"));
    CHECK_EQ(conf.get_string("server.host"), std::string("localhost"));
    CHECK_EQ(conf.get_int("server.port"), 6060);
    std::remove("test_wconf.yaml");
}

TEST_CASE("wconf get_int falls back to 0 on non-numeric string") {
    WConf conf;
    conf.set("bad", std::string("not-a-number"));
    CHECK_EQ(conf.get_int("bad"), 0);
}

TEST_CASE("wconf get_bool coerces common truthy/falsy strings") {
    WConf conf;
    conf.set("a", std::string("true"));
    conf.set("b", std::string("1"));
    conf.set("c", std::string("no"));
    CHECK(conf.get_bool("a"));
    CHECK(conf.get_bool("b"));
    CHECK(!conf.get_bool("c"));
}

TEST_CASE("wconf priority order is cli > env > file > default") {
    { std::ofstream f("test_wconf_priority.toml"); f << "port = 9090\n"; }
    WConf conf;
    conf.set("port", 8080); // default
    CHECK(conf.read_toml("test_wconf_priority.toml")); // file overrides default
    CHECK_EQ(conf.get_int("port"), 9090);

    setenv("APP_PORT", "1010", 1);
    conf.set_env_prefix("APP");
    conf.bind_env("port", "APP_PORT");
    CHECK_EQ(conf.get_int("port"), 1010); // env overrides file

    conf.set_cli("port", 1212);
    CHECK_EQ(conf.get_int("port"), 1212); // cli overrides env

    unsetenv("APP_PORT");
    std::remove("test_wconf_priority.toml");
}

TEST_CASE("wconf validate_errors reports missing-required and rejected-value separately") {
    WConf conf;
    conf.add_schema("name", nullptr, true); // required, no validator
    conf.add_schema("port", [](const WConf::ValueType& v) {
        return std::holds_alternative<int>(v) && std::get<int>(v) > 0;
    }, false);
    conf.set("port", -1);

    auto errors = conf.validate_errors();
    CHECK_EQ(errors.size(), static_cast<size_t>(2));
    // schemas_ 는 std::map 이라 알파벳 순 ("name" < "port")
    CHECK_EQ(errors[0].key, std::string("name"));
    CHECK_EQ(errors[0].reason, std::string("missing required key"));
    CHECK_EQ(errors[1].key, std::string("port"));
    CHECK_EQ(errors[1].reason, std::string("validator rejected value"));
    CHECK(!conf.validate());
}

TEST_CASE("wconf get_array parses inline arrays from json/toml/yaml") {
    { std::ofstream f("test_wconf_arr.json"); f << "{\n  \"tags\": [\"a\", \"b\", \"c\"]\n}\n"; }
    { std::ofstream f("test_wconf_arr.toml"); f << "tags = [\"x\", \"y\"]\n"; }
    { std::ofstream f("test_wconf_arr.yaml"); f << "tags: [p, q]\n"; }

    WConf j, t, y;
    CHECK(j.read_file("test_wconf_arr.json"));
    CHECK(t.read_file("test_wconf_arr.toml"));
    CHECK(y.read_file("test_wconf_arr.yaml"));

    auto jt = j.get_array("tags");
    CHECK_EQ(jt.size(), static_cast<size_t>(3));
    CHECK_EQ(jt[0], std::string("a"));
    CHECK_EQ(jt[2], std::string("c"));
    CHECK_EQ(t.get_array("tags").size(), static_cast<size_t>(2));
    CHECK_EQ(y.get_array("tags").size(), static_cast<size_t>(2));
    CHECK_EQ(j.get_array("nonexistent").size(), static_cast<size_t>(0));

    std::remove("test_wconf_arr.json");
    std::remove("test_wconf_arr.toml");
    std::remove("test_wconf_arr.yaml");
}

TEST_CASE("wconf write_file round-trips arrays") {
    WConf conf;
    conf.set("tags", std::vector<std::string>{"a", "b"});
    CHECK(conf.write_file("test_wconf_roundtrip.json"));

    WConf reloaded;
    CHECK(reloaded.read_file("test_wconf_roundtrip.json"));
    auto arr = reloaded.get_array("tags");
    CHECK_EQ(arr.size(), static_cast<size_t>(2));
    CHECK_EQ(arr[0], std::string("a"));
    CHECK_EQ(arr[1], std::string("b"));
    std::remove("test_wconf_roundtrip.json");
}
