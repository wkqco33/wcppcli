# Agent Development Guide

## Project overview

`wcppcli` is a dependency-free C++17 library providing command parsing,
configuration management, terminal UI helpers, and logging. Public headers are
under `include/wcppcli`; implementation files are under `src`.

## Development workflow

Use the TDD cycle for behavior changes:

1. Add or update a focused test that describes the desired behavior.
2. Run the smallest relevant test with `--filter`.
3. Implement the change.
4. Run the focused test again, then the complete suite.
5. Refactor only after the tests pass.

Do not commit generated build directories or installed package files.

## Configure and test

From the repository root:

```sh
cmake -S . -B build \
  -DWCPPCLI_BUILD_EXAMPLES=ON \
  -DWCPPCLI_BUILD_TOOL=ON \
  -DWCPPCLI_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run one focused test or list available tests:

```sh
build/wcppcli_tests --filter "wconf priority"
build/wcppcli_tests --list
```

On Visual Studio generators, use the configuration explicitly:

```powershell
cmake --build build --config Debug --parallel
ctest --test-dir build -C Debug --output-on-failure
```

The test executable uses the project-local lightweight harness in
`tests/test_framework.hpp`; do not add an external test dependency unless the
project direction changes.

## Test conventions

- Keep tests deterministic and independent.
- Use temporary files created in the test working directory and remove them
  before the test returns.
- Use `wtest::set_env` and `wtest::unset_env` for environment-variable tests;
  they are portable across POSIX and Windows.
- Prefer one behavior assertion per focused test case and descriptive names.
- Add regression coverage for bug fixes before changing production code.
- Keep CTest integration working when adding test source files.

## CMake and packaging

- The library target is `wcppcli`; installed consumers use
  `wcppcli::wcppcli` through `find_package(wcppcli CONFIG REQUIRED)`.
- Keep examples, the `wcli` tool, and tests behind their existing CMake
  options.
- Public headers must remain self-contained and compile with C++17.
- Keep the install/export rules and vcpkg files synchronized with release
  metadata.
- vcpkg releases must pin a public Git revision or tag and its matching
  archive hash; do not use an unpublished local commit.

## Portability and warnings

- Support POSIX and Windows builds.
- Use the platform-safe environment helper in `src/environment.hpp` rather
  than calling `getenv` directly in library code.
- Keep MSVC UTF-8 compilation enabled through the top-level CMake file.
- Treat new compiler warnings as issues to fix, especially narrowing
  conversions and platform-specific API failures.

## Change scope

Make focused changes, preserve existing public API behavior, and update the
README when user-facing build or packaging behavior changes. Do not silently
change parser semantics or CLI exit codes without regression tests.
