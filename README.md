# wcppcli (W-C++ CLI)

Go의 `cobra`, `viper` 그리고 Python의 `rich` 라이브러리에서 영감을 받아 제작된 C++용 고성능 CLI 프레임워크입니다. 복잡한 객체지향 설계를 배제하고 **데이터 중심(Data-driven)** 설계와 **성능 최우선** 원칙을 따르며, C++20 모던 문법을 지원합니다.

## 🚀 핵심 기능

- **wcli (Command & Flag Parsing)**: 
  - 계층형 명령어 구조와 타입 세이프한 플래그 파싱.
  - 포지셔널 인자(Positional Args) 지원으로 명령어 유연성 확보.
- **wconf (Configuration Management)**: 
  - **다양한 포맷 지원**: JSON, TOML, YAML, INI 형식을 자체 경량 파서로 처리.
  - **중첩 구조 지원**: 점 표기법(`server.port`)을 통한 계층형 설정 관리.
  - **확장자 자동 감지**: `read_file(path)` 하나로 모든 지원 포맷 자동 파싱.
  - **파일 자동 생성 (`ensure_file`)**: 설정 파일 부재 시 현재 설정된 기본값으로 파일 자동 생성.
  - **우선순위**: 환경 변수(`MYAPP_SERVER_PORT`) > 설정 파일 > 기본값.
- **wstyle (Terminal Styling & UI)**: 
  - ANSI 이스케이프 시퀀스 기반의 스타일링.
  - 표(Table), 진행 바(ProgressBar), 패널(Panel) 등 고수준 터미널 UI 컴포넌트 제공.

## 🛠️ 빌드 방법 (라이브러리 자체 빌드)

```bash
mkdir build && cd build
cmake ..
make
```

## 📖 라이브러리 사용 가이드

### 1. wconf (강화된 설정 관리)
다양한 설정 파일 포맷과 중첩 구조를 지원합니다.

```cpp
#include "wcppcli/wconf.hpp"

using namespace wcppcli;

WConf conf;
conf.set("server.port", 8080); // 기본값 설정

// 파일이 없으면 위 기본값으로 .yaml 파일 자동 생성, 있으면 로드
conf.ensure_file("config.yaml"); 

// 점 표기법으로 계층형 데이터 접근
int port = conf.get_int("server.port");
```

### 2. wstyle (패널 및 UI)
터미널 레이아웃을 패널로 감싸 가독성을 높일 수 있습니다.

```cpp
#include "wcppcli/wstyle.hpp"

Panel p;
p.title = "Server Status";
p.content = "Running on 0.0.0.0:8080";
p.border_style = {.fg = Color::Cyan};
p.render();
```

## 📦 다른 프로젝트에 통합하기

`wcppcli`는 외부 의존성이 전혀 없는 정적 라이브러리입니다. 다음과 같이 프로젝트에 통합할 수 있습니다.

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
3. 컴파일러 플래그에 `-std=c++20`을 추가하고 라이브러리를 링크합니다.

```bash
g++ -std=c++20 main.cpp -I./include -L./build -lwcppcli -o my_app
```

## 🧩 통합 예제 (Full Example)
모든 기능이 유기적으로 결합된 예제를 실행해보세요.
- `examples/nested_config_example.cpp`: 중첩 구조 지원 예제.
- `examples/ensure_file_example.cpp`: 설정 파일 자동 생성 예제.
- `examples/full_example.cpp`: 전체 기능 통합 예제.

## ⚖️ 설계 철학
- **성능 최우선**: 불필요한 메모리 할당과 추상화를 배제합니다.
- **데이터 중심**: 모든 설정과 명령어를 명확한 데이터 구조로 정의합니다.
- **의존성 제로**: 외부 라이브러리 없이 표준 C++만으로 모든 기능을 완벽히 처리합니다.
