# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [3.0.0] - 2025-06-27

### Changed

-   Converted certain `int`s to `long`s
-   Converted `float`s to `double`s
-   Converted `rand()` to `std::mt19937`
-   Programmatically output each number to the correct precision

### Removed

-   Verbose output (not supported any longer)
-   File output toggle (always enabled now)

## [2.2.2] - 2025-06-26

### Added

-   Added more input validation

## [2.2.1] - 2025-06-20

### Fixed

-   Fixed a bug adding an extraneous `endl` in the CSV

## [2.2.0] - 2025-06-20

### Added

-   Toggleable verbose console logging

## [2.1.0] - 2025-06-13

### Fixed

-   Moved header sanitization to fix a bug that would prevent headers from being split into separate tokens

## [2.0.0] - 2025-06-10

### Added

-   CSV output

### Changed

-   Replaced `std::cin` with `cxxopts`

## [1.0.0] - 2019-07-01

### Added

-   Initial release
