# VMManager

A native GTK4 desktop application for managing QEMU/KVM virtual machines and Incus/LXD containers from a single unified interface. Think Proxmox — but as a compiled C binary on your desktop. No browser, no web server, no runtime dependencies.

**Author:** Dharun
**License:** GPL-3.0

---

## What This Does

- **Dashboard** — Real-time CPU, RAM, disk monitoring + VM/container counts
- **Virtual Machines** — Create, start, shutdown, pause, resume, reboot, force stop, delete QEMU/KVM VMs via libvirt
- **Containers** — Create, start, stop, restart, force stop, delete Incus/LXD containers via REST API
- **Activity Log** — Live log of all operations with timestamps
- **Auto-refresh** — Dashboard updates every 3 seconds

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
│  └──────┬───────┘  └──────┬────────┘  │
│         │                  │           │
└─────────┼──────────────────┼───────────┘
          │                  │
    ┌─────┴─────┐     ┌─────┴─────┐
    │ QEMU/KVM  │     │ Incus/LXD │
    │ (VMs)     │     │(containers)│
    └───────────┘     └───────────┘
```

---

## Dependencies

### Install on Ubuntu 24.04:

```bash
# Build tools
sudo apt install gcc make pkg-config

# GTK 4 development
sudo apt install libgtk-4-dev

# libvirt development
sudo apt install libvirt-dev

# libcurl development
sudo apt install libcurl4-openssl-dev

# JSON-GLib development
sudo apt install libjson-glib-dev

# Runtime: QEMU/KVM + libvirt
sudo apt install qemu-kvm libvirt-daemon-system libvirt-clients virt-manager

# Runtime: Incus (optional, for container management)
# Follow: https://linuxcontainers.org/incus/docs/main/installing/
```

One-liner for all build deps:
```bash
sudo apt install gcc make pkg-config libgtk-4-dev libvirt-dev libcurl4-openssl-dev libjson-glib-dev
```

---

## Project Structure

```
vmmanager/
├── include/
│   └── vmmanager.h          # All type definitions and function declarations
├── src/
│   ├── main.c               # Entry point, logging, app lifecycle
│   ├── ui/
│   │   └── window.c         # GTK4 UI — sidebar, dashboard, VM/CT panels, dialogs
│   ├── backend/
│   │   ├── vm_manager.c     # libvirt C API — all VM operations
│   │   └── ct_manager.c     # Incus REST API via libcurl — all container operations
│   └── utils/
│       └── system_info.c    # CPU, RAM, disk monitoring from /proc
├── Makefile
└── README.md
```

---

## Build

```bash
cd vmmanager
make
```

That's it. Produces a single `vmmanager` binary.

### Other make targets:

```bash
make debug     # Build with debug symbols (-g -O0)
make clean     # Remove binary
make run       # Build and run
make install   # Install to /usr/local/bin
```

---

## Run

```bash
./vmmanager
```

### Important: Permissions

For **VM management** (libvirt), your user needs to be in the `libvirt` group:
```bash
sudo usermod -aG libvirt $USER
# Log out and back in for it to take effect
```

For **container management** (Incus), your user needs access to the Incus socket:
```bash
sudo usermod -aG incus $USER
# Log out and back in
```

If libvirt system connection fails, the app automatically falls back to session connection (`qemu:///session`).

---

## How It Works

### VM Management
Uses the **libvirt C API** directly. When you click "Create VM", the app:
1. Runs `qemu-img create` to make a qcow2 disk
2. Generates XML domain definition
3. Calls `virDomainDefineXML()` to register the VM
4. All operations (start/stop/pause/delete) go through libvirt → QEMU/KVM

### Container Management
Uses the **Incus/LXD REST API** via `libcurl` over a Unix socket. The app:
1. Checks for socket at `/var/lib/incus/unix.socket` (Incus) or `/var/snap/lxd/common/lxd/unix.socket` (LXD)
2. Makes HTTP requests to the local API (same API that `incus` CLI uses)
3. Parses JSON responses with json-glib

### System Monitoring
Reads directly from:
- `/proc/stat` — CPU usage
- `/proc/meminfo` — RAM usage
- `statvfs("/")` — Disk usage

---

## Troubleshooting

**"Failed to connect to libvirt"**
```bash
sudo systemctl start libvirtd
sudo systemctl enable libvirtd
sudo usermod -aG libvirt $USER
```

**"No Incus/LXD socket found"**
```bash
# Check if incus is running
sudo systemctl status incus
# Or for LXD
sudo systemctl status snap.lxd.daemon
```

**Build fails with missing headers**
```bash
# Make sure all dev packages are installed
pkg-config --exists gtk4 libvirt libcurl json-glib-1.0 && echo "All good" || echo "Missing deps"
```

**VMs created but won't start**
- Check if KVM is available: `egrep -c '(vmx|svm)' /proc/cpuinfo` (should be > 0)
- Check if default network is active: `virsh net-list --all` then `virsh net-start default`

---

## Future Ideas

- noVNC console access for VMs
- Snapshot management UI
- Resource graphs over time
- Network configuration panel
- Storage pool management
- Dark/light theme toggle
- .desktop file for GNOME app launcher
