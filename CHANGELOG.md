# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2026-09-13

### Added

- `WLog::set_min_level()` / `WLog::reset_min_level()`: programmatic log level control that
  takes precedence over `WCPPCLI_LOG_LEVEL`.
- `set_color_enabled()` / `reset_color_enabled()` and the stream-aware
  `color_enabled(std::ostream&)` / `format(text, style, os)`: explicit colour control, plus
  `TERM=dumb` detection.
- `ui::interactive_enabled()` / `ui::set_interactive_enabled()` / `ui::reset_interactive_enabled()`:
  prompts never block or consume stdin when standard input is not a terminal.
- `Command::usage_error_code`: lets an application return a dedicated "usage error" exit
  code (for example `2`) for parse failures.
- `Command::epilog`: documentation/issue links and examples appended to `--help`.

### Changed

- **Breaking (logging):** every `WLog` level, including `INFO` and `SUCCESS`, now writes to
  stderr so stdout stays machine-readable. Applications that piped stdout for logs should
  read stderr instead.
- `ui::confirm`, `ui::input`, and `ui::select` write their prompts to stderr and return their
  defaults when non-interactive.

### Fixed

- Parsing a flag that requires a value but has none (for example `--output` at the end of
  the command line) now fails with a usage error instead of being silently ignored.
- Subcommands inherit the parent `usage_error_code`.

## [0.1.0]

### Added

- Initial release: `wcli` command/flag parsing, `wconf` configuration with schema validation,
  `wstyle`/`wui` terminal components and prompts, and `wlog` structured logging.

[Unreleased]: https://github.com/wkqco33/wcppcli/compare/v0.1.0...HEAD
[0.2.0]: https://github.com/wkqco33/wcppcli/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/wkqco33/wcppcli/releases/tag/v0.1.0
