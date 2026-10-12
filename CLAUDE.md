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

Early-stage macOS renderer. `src/main.cpp` creates an `Application` and a top-level `Widget`, then calls `app.run()` (blocks in the Cocoa run loop).

Three library targets, with dependencies flowing `UI → Platform → Core`:

- **`src/core`** — header-only `INTERFACE` target `Core`. Holds abstract interfaces shared across layers (e.g. `IWidget`), so `src/platform` can refer to widgets without depending on `src/ui`.
- **`src/platform`** — static library target `Platform` (links `Core` and Cocoa publicly).
  - `Application` is a thin public facade over the `PrivateApplication` singleton, which owns the list of top-level `IWidget*`s and the native app state. Constructing `Application` initializes `[NSApplication sharedApplication]`, so it must come before any `Window` or `Widget`.
  - Classes holding native state (`PrivateApplication`, `Window`) use the **pimpl idiom** (`struct Impl; std::unique_ptr<Impl> m_impl;`) so their headers stay plain C++ and no Objective-C/AppKit types leak into C++ translation units.
  - Platform-specific implementations live in per-OS subdirectories; `src/platform` are Objective-C++ files that define `Impl` with AppKit objects (`NSApplication`, `NSWindow`, delegate classes). New OS backends would follow the same pattern alongside `darwin/`.
  - The project enables the `OBJCXX` language in the root `src/CMakeLists.txt`.
- **`src/ui`** — static library target `UI` (links `Platform` and `Core`). `Widget` implements `IWidget`; a parentless `Widget` registers itself via `Application::addTopLevelWidget`. Also holds geometry/value types (`Point`, `Size`, `Rect`, `Line`, `Polygon`, `Color`).
- ARC is not enabled (no `-fobjc-arc`), so Objective-C objects in `.mm` files are under manual reference counting.

### Include style

Each library exports **its own directory** as its include path (`Core` via `INTERFACE`, `Platform` via `PUBLIC`), so headers are included **bare**, without a module prefix, both inside and across modules: `#include "IWidget.h"`, `#include "Window.h"`, `#include "Application.h"`. A `"platform/..."` or `"core/..."` path will not resolve from another module. When adding a new library, export its own directory the same way. Header names must therefore be unique across modules.

`src/main.cpp` is the exception: it uses `"platform/Application.h"` / `"ui/Widget.h"`, which resolve only because it sits at the repo root and quoted includes search the including file's directory first.

## Conventions

- Member fields use the `m_` prefix with snake_case (`m_impl`, `m_window_delegate`); headers use `#pragma once`.
- Commit messages must follow [Conventional Commits](https://www.conventionalcommits.org/): `<type>(<optional scope>): <description>`, e.g. `feat(platform): add window resize callback`, `fix(darwin): release window delegate`. Common types: `feat`, `fix`, `refactor`, `build`, `docs`, `chore`, `test`, `perf`. Mark breaking changes with `!` after the type/scope or a `BREAKING CHANGE:` footer.
