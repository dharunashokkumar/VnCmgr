# Contributing to VMManager

Thank you for your interest in contributing to VMManager! This guide will help you get started.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Project Architecture](#project-architecture)
- [Making Changes](#making-changes)
- [Coding Standards](#coding-standards)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Reporting Bugs](#reporting-bugs)
- [Requesting Features](#requesting-features)

## Code of Conduct

This project follows our [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

## Getting Started

1. **Fork** the repository on GitHub
2. **Clone** your fork locally:
   ```bash
   git clone https://github.com/YOUR-USERNAME/vmmanager.git
   cd vmmanager
   ```
3. **Add the upstream remote:**
   ```bash
   git remote add upstream https://github.com/TripleETech/vmmanager.git
   ```
4. **Create a branch** for your work:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Setup

### Install Dependencies

```bash
# Ubuntu/Debian
sudo apt install gcc make pkg-config libgtk-4-dev libvirt-dev libcurl4-openssl-dev libjson-glib-dev

# Runtime (for testing)
sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients
```

### Build

```bash
make clean && make          # Release build
make debug                  # Debug build with symbols
make run                    # Build and run
make test                   # Build test programs
```

### Verify Your Setup

```bash
# Check all dependencies are available
pkg-config --exists gtk4 libvirt libcurl json-glib-1.0 && echo "Ready" || echo "Missing deps"
```

## Project Architecture

VMManager follows a modular C architecture:

```
src/
├── main.c                 # App entry, logging, lifecycle
├── ui/
│   ├── window.c           # Main UI (sidebar, dashboard, panels, CSS)
│   └── dialogs.c          # Dialogs, settings, loading, shortcuts, console
├── backend/
│   ├── vm_manager.c       # libvirt API for QEMU/KVM VMs
│   ├── ct_manager.c       # Incus/LXD REST API via libcurl
│   └── snapshot_manager.c # VM snapshot operations
└── utils/
    ├── error_handling.c   # Error codes, mapping, messages
    └── system_info.c      # /proc-based system monitoring
```

**Key design principles:**
- Single header file (`include/vmmanager.h`) for all declarations
- `AppData` struct is the central state object passed everywhere
- UI code never calls system APIs directly — always through backend functions
- Error handling uses structured `AppError` with codes, messages, and suggestions
- No `system()` calls — all process spawning uses `fork/exec`

## Making Changes

### Before You Start

- Check existing [issues](https://github.com/TripleETech/vmmanager/issues) to avoid duplicate work
- For large changes, open an issue first to discuss the approach
- Keep changes focused — one feature or fix per PR

### Development Workflow

1. Sync with upstream:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. Make your changes

3. Build and verify:
   ```bash
   make clean && make
   ```

4. Test your changes manually by running the app:
   ```bash
   ./vmmanager
   ```

5. Check for compiler warnings — we compile with `-Wall -Wextra`

## Coding Standards

### C Style

- **Naming:** `snake_case` for functions and variables, `UPPER_CASE` for constants and macros
- **Indentation:** 4 spaces, no tabs
- **Braces:** Opening brace on the same line
- **Line length:** Aim for 100 characters, hard limit at 120
- **Comments:** Use `/* */` for block comments, `//` is acceptable for inline

### Function Conventions

```c
/* ── Section Header ──────────────────────────────── */
bool vm_start(AppData *app, const char *name) {
    if (!app->virt_conn) {
        app_log(app, "ERROR", "Not connected");
        return false;
    }

    // ... implementation

    app_log(app, "INFO", "VM '%s' started", name);
    return true;
}
```

### Error Handling

- Always check return values from library calls
- Use `app_log()` for logging errors with context
- Use `AppError` for user-facing errors with suggestions
- Never silently swallow errors

### Memory Management

- Check `malloc`/`calloc` return values
- Free resources in reverse order of allocation
- Use `g_free()` for GLib-allocated memory, `free()` for stdlib
- Null pointers after freeing when the pointer remains in scope

### Security

- Never use `system()` — use `fork/exec` for process spawning
- Validate all user input (VM names, paths, etc.)
- Use `snprintf` instead of `sprintf`
- Use `strncpy` with explicit null termination

## Commit Messages

Follow conventional commit style:

```
<type>: <short description>

<optional longer description>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `refactor`: Code restructuring without behavior change
- `docs`: Documentation only
- `style`: Formatting, no code change
- `test`: Adding or updating tests
- `build`: Build system or dependency changes

**Examples:**
```
feat: add VM disk resize support
fix: handle null libvirt connection in snapshot list
docs: add Fedora installation instructions
refactor: extract CSS into separate resource file
```

## Pull Request Process

1. **Update your branch** with the latest upstream changes
2. **Ensure clean build** with no warnings: `make clean && make`
3. **Write a clear PR description** explaining:
   - What the change does
   - Why it's needed
   - How to test it
4. **Link related issues** using "Fixes #123" or "Closes #123"
5. **Keep PRs small and focused** — easier to review and merge
6. **Respond to review feedback** promptly

### PR Title Format

Use the same format as commit messages:
```
feat: add VM disk resize support
fix: container stop timeout not respected
```

## Reporting Bugs

Use the [Bug Report template](.github/ISSUE_TEMPLATE/bug_report.md) and include:

1. **System info:** distro, GTK version, libvirt version
2. **Steps to reproduce** the issue
3. **Expected behavior** vs. **actual behavior**
4. **Log output** from the Activity Log panel or stderr
5. **Screenshots** if it's a UI issue

## Requesting Features

Use the [Feature Request template](.github/ISSUE_TEMPLATE/feature_request.md) and include:

1. **Problem statement** — what's missing or painful
2. **Proposed solution** — how you'd like it to work
3. **Alternatives considered**
4. **Additional context** — mockups, related tools, etc.

## Areas Where Help is Needed

Here are some areas where contributions would be particularly valuable:

- **Packaging:** Flatpak, Snap, AUR, .deb, .rpm packages
- **Testing:** Unit tests, integration tests with libvirt test driver
- **Documentation:** User guides, tutorials, translations
- **UI/UX:** Dark theme, accessibility improvements, responsive layouts
- **Features:** See the [Roadmap](README.md#roadmap)

## Questions?

If you have questions about contributing, feel free to open a [Discussion](https://github.com/TripleETech/vmmanager/discussions) or ask in an issue.

Thank you for helping make VMManager better!
