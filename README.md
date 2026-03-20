<p align="center">
  <h1 align="center">VMManager</h1>
  <p align="center">
    A native GTK4 desktop application for managing QEMU/KVM virtual machines and Incus/LXD containers from a single unified interface.
    <br />
    Think <strong>Proxmox</strong> — but as a compiled C binary on your desktop. No browser, no web server, no runtime dependencies.
  </p>
</p>

<p align="center">
  <a href="#installation">Installation</a> •
  <a href="#features">Features</a> •
  <a href="#usage">Usage</a> •
  <a href="#architecture">Architecture</a> •
  <a href="#contributing">Contributing</a> •
  <a href="#license">License</a>
</p>

<p align="center">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0-blue.svg">
  <img alt="Version" src="https://img.shields.io/badge/version-2.0.0-green.svg">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Linux-lightgrey.svg">
  <img alt="GTK" src="https://img.shields.io/badge/GTK-4-orange.svg">
  <img alt="Language" src="https://img.shields.io/badge/language-C-blue.svg">
  <img alt="PRs Welcome" src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg">
</p>

---

## Features

| Feature | Description |
|---------|-------------|
| **Dashboard** | Real-time CPU, RAM, disk monitoring with VM/container counts |
| **VM Management** | Create, start, shutdown, pause, resume, reboot, force stop, delete QEMU/KVM VMs via libvirt |
| **Container Management** | Create, start, stop, restart, force stop, delete Incus/LXD containers via REST API |
| **Snapshots** | Create, revert, and delete VM snapshots with full UI |
| **Activity Log** | Live log of all operations with timestamps, export and clear |
| **Search & Filter** | Filter VMs/containers by state or search by name |
| **Console Access** | Launch VM viewer or container shell directly from the app |
| **Settings** | Persistent user configuration stored as JSON |
| **Auto-refresh** | Configurable dashboard auto-update (default: 3 seconds) |
| **Keyboard Shortcuts** | F5 refresh, Ctrl+, settings, Ctrl+Shift+N new VM, Ctrl+N new container |

## Screenshots

> Screenshots coming soon. Contributions welcome!
>
> Place screenshots in `assets/screenshots/` and open a PR.

---

## Installation

### Prerequisites

- **OS:** Linux (tested on Ubuntu 24.04, Debian 12, Fedora 40+)
- **Hypervisor:** QEMU/KVM with libvirt
- **Containers (optional):** Incus or LXD

### Build Dependencies

#### Ubuntu / Debian

```bash
# All build dependencies in one command
sudo apt install gcc make pkg-config libgtk-4-dev libvirt-dev libcurl4-openssl-dev libjson-glib-dev

# Runtime: QEMU/KVM + libvirt
sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients

# Optional: VM console viewer
sudo apt install virt-viewer

# Optional: Container runtime
# Follow: https://linuxcontainers.org/incus/docs/main/installing/
```

#### Fedora

```bash
sudo dnf install gcc make pkgconf-pkg-config gtk4-devel libvirt-devel libcurl-devel json-glib-devel
sudo dnf install qemu-kvm libvirt virt-manager
```

#### Arch Linux

```bash
sudo pacman -S gcc make pkgconf gtk4 libvirt curl json-glib
sudo pacman -S qemu-full libvirt virt-manager
```

### Build from Source

```bash
git clone https://github.com/TripleETech/vmmanager.git
cd vmmanager
make
```

That's it. Produces a single `vmmanager` binary (~127 KB).

### Other Build Targets

```bash
make debug       # Build with debug symbols (-g -O0)
make clean       # Remove build artifacts
make run         # Build and run
make install     # Install to /usr/local/bin
make uninstall   # Remove from /usr/local/bin
make test        # Build test programs
make check-deps  # Verify all dependencies are installed
make desktop     # Install .desktop file for app launcher
make dist        # Create source tarball
```

### Quick Install Script

```bash
git clone https://github.com/TripleETech/vmmanager.git
cd vmmanager
sudo ./scripts/install.sh
```

---

## Usage

### Running

```bash
./vmmanager
```

### Permissions Setup

For **VM management** (libvirt):
```bash
sudo usermod -aG libvirt $USER
# Log out and back in for it to take effect
```

For **container management** (Incus):
```bash
sudo usermod -aG incus $USER
# Log out and back in
```

> If the libvirt system connection fails, VMManager automatically falls back to the session connection (`qemu:///session`).

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `F5` | Refresh all data |
| `Ctrl+,` | Open settings |
| `Ctrl+Shift+N` | Create new VM |
| `Ctrl+N` | Create new container |

---

## Architecture

```
┌─────────────────────────────────────────┐
│         vmmanager (single binary)       │
│                                         │
│  ┌──────────────┐  ┌────────────────┐  │
│  │  GTK 4 GUI   │  │  GTK 4 GUI     │  │
│  │  VM Panel    │  │  Container     │  │
│  │              │  │  Panel         │  │
│  └──────┬───────┘  └───────┬────────┘  │
│         │                  │           │
│  ┌──────┴───────┐  ┌──────┴────────┐  │
│  │ libvirt API  │  │ Incus REST    │  │
│  │ (C library)  │  │ (libcurl via  │  │
│  │              │  │  Unix socket) │  │
│  └──────┬───────┘  └──────┴────────┘  │
│         │                  │           │
└─────────┼──────────────────┼───────────┘
          │                  │
    ┌─────┴─────┐     ┌─────┴─────┐
    │ QEMU/KVM  │     │ Incus/LXD │
    │ (VMs)     │     │(containers)│
    └───────────┘     └───────────┘
```

### Project Structure

```
vmmanager/
├── .github/
│   ├── ISSUE_TEMPLATE/          # Bug report & feature request templates
│   ├── workflows/               # CI/CD pipelines
│   └── PULL_REQUEST_TEMPLATE.md
├── assets/
│   └── screenshots/             # Application screenshots
├── docs/                        # Additional documentation
├── include/
│   └── vmmanager.h              # All type definitions & function declarations
├── src/
│   ├── main.c                   # Entry point, logging, app lifecycle
│   ├── ui/
│   │   ├── window.c             # GTK4 UI — sidebar, dashboard, panels
│   │   └── dialogs.c            # Dialogs, settings, shortcuts, console
│   ├── backend/
│   │   ├── vm_manager.c         # libvirt C API — all VM operations
│   │   ├── ct_manager.c         # Incus REST API — all container operations
│   │   └── snapshot_manager.c   # VM snapshot create/revert/delete
│   └── utils/
│       ├── error_handling.c     # Structured error management
│       └── system_info.c        # CPU, RAM, disk monitoring from /proc
├── scripts/
│   └── install.sh               # Quick installation script
├── tests/                       # Test programs and scripts
├── Makefile                     # Build system
├── CONTRIBUTING.md              # Contribution guidelines
├── CODE_OF_CONDUCT.md           # Community standards
├── SECURITY.md                  # Security policy
├── CHANGELOG.md                 # Release history
├── LICENSE                      # GPL-3.0 license text
└── README.md                    # This file
```

### How It Works

**VM Management** uses the **libvirt C API** directly:
1. Runs `qemu-img create` to make a qcow2 disk (via `fork`/`exec`, not `system()`)
2. Generates XML domain definition
3. Calls `virDomainDefineXML()` to register the VM
4. All operations (start/stop/pause/delete) go through libvirt → QEMU/KVM

**Container Management** uses the **Incus/LXD REST API** via `libcurl` over a Unix socket:
1. Auto-detects socket at multiple paths (Incus, LXD, alternatives)
2. Makes HTTP requests to the local API (same API that `incus` CLI uses)
3. Parses JSON responses with json-glib
4. Supports async operation waiting with timeout

**System Monitoring** reads directly from:
- `/proc/stat` — CPU usage (delta-based calculation)
- `/proc/meminfo` — RAM usage (accounts for buffers/cache)
- `statvfs("/")` — Disk usage

---

## Troubleshooting

<details>
<summary><strong>"Failed to connect to libvirt"</strong></summary>

```bash
sudo systemctl start libvirtd
sudo systemctl enable libvirtd
sudo usermod -aG libvirt $USER
# Log out and back in
```
</details>

<details>
<summary><strong>"No Incus/LXD socket found"</strong></summary>

```bash
# Check if Incus is running
sudo systemctl status incus

# Or for LXD
sudo systemctl status snap.lxd.daemon

# Ensure user is in the right group
sudo usermod -aG incus $USER
```
</details>

<details>
<summary><strong>Build fails with missing headers</strong></summary>

```bash
# Verify all dependencies
make check-deps

# Or manually check
pkg-config --exists gtk4 libvirt libcurl json-glib-1.0 && echo "All good" || echo "Missing deps"
```
</details>

<details>
<summary><strong>VMs created but won't start</strong></summary>

- Check if KVM is available: `egrep -c '(vmx|svm)' /proc/cpuinfo` (should be > 0)
- Check default network: `virsh net-list --all` then `virsh net-start default`
- Check permissions: `ls -la /dev/kvm`
</details>

<details>
<summary><strong>Application crashes on startup</strong></summary>

```bash
# Run in debug mode for more info
make debug
./vmmanager 2>debug.log

# Check the log
cat debug.log
```
</details>

---

## Roadmap

- [ ] noVNC console access for VMs
- [ ] Resource graphs over time (CPU/RAM history)
- [ ] Network configuration panel
- [ ] Storage pool management
- [ ] Dark/light theme toggle
- [ ] VM migration between hosts
- [ ] Container resource limits (CPU/RAM caps)
- [ ] Backup/restore functionality
- [ ] Plugin system for extensibility
- [ ] Flatpak packaging

See the [open issues](https://github.com/TripleETech/vmmanager/issues) for a full list of proposed features and known bugs.

---

## Contributing

Contributions make the open-source community an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, development setup, and the process for submitting pull requests.

### Quick Start for Contributors

```bash
# Fork and clone
git clone https://github.com/YOUR_USERNAME/vmmanager.git
cd vmmanager

# Install deps and build
sudo apt install gcc make pkg-config libgtk-4-dev libvirt-dev libcurl4-openssl-dev libjson-glib-dev
make debug

# Create a branch
git checkout -b feature/my-feature

# Make changes, test, commit
make
./vmmanager
git commit -am "Add my feature"
git push origin feature/my-feature
```

Then open a Pull Request.

---

## Security

If you discover a security vulnerability, please follow the responsible disclosure process outlined in [SECURITY.md](SECURITY.md). **Do not open a public issue for security vulnerabilities.**

---

## License

Distributed under the **GNU General Public License v3.0**. See [LICENSE](LICENSE) for the full text.

```
VMManager - Native Desktop VM & Container Manager
Copyright (C) 2024-2026 Dharun (TripleETech)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

---

## Acknowledgments

- [GTK](https://gtk.org/) — The GUI toolkit
- [libvirt](https://libvirt.org/) — Virtualization API
- [QEMU/KVM](https://www.qemu.org/) — Machine emulator and virtualizer
- [Incus](https://linuxcontainers.org/incus/) / [LXD](https://canonical.com/lxd) — Container manager
- [libcurl](https://curl.se/libcurl/) — HTTP client library
- [json-glib](https://gnome.pages.gitlab.gnome.org/json-glib/) — JSON parser for GLib

---

<p align="center">
  Made with C and determination by <a href="https://github.com/TripleETech">Dharun</a>
</p>
