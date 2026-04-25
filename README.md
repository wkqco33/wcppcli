# wcppcli (W-C++ CLI)

Go의 `cobra`, `viper` 그리고 Python의 `rich` 라이브러리에서 영감을 받아 제작된 C++용 고성능 CLI 프레임워크입니다. 복잡한 객체지향 설계를 배제하고 **데이터 중심(Data-driven)** 설계와 **성능 최우선** 원칙을 따르며, **C++17** 표준을 지원하여 다양한 프로젝트 환경에서 높은 호환성을 제공합니다.

## 🚀 핵심 기능

- **wcli (Command & Flag Parsing)**: 
  - 계층형 명령어 구조와 타입 세이프한 플래그 파싱.
  - **설정값 우선순위 병합**: CLI 플래그 > 환경 변수 > 설정 파일 > 기본값 순의 계층적 설정 시스템.
  - **쉘 자동 완성**: Bash용 자동 완성 스크립트(`generate_bash_completion`) 생성 지원.
- **wconf (Configuration Management)**: 
  - **다양한 포맷 지원**: JSON, TOML, YAML, INI 형식을 자체 경량 파서로 처리.
  - **중첩 구조 지원**: 점 표기법(`server.port`)을 통한 계층형 설정 관리.
  - **데이터 스키마 검증**: 필수 값 체크 및 사용자 정의 검증기(`Validator`)를 통한 무결성 검사.
- **wstyle & wui (Terminal UI & Prompts)**: 
  - **UI 컴포넌트**: 표(Table), 진행 바(ProgressBar), 패널(Panel) 제공.
  - **인터랙티브 프롬프트**: `confirm`, `input`, `select` 등 사용자 입력 UI 지원.
- **wlog (Structured Logging)**: 
  - `DEBUG`, `INFO`, `SUCCESS`, `WARN`, `ERROR` 레벨별 스타일 로깅 제공.

## ⚡ wcli 제너레이터 (Code Generator)

Cobra CLI와 유사하게, `wcppcli` 기반 프로젝트를 빠르게 시작하고 체계적인 구조로 명령어를 확장할 수 있는 도구를 제공합니다.

### 1. 제너레이터 빌드
프로젝트 루트에서 빌드를 수행하면 `build/wcli` 실행 파일이 생성됩니다.
```bash
mkdir build && cd build
cmake ..
make wcli
```

### 2. 프로젝트 초기화 (`init`)
새로운 CLI 앱의 폴더 구조와 빌드 환경을 자동으로 구성합니다.
```bash
# 기본 이름(myapp)으로 초기화
./wcli init 
# 특정 이름으로 초기화
./wcli init my-cool-app
```
**생성되는 체계적 구조:**
- `include/[project_name]/`: 커맨드 헤더 파일(`.hpp`) 관리
- `src/commands/`: 커맨드 구현부(`.cpp`) 관리 (자동 빌드 대상)
- `src/main.cpp`: 프로그램 진입점 및 루드 커맨드 설정
- `CMakeLists.txt`: `wcppcli` 라이브러리 연동 및 소스 자동 스캔 설정

### 3. 명령어 추가 (`add`)
기존 프로젝트에 새로운 서브커맨드 템플릿을 추가합니다.
```bash
./wcli add serve
```
- `include/[project_name]/cmd_serve.hpp` 및 `src/commands/cmd_serve.cpp`가 생성됩니다.
- 생성된 파일은 `CMakeLists.txt`에 의해 자동으로 컴파일 대상에 포함됩니다.

## 🛠️ 빌드 방법 (라이브러리 자체 빌드)

```bash
mkdir build && cd build
cmake ..
make
```

## 📖 라이브러리 사용 가이드

### 1. 설정 우선순위 및 스키마 검증 (wconf)
CLI 플래그와 환경 변수를 자동으로 병합하며, 데이터의 유효성을 검사할 수 있습니다.

```cpp
#include "wcppcli/wconf.hpp"

WConf conf;
// 스키마 등록: 필수 값 여부 및 범위 검증
conf.add_schema("port", [](const WConf::ValueType& v) {
    int p = std::get<int>(v);
    return p >= 1 && p <= 65535;
}, true);

conf.set("port", 8080); // 기본값
if (!conf.validate()) { /* 에러 처리 */ }
```

### 2. 인터랙티브 UI 및 로깅 (wui, wlog)
사용자와 상호작용하고 상태를 시각적으로 출력합니다.

```cpp
#include "wcppcli/wui.hpp"
#include "wcppcli/wlog.hpp"

using namespace wcppcli;

// 사용자 입력 받기
if (ui::confirm("서비스를 시작할까요?")) {
    WLog::info("서비스 시작 중...");
    // ...
    WLog::success("서비스가 실행되었습니다.");
}
```

### 3. CLI 자동 완성 지원 (wcli)
등록된 명령어 구조를 바탕으로 Bash 자동 완성 스크립트를 생성합니다.

```cpp
Command root;
root.name = "myapp";
// ... 명령어 및 플래그 등록 ...
std::cout << root.generate_bash_completion() << std::endl;
```

## 📦 프로젝트 통합 가이드

`wcppcli`는 외부 의존성이 전혀 없는 정적 라이브러리입니다.

### 방법 A: CMake `add_subdirectory` (권장)
사용자의 프로젝트 하위 디렉토리에 `wcppcli`를 포함시킨 후 `CMakeLists.txt`에 추가합니다.

```cmake
# 사용자의 CMakeLists.txt
add_subdirectory(third_party/wcppcli)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE wcppcli)
```

### 방법 B: 빌드된 라이브러리 직접 링크
1. `build` 디렉토리에서 생성된 `libwcppcli.a`를 가져옵니다.
2. `include/wcppcli` 헤더 파일들을 프로젝트의 헤더 경로에 추가합니다.
3. 컴파일러 플래그에 `-std=c++17`을 추가하고 라이브러리를 링크합니다.

## 🧩 주요 예제 목록
- `examples/priority_merge_example.cpp`: 설정 우선순위(CLI > Env > File) 테스트.
- `examples/schema_validation_example.cpp`: 데이터 유효성 검증 예제.
- `examples/interactive_example.cpp`: 인터랙티브 프롬프트(`confirm`, `input`, `select`) 예제.
- `examples/log_example.cpp`: 레벨별 스타일 로깅 예제.
- `examples/completion_example.cpp`: Bash 자동 완성 스크립트 생성 예제.
- `examples/full_example.cpp`: 전체 기능 통합 예제.

## ⚖️ 설계 철학
- **성능 최우선**: 불필요한 메모리 할당과 추상화를 배제합니다.
- **데이터 중심**: 모든 설정과 명령어를 명확한 데이터 구조로 정의합니다.
- **의존성 제로**: 외부 라이브러리 없이 표준 C++(C++17)만으로 모든 기능을 완벽히 처리합니다.
