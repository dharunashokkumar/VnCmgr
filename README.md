<div align="center">

# VMManager

**A native GTK4 desktop application for managing QEMU/KVM virtual machines and Incus/LXD containers.**

Think Proxmox — but as a compiled C binary on your desktop. No browser, no web server, no runtime dependencies.

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-2.0.0-green.svg)](https://github.com/TripleETech/vmmanager/releases)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)]()
[![GTK](https://img.shields.io/badge/GTK-4.x-orange.svg)]()
[![C](https://img.shields.io/badge/language-C-blue.svg)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

[Features](#features) · [Installation](#installation) · [Usage](#usage) · [Contributing](#contributing) · [Roadmap](#roadmap)

</div>

---

## Features

- **Dashboard** — Real-time CPU, RAM, disk monitoring + VM/container counts
- **Virtual Machines** — Create, start, shutdown, pause, resume, reboot, force stop, delete QEMU/KVM VMs via libvirt
- **Containers** — Create, start, stop, restart, force stop, delete Incus/LXD containers via REST API
- **Snapshots** — Create, revert, and delete VM snapshots
- **Activity Log** — Live log of all operations with timestamps, export and clear
- **Search & Filter** — Filter VMs/containers by state or search by name
- **Console Access** — Launch VM viewer or container shell directly from the app
- **Settings** — Persistent user settings stored at `~/.config/vmmanager/settings.json`
- **Keyboard Shortcuts** — F5 refresh, Ctrl+, settings, Ctrl+Shift+N new VM, Ctrl+N new container
- **Auto-refresh** — Dashboard updates every 3 seconds (configurable)

## Screenshots

<!-- Add screenshots here -->
<!-- ![Dashboard](docs/screenshots/dashboard.png) -->

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
│  └──────┬───────┘  └──────┬────────┘  │
│         │                  │           │
└─────────┼──────────────────┼───────────┘
          │                  │
    ┌─────┴─────┐     ┌─────┴─────┐
    │ QEMU/KVM  │     │ Incus/LXD │
    │ (VMs)     │     │(containers)│
    └───────────┘     └───────────┘
```

## Installation

### Prerequisites

| Dependency | Package | Purpose |
|-----------|---------|---------|
| GCC | `gcc` | C compiler |
| Make | `make` | Build system |
| pkg-config | `pkg-config` | Dependency resolution |
| GTK 4 | `libgtk-4-dev` | GUI framework |
| libvirt | `libvirt-dev` | VM management API |
| libcurl | `libcurl4-openssl-dev` | HTTP client for container API |
| JSON-GLib | `libjson-glib-dev` | JSON parsing |

### Quick Install (Ubuntu/Debian)

```bash
# Install all build dependencies
sudo apt install gcc make pkg-config libgtk-4-dev libvirt-dev libcurl4-openssl-dev libjson-glib-dev

# Install runtime dependencies
sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients

# Optional: Incus for container management
# See https://linuxcontainers.org/incus/docs/main/installing/
```

### Fedora/RHEL

```bash
sudo dnf install gcc make pkgconf-pkg-config gtk4-devel libvirt-devel libcurl-devel json-glib-devel
sudo dnf install qemu-kvm libvirt libvirt-client
```

### Arch Linux

```bash
sudo pacman -S gcc make pkg-config gtk4 libvirt curl json-glib
sudo pacman -S qemu-full libvirt
```

### Build from Source

```bash
git clone https://github.com/TripleETech/vmmanager.git
cd vmmanager
make
```

That's it. Produces a single `vmmanager` binary.

### Other Make Targets

| Target | Description |
|--------|-------------|
| `make` | Build release binary |
| `make debug` | Build with debug symbols (`-g -O0`) |
| `make clean` | Remove build artifacts |
| `make run` | Build and run |
| `make install` | Install to `/usr/local/bin` |
| `make uninstall` | Remove from `/usr/local/bin` |
| `make test` | Build test programs |

### System Install

```bash
sudo make install      # installs to /usr/local/bin
sudo make uninstall    # removes it
```

## Usage

```bash
./vmmanager
```

### Permissions Setup

For **VM management**, your user needs to be in the `libvirt` group:
```bash
sudo usermod -aG libvirt $USER
# Log out and back in for it to take effect
```

For **container management**, your user needs access to the Incus socket:
```bash
sudo usermod -aG incus $USER
# Log out and back in
```

### How It Works

**VM Management** — Uses the libvirt C API directly. When you create a VM, the app generates XML domain definitions, creates qcow2 disk images via `qemu-img`, and registers domains through `virDomainDefineXML()`. All lifecycle operations go through libvirt to QEMU/KVM.

**Container Management** — Uses the Incus/LXD REST API via `libcurl` over a Unix socket. The app checks multiple socket paths for Incus/LXD compatibility and communicates using the same API that CLI tools use.

**System Monitoring** — Reads directly from `/proc/stat` (CPU), `/proc/meminfo` (RAM), and `statvfs("/")` (disk).

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `F5` | Refresh all data |
| `Ctrl + ,` | Open settings |
| `Ctrl + Shift + N` | Create new VM |
| `Ctrl + N` | Create new container |

## Project Structure

```
vmmanager/
├── .github/
│   ├── ISSUE_TEMPLATE/        # Bug report & feature request templates
│   └── workflows/             # CI/CD pipelines
├── assets/                    # Icons and images
├── docs/                      # Documentation
├── include/
│   └── vmmanager.h            # All type definitions and function declarations
├── src/
│   ├── main.c                 # Entry point, logging, app lifecycle
│   ├── ui/
│   │   ├── window.c           # GTK4 UI — sidebar, dashboard, panels
│   │   └── dialogs.c          # Dialogs, settings, loading indicators
│   ├── backend/
│   │   ├── vm_manager.c       # libvirt C API — VM operations
│   │   ├── ct_manager.c       # Incus REST API — container operations
│   │   └── snapshot_manager.c # VM snapshot management
│   └── utils/
│       ├── error_handling.c   # Structured error handling
│       └── system_info.c      # CPU, RAM, disk monitoring
├── tests/                     # Test programs and scripts
├── scripts/                   # Utility scripts
├── Makefile                   # Build system
├── LICENSE                    # GPL-3.0
├── CONTRIBUTING.md            # Contribution guidelines
├── CODE_OF_CONDUCT.md         # Community standards
├── CHANGELOG.md               # Release history
└── SECURITY.md                # Security policy
```

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

# Ensure your user has socket access
sudo usermod -aG incus $USER
```
</details>

<details>
<summary><strong>Build fails with missing headers</strong></summary>

```bash
# Verify all dependencies are installed
pkg-config --exists gtk4 libvirt libcurl json-glib-1.0 && echo "All good" || echo "Missing deps"
```
</details>

<details>
<summary><strong>VMs created but won't start</strong></summary>

- Check KVM support: `egrep -c '(vmx|svm)' /proc/cpuinfo` (should be > 0)
- Check default network: `virsh net-list --all` then `virsh net-start default`
</details>

<details>
<summary><strong>Container operations are slow</strong></summary>

Image pulls from `images.linuxcontainers.org` can take time on first use. Subsequent creates using cached images are faster.
</details>

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

Quick start:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Make your changes
4. Run `make clean && make` to verify the build
5. Commit your changes
6. Push to your fork and open a Pull Request

## Roadmap

- [ ] noVNC console access for VMs
- [ ] Resource usage graphs over time
- [ ] Network configuration panel
- [ ] Storage pool management
- [ ] Dark/light theme toggle
- [ ] `.desktop` file + AppStream metadata for Linux app stores
- [ ] Flatpak packaging
- [ ] VM migration support
- [ ] Container resource limits (CPU, memory)
- [ ] Bulk operations (start/stop multiple VMs)

## Security

Found a vulnerability? Please see [SECURITY.md](SECURITY.md) for responsible disclosure guidelines.

## License

This project is licensed under the GNU General Public License v3.0 — see the [LICENSE](LICENSE) file for details.

## Author

**Dharun** — [TripleETech](https://github.com/TripleETech)

---

<div align="center">

If you find VMManager useful, please consider giving it a star on GitHub!

</div>
