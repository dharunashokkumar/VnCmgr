# Contributing to VMManager

Thank you for your interest in contributing to VMManager! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Project Structure](#project-structure)
- [Coding Standards](#coding-standards)
- [Making Changes](#making-changes)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Reporting Bugs](#reporting-bugs)
- [Requesting Features](#requesting-features)
- [Security Vulnerabilities](#security-vulnerabilities)

---

## Code of Conduct

This project adheres to a [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

---

## Getting Started

### Prerequisites

- **C compiler:** GCC 11+ or Clang 14+
- **Build system:** GNU Make
- **Package manager:** pkg-config
- **Libraries:** GTK 4, libvirt, libcurl, json-glib
- **Runtime:** QEMU/KVM, libvirt daemon, optionally Incus/LXD

### First-time Setup

1. **Fork** the repository on GitHub
2. **Clone** your fork:
   ```bash
   git clone https://github.com/YOUR_USERNAME/vmmanager.git
   cd vmmanager
   ```
3. **Add upstream remote:**
   ```bash
   git remote add upstream https://github.com/TripleETech/vmmanager.git
   ```
4. **Install dependencies:**
   ```bash
   # Ubuntu/Debian
   sudo apt install gcc make pkg-config libgtk-4-dev libvirt-dev \
       libcurl4-openssl-dev libjson-glib-dev

   # Fedora
   sudo dnf install gcc make pkgconf-pkg-config gtk4-devel \
       libvirt-devel libcurl-devel json-glib-devel
   ```
5. **Build and verify:**
   ```bash
   make check-deps
   make debug
   ./vmmanager
   ```

---

## Development Setup

### Debug Build

```bash
make debug     # Builds with -g -O0 -DDEBUG flags
./vmmanager    # Logs go to stderr
```

### Running Tests

```bash
make test      # Build test binaries
cd tests && bash test_ct.sh  # Run shell-based tests
```

### Checking Dependencies

```bash
make check-deps
```

### Useful Tools

- **Valgrind** for memory leak detection:
  ```bash
  valgrind --leak-check=full ./vmmanager
  ```
- **GDB** for debugging:
  ```bash
  make debug
  gdb ./vmmanager
  ```
- **GTK Inspector** for UI debugging:
  ```bash
  GTK_DEBUG=interactive ./vmmanager
  ```

---

## Project Structure

```
src/
├── main.c                  # Entry point, logging, app lifecycle
├── ui/
│   ├── window.c            # Main window, sidebar, all panels
│   └── dialogs.c           # Dialogs, settings, console, shortcuts
├── backend/
│   ├── vm_manager.c        # libvirt C API for VMs
│   ├── ct_manager.c        # Incus REST API for containers
│   └── snapshot_manager.c  # VM snapshots
└── utils/
    ├── error_handling.c    # Error codes, messages, suggestions
    └── system_info.c       # CPU/RAM/disk monitoring
```

### Key Patterns

- **AppData** struct (`include/vmmanager.h`) is the central application state — passed everywhere
- **Error handling** uses `AppError` structs with error codes, messages, and user-facing suggestions
- **Backend functions** return `bool` for success/failure and log errors via `app_log()`
- **UI functions** are prefixed with `ui_` and live in `src/ui/`
- **Backend functions** are prefixed with `vm_`, `ct_`, or `sys_`

---

## Coding Standards

### C Style

- **Standard:** C11 with GNU extensions (`_GNU_SOURCE`)
- **Indentation:** 4 spaces (no tabs)
- **Line length:** Soft limit 100 characters
- **Braces:** Opening brace on same line
- **Naming:** `snake_case` for functions and variables, `UPPER_CASE` for constants
- **Pointer style:** `char *ptr` (space before `*`, not after)

### Example

```c
/* Good */
bool vm_start(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) {
        app_log(app, "ERROR", "VM '%s' not found", name);
        return false;
    }

    int ret = virDomainCreate(dom);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to start VM '%s'", name);
        return false;
    }

    app_log(app, "INFO", "VM '%s' started", name);
    return true;
}
```

### Guidelines

1. **Always check return values** from system calls and library functions
2. **Always free allocated memory** — use `free()`, `g_free()`, `g_object_unref()` as appropriate
3. **Use `strncpy` with explicit null-termination** — never use `strcpy`
4. **Use `snprintf` instead of `sprintf`** — always prevent buffer overflows
5. **Use `fork`/`exec` instead of `system()`** — prevents command injection
6. **Validate all user input** before passing to system APIs
7. **Log operations** using `app_log()` with appropriate levels: `INFO`, `WARN`, `ERROR`
8. **Handle NULL pointers** at function entry points
9. **No compiler warnings** — code must compile cleanly with `-Wall -Wextra`

### Security Requirements

- Never use `system()` or `popen()` — use `fork()`/`exec()` family
- Validate and sanitize all inputs that go into XML, JSON, or system commands
- Check buffer sizes before string operations
- Don't hardcode paths — use runtime detection
- Don't store credentials in source code

---

## Making Changes

### Branching Strategy

- `main` — stable, release-ready code
- `feature/*` — new features
- `fix/*` — bug fixes
- `docs/*` — documentation changes

### Workflow

1. **Sync with upstream:**
   ```bash
   git fetch upstream
   git checkout main
   git merge upstream/main
   ```

2. **Create a feature branch:**
   ```bash
   git checkout -b feature/my-feature
   ```

3. **Make your changes** following the coding standards above

4. **Build and test:**
   ```bash
   make clean && make
   ./vmmanager  # Manual testing
   make test    # Run tests
   ```

5. **Check for warnings:**
   ```bash
   make clean && make 2>&1 | grep -i warning
   ```

---

## Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `style` | Code style (formatting, no logic change) |
| `refactor` | Code refactoring (no feature/fix) |
| `test` | Adding or updating tests |
| `build` | Build system or dependencies |
| `ci` | CI/CD changes |
| `chore` | Maintenance tasks |

### Examples

```
feat(vm): add VM cloning functionality
fix(ct): handle timeout when creating large containers
docs: update installation instructions for Fedora
build: add Flatpak build target to Makefile
```

---

## Pull Request Process

1. **Ensure your code compiles** without warnings (`make clean && make`)
2. **Test manually** — create/start/stop VMs and containers if your changes affect those paths
3. **Update documentation** if you change user-facing behavior
4. **Update CHANGELOG.md** under the `[Unreleased]` section
5. **Fill out the PR template** completely
6. **Request review** from a maintainer

### PR Checklist

- [ ] Code compiles without warnings
- [ ] Changes tested manually
- [ ] No security vulnerabilities introduced
- [ ] Documentation updated (if applicable)
- [ ] CHANGELOG.md updated
- [ ] Commit messages follow convention

### Review Process

- At least one maintainer approval is required
- CI must pass (compilation check)
- Feedback will be given within 1 week
- Address review comments or explain why you disagree

---

## Reporting Bugs

Use the [Bug Report template](https://github.com/TripleETech/vmmanager/issues/new?template=bug_report.yml) and include:

1. **Description** of the bug
2. **Steps to reproduce**
3. **Expected behavior**
4. **Actual behavior**
5. **Environment** (distro, GTK version, libvirt version)
6. **Logs** from stderr output

---

## Requesting Features

Use the [Feature Request template](https://github.com/TripleETech/vmmanager/issues/new?template=feature_request.yml) and include:

1. **Problem** you're trying to solve
2. **Proposed solution**
3. **Alternatives** you've considered
4. **Priority** and use case

---

## Security Vulnerabilities

**Do not open a public issue** for security vulnerabilities. Instead, follow the process in [SECURITY.md](SECURITY.md).

---

## Areas Where Help is Needed

We especially welcome contributions in these areas:

- **Flatpak packaging** — making VMManager available on Flathub
- **Testing** — automated test suite with a proper C testing framework
- **Accessibility** — ensuring the UI works well with screen readers
- **Internationalization** — adding i18n/l10n support
- **Documentation** — tutorials, screenshots, wiki pages
- **New distro support** — testing and packaging for more Linux distributions

---

## Questions?

Open a [Discussion](https://github.com/TripleETech/vmmanager/discussions) or reach out to the maintainers. We're happy to help!

Thank you for contributing!
