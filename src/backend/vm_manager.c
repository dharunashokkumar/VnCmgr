/*
 * vm_manager.c - libvirt backend for QEMU/KVM virtual machine management
 */

#include "../include/vmmanager.h"
#include <sys/wait.h>

/* ── Validate VM name (alphanumeric, dash, underscore only) ─── */
static bool is_valid_vm_name(const char *name) {
    if (!name || strlen(name) == 0 || strlen(name) > 255) return false;
    for (const char *p = name; *p; p++) {
        if (!g_ascii_isalnum(*p) && *p != '-' && *p != '_') return false;
    }
    return true;
}

/* ── Safely create disk image using fork/exec ───────────────── */
static bool create_disk_image(const char *path, int size_gb) {
    pid_t pid = fork();
    if (pid < 0) return false;
    
    if (pid == 0) {
        /* Child process */
        char size_str[32];
        snprintf(size_str, sizeof(size_str), "%dG", size_gb);
        execlp("qemu-img", "qemu-img", "create", "-f", "qcow2", path, size_str, NULL);
        _exit(127);  /* exec failed */
    }
    
    /* Parent process - wait for child */
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

/* ── Convert libvirt state to our enum ─────────────────────── */
static VmState libvirt_state_to_vm_state(int state) {
    switch (state) {
        case VIR_DOMAIN_RUNNING:  return VM_STATE_RUNNING;
        case VIR_DOMAIN_PAUSED:   return VM_STATE_PAUSED;
        case VIR_DOMAIN_SHUTOFF:  return VM_STATE_SHUTOFF;
        case VIR_DOMAIN_CRASHED:  return VM_STATE_CRASHED;
        default:                  return VM_STATE_UNKNOWN;
    }
}

const char *vm_state_str(VmState s) {
    switch (s) {
        case VM_STATE_RUNNING: return "Running";
        case VM_STATE_PAUSED:  return "Paused";
        case VM_STATE_SHUTOFF: return "Stopped";
        case VM_STATE_CRASHED: return "Crashed";
        default:               return "Unknown";
    }
}

const char *vm_state_css(VmState s) {
    switch (s) {
        case VM_STATE_RUNNING: return "state-running";
        case VM_STATE_PAUSED:  return "state-paused";
        case VM_STATE_SHUTOFF: return "state-stopped";
        case VM_STATE_CRASHED: return "state-crashed";
        default:               return "state-unknown";
    }
}

/* ── Connect to libvirt ────────────────────────────────────── */
bool vm_backend_connect(AppData *app) {
    app->virt_conn = virConnectOpen("qemu:///system");
    if (!app->virt_conn) {
        app_log(app, "ERROR", "Failed to connect to libvirt (qemu:///system)");
        /* Try session connection as fallback */
        app->virt_conn = virConnectOpen("qemu:///session");
        if (!app->virt_conn) {
            app_log(app, "ERROR", "Failed to connect to libvirt (qemu:///session)");
            return false;
        }
        app_log(app, "INFO", "Connected to libvirt (session)");
    } else {
        app_log(app, "INFO", "Connected to libvirt (system)");
    }
    return true;
}

void vm_backend_disconnect(AppData *app) {
    if (app->virt_conn) {
        virConnectClose(app->virt_conn);
        app->virt_conn = NULL;
    }
}

/* ── Refresh VM list ───────────────────────────────────────── */
bool vm_backend_refresh(AppData *app) {
    if (!app->virt_conn) return false;

    /* Free old list */
    if (app->vms) {
        free(app->vms);
        app->vms = NULL;
    }
    app->vm_count = 0;

    virDomainPtr *domains = NULL;
    int flags = VIR_CONNECT_LIST_DOMAINS_ACTIVE | VIR_CONNECT_LIST_DOMAINS_INACTIVE;
    int count = virConnectListAllDomains(app->virt_conn, &domains, flags);

    if (count < 0) {
        app_log(app, "ERROR", "Failed to list VMs");
        return false;
    }

    if (count == 0) return true;

    app->vms = calloc(count, sizeof(VmInfo));
    if (!app->vms) return false;
    app->vm_count = count;

    app->stats.total_vms = count;
    app->stats.running_vms = 0;

    for (int i = 0; i < count; i++) {
        virDomainInfo info;
        const char *name = virDomainGetName(domains[i]);
        char uuid_str[VIR_UUID_STRING_BUFLEN];

        if (name) {
            strncpy(app->vms[i].name, name, 255);
            app->vms[i].name[255] = '\0';  /* Ensure NULL termination */
        }

        if (virDomainGetUUIDString(domains[i], uuid_str) == 0) {
            strncpy(app->vms[i].uuid, uuid_str, 63);
            app->vms[i].uuid[63] = '\0';  /* Ensure NULL termination */
        }

        if (virDomainGetInfo(domains[i], &info) == 0) {
            app->vms[i].state = libvirt_state_to_vm_state(info.state);
            app->vms[i].vcpus = info.nrVirtCpu;
            app->vms[i].max_mem_kb = info.maxMem;
            app->vms[i].used_mem_kb = info.memory;

            if (info.state == VIR_DOMAIN_RUNNING)
                app->stats.running_vms++;
        }

        int autostart = 0;
        if (virDomainGetAutostart(domains[i], &autostart) < 0) {
            autostart = 0;  /* Default to disabled on error */
        }
        app->vms[i].autostart = autostart;

        virDomainFree(domains[i]);
    }

    free(domains);
    return true;
}

/* ── VM Operations ─────────────────────────────────────────── */
static virDomainPtr find_domain(AppData *app, const char *name) {
    if (!app->virt_conn) return NULL;
    return virDomainLookupByName(app->virt_conn, name);
}

bool vm_start(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) { app_log(app, "ERROR", "VM '%s' not found", name); return false; }

    int ret = virDomainCreate(dom);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to start VM '%s'", name);
        return false;
    }
    app_log(app, "INFO", "VM '%s' started", name);
    return true;
}

bool vm_shutdown(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) return false;

    int ret = virDomainShutdown(dom);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to shutdown VM '%s'", name);
        return false;
    }
    app_log(app, "INFO", "VM '%s' shutdown signal sent", name);
    return true;
}

bool vm_force_stop(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) return false;

    int ret = virDomainDestroy(dom);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to force stop VM '%s'", name);
        return false;
    }
    app_log(app, "INFO", "VM '%s' force stopped", name);
    return true;
}

bool vm_reboot(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) return false;

    int ret = virDomainReboot(dom, 0);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to reboot VM '%s'", name);
        return false;
    }
    app_log(app, "INFO", "VM '%s' reboot signal sent", name);
    return true;
}

bool vm_pause(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) return false;

    int ret = virDomainSuspend(dom);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to pause VM '%s'", name);
        return false;
    }
    app_log(app, "INFO", "VM '%s' paused", name);
    return true;
}

bool vm_resume(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) return false;

    int ret = virDomainResume(dom);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to resume VM '%s'", name);
        return false;
    }
    app_log(app, "INFO", "VM '%s' resumed", name);
    return true;
}

bool vm_delete(AppData *app, const char *name) {
    virDomainPtr dom = find_domain(app, name);
    if (!dom) return false;

    /* Stop if running */
    virDomainInfo info;
    if (virDomainGetInfo(dom, &info) == 0 && info.state == VIR_DOMAIN_RUNNING) {
        virDomainDestroy(dom);
        /* Re-lookup after destroy */
        dom = find_domain(app, name);
        if (!dom) return false;
    }

    int ret = virDomainUndefine(dom);
    virDomainFree(dom);

    if (ret < 0) {
        app_log(app, "ERROR", "Failed to delete VM '%s'", name);
        return false;
    }
    app_log(app, "INFO", "VM '%s' deleted", name);
    return true;
}

/* ── Create VM with XML ────────────────────────────────────── */
bool vm_create(AppData *app, const char *name, int vcpus, int ram_mb,
               int disk_gb, const char *iso_path) {
    if (!app->virt_conn) return false;
    
    /* Validate VM name to prevent injection */
    if (!is_valid_vm_name(name)) {
        app_log(app, "ERROR", "Invalid VM name: must be alphanumeric, dash, or underscore");
        return false;
    }

    /* Use GString for dynamic XML building */
    GString *xml = g_string_new(NULL);
    char disk_path[512];
    snprintf(disk_path, sizeof(disk_path), "/var/lib/libvirt/images/%s.qcow2", name);

    /* Create disk image safely using fork/exec */
    if (!create_disk_image(disk_path, disk_gb)) {
        app_log(app, "WARN", "Could not create disk at %s, trying home dir", disk_path);
        const char *home = getenv("HOME");
        if (!home) home = "/tmp";
        snprintf(disk_path, sizeof(disk_path), "%s/.local/share/libvirt/images/%s.qcow2", home, name);
        
        /* Create directory first */
        char *dir_end = strrchr(disk_path, '/');
        if (dir_end) {
            size_t dir_len = dir_end - disk_path;
            char dir_path[512];
            strncpy(dir_path, disk_path, dir_len);
            dir_path[dir_len] = '\0';
            g_mkdir_with_parents(dir_path, 0755);
        }
        
        if (!create_disk_image(disk_path, disk_gb)) {
            app_log(app, "ERROR", "Failed to create disk image for VM '%s'", name);
            g_string_free(xml, TRUE);
            return false;
        }
    }

    /* Build XML definition safely using GString */
    g_string_append(xml, "<domain type='kvm'>\n");
    g_string_append_printf(xml, "  <name>%s</name>\n", name);
    g_string_append_printf(xml, "  <memory unit='MiB'>%d</memory>\n", ram_mb);
    g_string_append_printf(xml, "  <vcpu>%d</vcpu>\n", vcpus);
    g_string_append(xml, "  <os>\n");
    g_string_append(xml, "    <type arch='x86_64'>hvm</type>\n");
    g_string_append(xml, "    <boot dev='cdrom'/>\n");
    g_string_append(xml, "    <boot dev='hd'/>\n");
    g_string_append(xml, "  </os>\n");
    g_string_append(xml, "  <features><acpi/><apic/></features>\n");
    g_string_append(xml, "  <devices>\n");
    g_string_append(xml, "    <disk type='file' device='disk'>\n");
    g_string_append(xml, "      <driver name='qemu' type='qcow2'/>\n");
    g_string_append_printf(xml, "      <source file='%s'/>\n", disk_path);
    g_string_append(xml, "      <target dev='vda' bus='virtio'/>\n");
    g_string_append(xml, "    </disk>\n");
    
    if (iso_path && strlen(iso_path) > 0) {
        g_string_append(xml, "    <disk type='file' device='cdrom'>\n");
        g_string_append(xml, "      <driver name='qemu' type='raw'/>\n");
        g_string_append_printf(xml, "      <source file='%s'/>\n", iso_path);
        g_string_append(xml, "      <target dev='sda' bus='sata'/>\n");
        g_string_append(xml, "      <readonly/>\n");
        g_string_append(xml, "    </disk>\n");
    }
    
    g_string_append(xml, "    <interface type='network'>\n");
    g_string_append(xml, "      <source network='default'/>\n");
    g_string_append(xml, "      <model type='virtio'/>\n");
    g_string_append(xml, "    </interface>\n");
    g_string_append(xml, "    <graphics type='spice' autoport='yes'/>\n");
    g_string_append(xml, "    <video><model type='virtio'/></video>\n");
    g_string_append(xml, "    <console type='pty'/>\n");
    g_string_append(xml, "  </devices>\n");
    g_string_append(xml, "</domain>\n");

    virDomainPtr dom = virDomainDefineXML(app->virt_conn, xml->str);
    g_string_free(xml, TRUE);
    
    if (!dom) {
        app_log(app, "ERROR", "Failed to define VM '%s'", name);
        return false;
    }

    virDomainFree(dom);
    app_log(app, "INFO", "VM '%s' created (%d vCPUs, %d MB RAM, %d GB disk)",
            name, vcpus, ram_mb, disk_gb);
    return true;
}
