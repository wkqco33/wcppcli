#include "wcppcli/wconf.hpp"
#include "wcppcli/wstyle.hpp"
#include <iostream>

using namespace wcppcli;

int main() {
    WConf conf;

    // 1. 스키마 등록
    // - port: 필수, 1~65535 범위
    conf.add_schema("port", [](const WConf::ValueType& v) {
        if (std::holds_alternative<int>(v)) {
            int p = std::get<int>(v);
            return p >= 1 && p <= 65535;
        }
        return false;
    }, true);

    // - env: 필수 아님, "dev", "prod", "test" 중 하나
    conf.add_schema("env", [](const WConf::ValueType& v) {
        if (std::holds_alternative<std::string>(v)) {
            std::string s = std::get<std::string>(v);
            return s == "dev" || s == "prod" || s == "test";
        }
        return false;
    });

    // 2. 데이터 설정 (정상 케이스)
    conf.set("port", 8080);
    conf.set("env", std::string("dev"));

    print("--- Schema Validation Test (Valid Data) ---", Style(Color::Cyan, Color::None, true));
    if (conf.validate()) {
        print("Validation PASSED as expected.", Style(Color::Green));
    } else {
        print("Validation FAILED (Unexpected).", Style(Color::Red));
    }

    // 3. 잘못된 데이터 설정 (범위 초과)
    conf.set("port", 70000);
    print("\n--- Schema Validation Test (Invalid Range) ---", Style(Color::Cyan, Color::None, true));
    if (!conf.validate()) {
        print("Validation FAILED as expected (Port 70000 is invalid).", Style(Color::Green));
    } else {
        print("Validation PASSED (Unexpected).", Style(Color::Red));
    }

    // 4. 필수 값 누락 테스트 (port 제거)
    // WConf에 키 삭제 기능이 없으므로, CLI 오버라이드나 다른 방식으로 테스트할 수도 있지만
    // 여기서는 새로운 인스턴스로 테스트합니다.
    WConf conf2;
    conf2.add_schema("port", nullptr, true); // 필수 값으로만 등록
    print("\n--- Schema Validation Test (Missing Required) ---", Style(Color::Cyan, Color::None, true));
    if (!conf2.validate()) {
        print("Validation FAILED as expected (Port is missing).", Style(Color::Green));
    } else {
        print("Validation PASSED (Unexpected).", Style(Color::Red));
    }

    return 0;
}
