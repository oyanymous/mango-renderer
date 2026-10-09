# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & run

CMake (≥ 3.24) with a single preset, `macos` (Apple Clang, Debug, output in `build/macos/`):

```bash
cmake --preset macos
cmake --build --preset macos
./build/macos/main
```

There are no tests or lint configuration yet. The top-level `build/` directory (outside `build/macos/`) holds stale output from an earlier non-preset configure (e.g. an old `Renderer` target) — ignore it.

## Architecture

Early-stage macOS renderer. Currently only a platform layer exists: `main.cpp` creates an `Application` and a `Window`, then calls `app.run()` (blocks in the Cocoa run loop).

- **`platform/`** — static library target `Platform`, linked by `main`.
  - Public headers (`Application.h`, `Window.h`) are plain C++ and use the **pimpl idiom** (`struct Impl; std::unique_ptr<Impl> m_impl;`) so no Objective-C/AppKit types leak into C++ translation units.
  - Platform-specific implementations live in per-OS subdirectories; `platform/darwin/*.mm` are Objective-C++ files that define `Impl` with AppKit objects (`NSApplication`, `NSWindow`, delegate classes). New OS backends would follow the same pattern alongside `darwin/`.
  - The project enables the `OBJCXX` language in the root `CMakeLists.txt`; Cocoa is linked publicly via `-framework Cocoa`.
- `Platform`'s include directory is `PRIVATE`, so consumers include headers by repo-relative path (`#include "platform/Window.h"`), while files inside `platform/` include them bare (`#include "Window.h"`).
- ARC is not enabled (no `-fobjc-arc`), so Objective-C objects in `.mm` files are under manual reference counting.
- `Application` must be constructed before any `Window`, since it initializes `[NSApplication sharedApplication]`.

## Conventions

- Member fields use the `m_` prefix with snake_case (`m_impl`, `m_window_delegate`); headers use `#pragma once`.
- Commit messages must follow [Conventional Commits](https://www.conventionalcommits.org/): `<type>(<optional scope>): <description>`, e.g. `feat(platform): add window resize callback`, `fix(darwin): release window delegate`. Common types: `feat`, `fix`, `refactor`, `build`, `docs`, `chore`, `test`, `perf`. Mark breaking changes with `!` after the type/scope or a `BREAKING CHANGE:` footer.
