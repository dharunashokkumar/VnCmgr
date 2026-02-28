/*
 * VMManager - Native Desktop VM & Container Manager
 * A Proxmox-like GTK4 application for managing QEMU/KVM VMs and Incus containers
 *
 * Author: Dharun (TripleETech)
 * License: GPL-3.0
 */

#ifndef VMMANAGER_H
#define VMMANAGER_H

#include <gtk/gtk.h>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <json-glib/json-glib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

/* ── Application Constants ─────────────────────────────────── */
#define APP_ID           "com.tripletech.vmmanager"
#define APP_NAME         "VMManager"
#define APP_VERSION      "1.0.0"
#define INCUS_SOCKET     "/var/lib/incus/unix.socket"
#define LXD_SOCKET       "/var/snap/lxd/common/lxd/unix.socket"
#define REFRESH_INTERVAL 3000

/* ── VM States ─────────────────────────────────────────────── */
typedef enum {
    VM_STATE_RUNNING,
    VM_STATE_PAUSED,
    VM_STATE_SHUTOFF,
    VM_STATE_CRASHED,
    VM_STATE_UNKNOWN
} VmState;

/* ── Container States ──────────────────────────────────────── */
typedef enum {
    CT_STATE_RUNNING,
    CT_STATE_STOPPED,
    CT_STATE_FROZEN,
    CT_STATE_UNKNOWN
} CtState;

/* ── VM Information ────────────────────────────────────────── */
typedef struct {
    char     name[256];
    char     uuid[64];
    VmState  state;
    int      vcpus;
    unsigned long max_mem_kb;
    unsigned long used_mem_kb;
    int      autostart;
} VmInfo;

/* ── Container Information ─────────────────────────────────── */
typedef struct {
    char     name[256];
    char     type[32];
    CtState  state;
    char     ipv4[64];
    char     image[256];
    char     created[64];
} CtInfo;

/* ── System Stats ──────────────────────────────────────────── */
typedef struct {
    double   cpu_usage_percent;
    int64_t  total_ram_mb;
    int64_t  used_ram_mb;
    int64_t  total_disk_mb;
    int64_t  used_disk_mb;
    int      total_vms;
    int      running_vms;
    int      total_containers;
    int      running_containers;
} SystemStats;

/* ── HTTP Response Buffer ──────────────────────────────────── */
typedef struct {
    char   *data;
    size_t  size;
} HttpBuffer;

/* ── Main Application Structure ────────────────────────────── */
typedef struct {
    GtkApplication  *app;
    GtkWidget       *window;
    GtkWidget       *header_bar;
    GtkWidget       *main_stack;
    GtkWidget       *sidebar_list;

    /* Dashboard */
    GtkWidget       *dash_cpu_bar;
    GtkWidget       *dash_ram_bar;
    GtkWidget       *dash_disk_bar;
    GtkWidget       *dash_cpu_label;
    GtkWidget       *dash_ram_label;
    GtkWidget       *dash_disk_label;
    GtkWidget       *dash_vm_running_label;
    GtkWidget       *dash_vm_total_label;
    GtkWidget       *dash_ct_running_label;
    GtkWidget       *dash_ct_total_label;

    /* VM Panel */
    GtkWidget       *vm_list_box;
    GtkWidget       *vm_status_label;

    /* Container Panel */
    GtkWidget       *ct_list_box;
    GtkWidget       *ct_status_label;

    /* Log Panel */
    GtkTextBuffer   *log_buffer;
    GtkWidget       *log_text_view;

    /* Backend */
    virConnectPtr    virt_conn;
    bool             incus_available;
    char             incus_socket_path[512];

    /* State */
    VmInfo          *vms;
    int              vm_count;
    CtInfo          *containers;
    int              ct_count;
    SystemStats      stats;
    guint            refresh_timer;
} AppData;

/* ── VM Backend ────────────────────────────────────────────── */
bool        vm_backend_connect(AppData *app);
void        vm_backend_disconnect(AppData *app);
bool        vm_backend_refresh(AppData *app);
bool        vm_start(AppData *app, const char *name);
bool        vm_shutdown(AppData *app, const char *name);
bool        vm_force_stop(AppData *app, const char *name);
bool        vm_reboot(AppData *app, const char *name);
bool        vm_pause(AppData *app, const char *name);
bool        vm_resume(AppData *app, const char *name);
bool        vm_delete(AppData *app, const char *name);
bool        vm_create(AppData *app, const char *name, int vcpus, int ram_mb,
                      int disk_gb, const char *iso_path);
const char *vm_state_str(VmState s);
const char *vm_state_css(VmState s);

/* ── Container Backend ─────────────────────────────────────── */
bool        ct_backend_connect(AppData *app);
bool        ct_backend_refresh(AppData *app);
bool        ct_start(AppData *app, const char *name);
bool        ct_stop(AppData *app, const char *name);
bool        ct_force_stop(AppData *app, const char *name);
bool        ct_restart(AppData *app, const char *name);
bool        ct_delete(AppData *app, const char *name);
bool        ct_create(AppData *app, const char *name, const char *image);
char       *ct_api_request(AppData *app, const char *method,
                           const char *endpoint, const char *body);
const char *ct_state_str(CtState s);
const char *ct_state_css(CtState s);

/* ── System Utils ──────────────────────────────────────────── */
void        sys_get_stats(AppData *app);
double      sys_get_cpu_usage(void);

/* ── UI Builders ───────────────────────────────────────────── */
void        ui_build_window(AppData *app);
GtkWidget  *ui_build_sidebar(AppData *app);
GtkWidget  *ui_build_dashboard(AppData *app);
GtkWidget  *ui_build_vm_panel(AppData *app);
GtkWidget  *ui_build_ct_panel(AppData *app);
GtkWidget  *ui_build_log_panel(AppData *app);

/* ── UI Refresh ────────────────────────────────────────────── */
void        ui_refresh_all(AppData *app);
void        ui_refresh_dashboard(AppData *app);
void        ui_refresh_vm_list(AppData *app);
void        ui_refresh_ct_list(AppData *app);
gboolean    ui_auto_refresh(gpointer data);

/* ── UI Dialogs ────────────────────────────────────────────── */
void        ui_show_vm_create_dialog(AppData *app);
void        ui_show_ct_create_dialog(AppData *app);

/* ── Logging ───────────────────────────────────────────────── */
void        app_log(AppData *app, const char *level, const char *fmt, ...);

/* ── Error Handling ────────────────────────────────────────── */
typedef enum {
    ERR_NONE = 0,
    ERR_CONNECTION_FAILED,
    ERR_PERMISSION_DENIED,
    ERR_NOT_FOUND,
    ERR_ALREADY_EXISTS,
    ERR_INVALID_INPUT,
    ERR_OPERATION_FAILED,
    ERR_TIMEOUT,
    ERR_OUT_OF_MEMORY,
    ERR_SERVICE_UNAVAILABLE
} ErrorCode;

typedef struct {
    ErrorCode   code;
    char        message[512];
    char        suggestion[256];
    bool        recoverable;
} AppError;

void        error_set(AppError *err, ErrorCode code, const char *msg, const char *suggestion);
void        error_clear(AppError *err);
const char *error_code_str(ErrorCode code);

/* ── UI Dialogs ────────────────────────────────────────────── */
void        ui_show_error_dialog(AppData *app, const char *title, 
                                  const char *message, const char *suggestion);
void        ui_show_confirm_dialog(AppData *app, const char *title,
                                    const char *message, GCallback on_confirm,
                                    gpointer data);
void        ui_show_info_dialog(AppData *app, const char *title, const char *message);
void        ui_show_vm_details_dialog(AppData *app, VmInfo *vm);
void        ui_show_ct_details_dialog(AppData *app, CtInfo *ct);

/* ── Loading Indicators ────────────────────────────────────── */
typedef struct {
    GtkWidget *overlay;
    GtkWidget *spinner;
    GtkWidget *label;
    GtkWidget *parent;
    bool       active;
} LoadingIndicator;

LoadingIndicator *ui_loading_start(AppData *app, GtkWidget *parent, const char *message);
void              ui_loading_end(LoadingIndicator *loader);
void              ui_loading_update(LoadingIndicator *loader, const char *message);

/* ── Search/Filter ─────────────────────────────────────────── */
typedef enum {
    FILTER_ALL,
    FILTER_RUNNING,
    FILTER_STOPPED,
    FILTER_PAUSED
} FilterState;

void        ui_set_vm_filter(AppData *app, FilterState filter, const char *search);
void        ui_set_ct_filter(AppData *app, FilterState filter, const char *search);

/* ── Snapshot Management ───────────────────────────────────── */
typedef struct {
    char name[128];
    char created_at[64];
    char state[32];
} VmSnapshot;

bool        vm_snapshot_create(AppData *app, const char *vm_name, const char *snapshot_name);
bool        vm_snapshot_revert(AppData *app, const char *vm_name, const char *snapshot_name);
bool        vm_snapshot_delete(AppData *app, const char *vm_name, const char *snapshot_name);
VmSnapshot *vm_snapshot_list(AppData *app, const char *vm_name, int *count);
void        ui_show_snapshot_dialog(AppData *app, const char *vm_name);

/* ── Settings ──────────────────────────────────────────────── */
typedef struct {
    int      refresh_interval_ms;
    bool     confirm_destructive_actions;
    bool     auto_refresh;
    char     default_iso_path[512];
    char     default_image[128];
    int      default_vcpus;
    int      default_ram_mb;
    int      default_disk_gb;
} AppSettings;

void        settings_load(AppData *app);
void        settings_save(AppData *app);
void        ui_show_settings_dialog(AppData *app);

/* ── Keyboard Shortcuts ────────────────────────────────────── */
void        ui_setup_shortcuts(AppData *app);

/* ── Console/Terminal ──────────────────────────────────────── */
void        ui_open_vm_console(AppData *app, const char *vm_name);
void        ui_open_ct_console(AppData *app, const char *ct_name);

/* ── Batch Operations ──────────────────────────────────────── */
bool        vm_batch_start(AppData *app, char **names, int count);
bool        vm_batch_stop(AppData *app, char **names, int count);
bool        ct_batch_start(AppData *app, char **names, int count);
bool        ct_batch_stop(AppData *app, char **names, int count);

#endif
