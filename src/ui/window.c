/*
 * window.c - Main window, sidebar, and all UI panel construction
 */

#include "../include/vmmanager.h"
#include <inttypes.h>

/* ── CSS Stylesheet - Professional Enterprise Design ─────────── */
static const char *APP_CSS =
    "/* Global */\n"
    "window { background-color: #f5f5f5; }\n"
    "\n"
    "/* Sidebar */\n"
    ".sidebar { background-color: #2c2c2c; border-right: 1px solid #3c3c3c; }\n"
    ".sidebar-title { font-size: 13px; font-weight: 600; color: #ffffff; \n"
    "                 padding: 20px 16px; letter-spacing: 0.5px; }\n"
    ".sidebar-btn { background: transparent; border-radius: 0px; color: #b0b0b0;\n"
    "               padding: 12px 16px; margin: 0px; border: none;\n"
    "               font-size: 13px; font-weight: 400; }\n"
    ".sidebar-btn:hover { background-color: #3a3a3a; color: #ffffff; }\n"
    ".sidebar-btn:checked, .sidebar-btn.active { background-color: #0066cc;\n"
    "               color: #ffffff; font-weight: 500; }\n"
    ".sidebar-version { color: #707070; font-size: 11px; padding: 12px 16px; }\n"
    "\n"
    "/* Cards */\n"
    ".card { background-color: #ffffff; border-radius: 2px; padding: 16px;\n"
    "        border: 1px solid #e0e0e0; }\n"
    ".card-title { font-size: 11px; color: #707070; font-weight: 500;\n"
    "              letter-spacing: 0.5px; text-transform: uppercase; }\n"
    ".card-value { font-size: 24px; font-weight: 600; color: #212121; }\n"
    ".card-small { font-size: 12px; color: #616161; }\n"
    "\n"
    "/* Progress bars */\n"
    "progressbar { min-height: 6px; }\n"
    "progressbar trough { background-color: #e0e0e0; border-radius: 0px; min-height: 6px; }\n"
    "progressbar progress { border-radius: 0px; min-height: 6px; }\n"
    ".bar-cpu progress { background-color: #0066cc; }\n"
    ".bar-ram progress { background-color: #00875a; }\n"
    ".bar-disk progress { background-color: #997700; }\n"
    "\n"
    "/* Resource list rows */\n"
    ".resource-row { background-color: #ffffff; border-radius: 0px;\n"
    "                padding: 14px 16px; margin: 0px 0px 1px 0px; \n"
    "                border: 1px solid #e0e0e0; }\n"
    ".resource-row:hover { background-color: #fafafa; }\n"
    ".resource-row:selected { background-color: #e3fcef; }\n"
    ".resource-name { font-size: 14px; font-weight: 500; color: #212121; }\n"
    ".resource-detail { font-size: 12px; color: #707070; }\n"
    "\n"
    "/* State badges */\n"
    ".state-badge { border-radius: 2px; padding: 2px 8px; font-size: 11px;\n"
    "               font-weight: 500; text-transform: uppercase; }\n"
    ".state-running { background-color: #e3fcef; color: #006644; }\n"
    ".state-stopped { background-color: #ffebe6; color: #bf2600; }\n"
    ".state-paused  { background-color: #fff0e0; color: #997700; }\n"
    ".state-crashed { background-color: #ffebe6; color: #bf2600; }\n"
    ".state-unknown { background-color: #f4f5f7; color: #5e6c84; }\n"
    "\n"
    "/* Action buttons */\n"
    ".btn-action { border-radius: 2px; padding: 6px 12px; font-size: 12px;\n"
    "              font-weight: 500; border: 1px solid transparent; min-height: 28px; }\n"
    ".btn-start   { background-color: #ffffff; color: #0066cc; border-color: #0066cc; }\n"
    ".btn-start:hover { background-color: #0066cc; color: #ffffff; }\n"
    ".btn-stop    { background-color: #ffffff; color: #bf2600; border-color: #bf2600; }\n"
    ".btn-stop:hover { background-color: #bf2600; color: #ffffff; }\n"
    ".btn-pause   { background-color: #ffffff; color: #997700; border-color: #997700; }\n"
    ".btn-pause:hover { background-color: #997700; color: #ffffff; }\n"
    ".btn-delete  { background-color: #ffffff; color: #bf2600; border-color: #bf2600; }\n"
    ".btn-delete:hover { background-color: #bf2600; color: #ffffff; }\n"
    ".btn-create  { background-color: #0066cc; color: #ffffff; font-size: 13px;\n"
    "               padding: 8px 16px; border-radius: 2px; font-weight: 500; }\n"
    ".btn-create:hover { background-color: #0052a3; }\n"
    ".btn-reboot  { background-color: #ffffff; color: #5e6c84; border-color: #dfe1e6; }\n"
    ".btn-reboot:hover { background-color: #f4f5f7; }\n"
    "\n"
    "/* Section headers */\n"
    ".section-title { font-size: 18px; font-weight: 600; color: #212121; }\n"
    ".section-subtitle { font-size: 13px; color: #707070; }\n"
    "\n"
    "/* Log view */\n"
    ".log-view { background-color: #1e1e1e; color: #d4d4d4;\n"
    "            font-family: monospace; font-size: 12px;\n"
    "            border-radius: 0px; padding: 12px; border: 1px solid #303030; }\n"
    "textview { background-color: #1e1e1e; }\n"
    "textview text { background-color: #1e1e1e; color: #d4d4d4; }\n"
    "\n"
    "/* Header bar */\n"
    "headerbar { background-color: #2c2c2c; border-bottom: 1px solid #3c3c3c; }\n"
    "headerbar title { color: #ffffff; }\n"
    "\n"
    "/* Scrolled window */\n"
    "scrolledwindow { background-color: transparent; }\n"
    "\n"
    "/* Dialog styling */\n"
    ".dialog-label { font-size: 13px; color: #42526e; font-weight: 500; }\n"
    ".dialog-entry { min-height: 32px; padding: 4px 8px; }\n"
    ".dialog-hint { font-size: 11px; color: #707070; }\n"
    "\n"
    "/* Search and filter */\n"
    ".search-box { background-color: #ffffff; border: 1px solid #dfe1e6; "
    "              border-radius: 4px; padding: 4px 8px; }\n"
    ".search-entry { background: transparent; border: none; min-height: 28px; }\n"
    ".filter-btn { background-color: #f4f5f7; color: #42526e; border-radius: 4px; "
    "              padding: 4px 12px; font-size: 12px; }\n"
    ".filter-btn:checked { background-color: #0066cc; color: #ffffff; }\n"
    ".filter-box { background-color: #ffffff; border-bottom: 1px solid #e0e0e0; "
    "              padding: 8px 16px; }\n"
    "\n"
    "/* Loading overlay */\n"
    ".loading-overlay { background-color: rgba(255, 255, 255, 0.9); }\n"
    ".loading-spinner { color: #0066cc; }\n"
    ".loading-label { font-size: 13px; color: #42526e; }\n"
    "\n"
    "/* Empty state */\n"
    ".empty-state { padding: 48px 24px; text-align: center; }\n"
    ".empty-icon { font-size: 48px; color: #dfe1e6; }\n"
    ".empty-title { font-size: 16px; font-weight: 500; color: #212121; margin-top: 16px; }\n"
    ".empty-desc { font-size: 13px; color: #707070; margin-top: 8px; }\n";

/* ── Apply CSS ─────────────────────────────────────────────── */
static void apply_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, APP_CSS);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

/* ── Helper: create a labeled progress card ────────────────── */
static GtkWidget *make_stat_card(const char *title, GtkWidget **bar,
                                  GtkWidget **label, const char *bar_class) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(card, "card");
    gtk_widget_set_hexpand(card, TRUE);

    GtkWidget *title_lbl = gtk_label_new(title);
    gtk_widget_add_css_class(title_lbl, "card-title");
    gtk_label_set_xalign(GTK_LABEL(title_lbl), 0);
    gtk_box_append(GTK_BOX(card), title_lbl);

    *label = gtk_label_new("0%");
    gtk_widget_add_css_class(*label, "card-value");
    gtk_label_set_xalign(GTK_LABEL(*label), 0);
    gtk_box_append(GTK_BOX(card), *label);

    *bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(*bar), 0.0);
    gtk_widget_add_css_class(*bar, bar_class);
    gtk_box_append(GTK_BOX(card), *bar);

    return card;
}

/* ── Helper: count card ────────────────────────────────────── */
static GtkWidget *make_count_card(const char *title, GtkWidget **running_lbl,
                                   GtkWidget **total_lbl) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_add_css_class(card, "card");
    gtk_widget_set_hexpand(card, TRUE);

    GtkWidget *title_w = gtk_label_new(title);
    gtk_widget_add_css_class(title_w, "card-title");
    gtk_label_set_xalign(GTK_LABEL(title_w), 0);
    gtk_box_append(GTK_BOX(card), title_w);

    *running_lbl = gtk_label_new("0");
    gtk_widget_add_css_class(*running_lbl, "card-value");
    gtk_label_set_xalign(GTK_LABEL(*running_lbl), 0);
    gtk_box_append(GTK_BOX(card), *running_lbl);

    GtkWidget *sub_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *total_prefix = gtk_label_new("Total:");
    gtk_widget_add_css_class(total_prefix, "card-small");
    *total_lbl = gtk_label_new("0");
    gtk_widget_add_css_class(*total_lbl, "card-small");
    gtk_box_append(GTK_BOX(sub_box), total_prefix);
    gtk_box_append(GTK_BOX(sub_box), *total_lbl);
    gtk_box_append(GTK_BOX(card), sub_box);

    return card;
}

/* ── Build Dashboard ───────────────────────────────────────── */
GtkWidget *ui_build_dashboard(AppData *app) {
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);
    gtk_widget_set_margin_top(box, 24);
    gtk_widget_set_margin_bottom(box, 24);

    /* Title */
    GtkWidget *title = gtk_label_new("Dashboard");
    gtk_widget_add_css_class(title, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(box), title);

    GtkWidget *subtitle = gtk_label_new("System overview and resource utilization");
    gtk_widget_add_css_class(subtitle, "section-subtitle");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0);
    gtk_box_append(GTK_BOX(box), subtitle);

    /* Resource bars row */
    GtkWidget *res_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_append(GTK_BOX(res_row),
        make_stat_card("CPU", &app->dash_cpu_bar, &app->dash_cpu_label, "bar-cpu"));
    gtk_box_append(GTK_BOX(res_row),
        make_stat_card("MEMORY", &app->dash_ram_bar, &app->dash_ram_label, "bar-ram"));
    gtk_box_append(GTK_BOX(res_row),
        make_stat_card("DISK", &app->dash_disk_bar, &app->dash_disk_label, "bar-disk"));
    gtk_box_append(GTK_BOX(box), res_row);

    /* Count cards row */
    GtkWidget *count_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_append(GTK_BOX(count_row),
        make_count_card("VIRTUAL MACHINES",
                        &app->dash_vm_running_label, &app->dash_vm_total_label));
    gtk_box_append(GTK_BOX(count_row),
        make_count_card("CONTAINERS",
                        &app->dash_ct_running_label, &app->dash_ct_total_label));
    gtk_box_append(GTK_BOX(box), count_row);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), box);
    return scroll;
}

/* ──────────────────────────────────────────────────────────── */
/* ── VM Action Callbacks ───────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

typedef struct {
    AppData *app;
    char name[256];
} ActionCtx;

/* Confirmation context for destructive actions */
typedef struct {
    AppData *app;
    char name[256];
    bool confirmed;
} ConfirmActionCtx;

/* ── ActionCtx cleanup callback ──────────────────────────────── */
static void on_ctx_destroy(gpointer data) {
    if (data) g_free(data);
}

static void on_vm_start(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    vm_start(ctx->app, ctx->name);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
}
static void on_vm_shutdown(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    vm_shutdown(ctx->app, ctx->name);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
}
static void on_vm_force_stop_confirmed(gpointer data) {
    ConfirmActionCtx *ctx = data;
    vm_force_stop(ctx->app, ctx->name);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
    g_free(ctx);
}

static void on_vm_force_stop(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    
    ConfirmActionCtx *confirm_ctx = g_new0(ConfirmActionCtx, 1);
    confirm_ctx->app = ctx->app;
    g_strlcpy(confirm_ctx->name, ctx->name, sizeof(confirm_ctx->name));
    
    char message[512];
    snprintf(message, sizeof(message), "Force stop will immediately kill VM '%s' without graceful shutdown. Continue?", ctx->name);
    
    ui_show_confirm_dialog(ctx->app, "Force Stop Virtual Machine", message,
                          G_CALLBACK(on_vm_force_stop_confirmed), confirm_ctx);
}
static void on_vm_pause(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    vm_pause(ctx->app, ctx->name);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
}
static void on_vm_resume(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    vm_resume(ctx->app, ctx->name);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
}
static void on_vm_reboot(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    vm_reboot(ctx->app, ctx->name);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
}
static void on_vm_delete_confirmed(gpointer data) {
    ConfirmActionCtx *ctx = data;
    vm_delete(ctx->app, ctx->name);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
    ui_refresh_dashboard(ctx->app);
    g_free(ctx);
}

static void on_vm_delete(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    
    ConfirmActionCtx *confirm_ctx = g_new0(ConfirmActionCtx, 1);
    confirm_ctx->app = ctx->app;
    g_strlcpy(confirm_ctx->name, ctx->name, sizeof(confirm_ctx->name));
    
    char message[512];
    snprintf(message, sizeof(message), "Are you sure you want to delete VM '%s'?", ctx->name);
    
    ui_show_confirm_dialog(ctx->app, "Delete Virtual Machine", message,
                          G_CALLBACK(on_vm_delete_confirmed), confirm_ctx);
}

/* ── Build a single VM row ─────────────────────────────────── */
static void on_vm_details(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    for (int i = 0; i < ctx->app->vm_count; i++) {
        if (strcmp(ctx->app->vms[i].name, ctx->name) == 0) {
            ui_show_vm_details_dialog(ctx->app, &ctx->app->vms[i]);
            break;
        }
    }
}

static void on_vm_snapshot(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    ui_show_snapshot_dialog(ctx->app, ctx->name);
}

static void on_vm_console(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    ui_open_vm_console(ctx->app, ctx->name);
}

static GtkWidget *make_vm_row(AppData *app, VmInfo *vm) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(row, "resource-row");

    /* Info section */
    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(info_box, TRUE);

    GtkWidget *name_lbl = gtk_label_new(vm->name);
    gtk_widget_add_css_class(name_lbl, "resource-name");
    gtk_label_set_xalign(GTK_LABEL(name_lbl), 0);
    gtk_box_append(GTK_BOX(info_box), name_lbl);

    char detail[256];
    snprintf(detail, sizeof(detail), "%d vCPU  •  %lu MB RAM  •  %s",
             vm->vcpus, vm->max_mem_kb / 1024,
             vm->autostart ? "Autostart ON" : "");
    GtkWidget *detail_lbl = gtk_label_new(detail);
    gtk_widget_add_css_class(detail_lbl, "resource-detail");
    gtk_label_set_xalign(GTK_LABEL(detail_lbl), 0);
    gtk_box_append(GTK_BOX(info_box), detail_lbl);

    gtk_box_append(GTK_BOX(row), info_box);

    /* State badge */
    GtkWidget *badge = gtk_label_new(vm_state_str(vm->state));
    gtk_widget_add_css_class(badge, "state-badge");
    gtk_widget_add_css_class(badge, vm_state_css(vm->state));
    gtk_widget_set_valign(badge, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(row), badge);

    /* Action buttons */
    ActionCtx *ctx = g_new0(ActionCtx, 1);
    ctx->app = app;
    strncpy(ctx->name, vm->name, 255);
    ctx->name[255] = '\0';

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_valign(btn_box, GTK_ALIGN_CENTER);
    g_object_set_data_full(G_OBJECT(btn_box), "action-ctx", ctx, on_ctx_destroy);

    /* Details button - always visible */
    GtkWidget *details_btn = gtk_button_new_with_label("Details");
    gtk_widget_add_css_class(details_btn, "btn-action");
    gtk_widget_add_css_class(details_btn, "btn-reboot");
    g_signal_connect(details_btn, "clicked", G_CALLBACK(on_vm_details), ctx);
    gtk_box_append(GTK_BOX(btn_box), details_btn);

    if (vm->state == VM_STATE_SHUTOFF) {
        GtkWidget *b = gtk_button_new_with_label("Start");
        gtk_widget_add_css_class(b, "btn-action");
        gtk_widget_add_css_class(b, "btn-start");
        g_signal_connect(b, "clicked", G_CALLBACK(on_vm_start), ctx);
        gtk_box_append(GTK_BOX(btn_box), b);
        
        GtkWidget *del_btn = gtk_button_new_with_label("Delete");
        gtk_widget_add_css_class(del_btn, "btn-action");
        gtk_widget_add_css_class(del_btn, "btn-delete");
        g_signal_connect(del_btn, "clicked", G_CALLBACK(on_vm_delete), ctx);
        gtk_box_append(GTK_BOX(btn_box), del_btn);
    } else if (vm->state == VM_STATE_RUNNING) {
        GtkWidget *console_btn = gtk_button_new_with_label("Console");
        gtk_widget_add_css_class(console_btn, "btn-action");
        gtk_widget_add_css_class(console_btn, "btn-create");
        g_signal_connect(console_btn, "clicked", G_CALLBACK(on_vm_console), ctx);
        gtk_box_append(GTK_BOX(btn_box), console_btn);
        
        GtkWidget *b1 = gtk_button_new_with_label("Shutdown");
        gtk_widget_add_css_class(b1, "btn-action");
        gtk_widget_add_css_class(b1, "btn-stop");
        g_signal_connect(b1, "clicked", G_CALLBACK(on_vm_shutdown), ctx);
        gtk_box_append(GTK_BOX(btn_box), b1);

        GtkWidget *b2 = gtk_button_new_with_label("Pause");
        gtk_widget_add_css_class(b2, "btn-action");
        gtk_widget_add_css_class(b2, "btn-pause");
        g_signal_connect(b2, "clicked", G_CALLBACK(on_vm_pause), ctx);
        gtk_box_append(GTK_BOX(btn_box), b2);

        GtkWidget *b3 = gtk_button_new_with_label("Reboot");
        gtk_widget_add_css_class(b3, "btn-action");
        gtk_widget_add_css_class(b3, "btn-reboot");
        g_signal_connect(b3, "clicked", G_CALLBACK(on_vm_reboot), ctx);
        gtk_box_append(GTK_BOX(btn_box), b3);

        GtkWidget *kill_btn = gtk_button_new_with_label("Kill");
        gtk_widget_add_css_class(kill_btn, "btn-action");
        gtk_widget_add_css_class(kill_btn, "btn-delete");
        g_signal_connect(kill_btn, "clicked", G_CALLBACK(on_vm_force_stop), ctx);
        gtk_box_append(GTK_BOX(btn_box), kill_btn);
    } else if (vm->state == VM_STATE_PAUSED) {
        GtkWidget *b = gtk_button_new_with_label("Resume");
        gtk_widget_add_css_class(b, "btn-action");
        gtk_widget_add_css_class(b, "btn-start");
        g_signal_connect(b, "clicked", G_CALLBACK(on_vm_resume), ctx);
        gtk_box_append(GTK_BOX(btn_box), b);
        
        GtkWidget *kill_btn = gtk_button_new_with_label("Kill");
        gtk_widget_add_css_class(kill_btn, "btn-action");
        gtk_widget_add_css_class(kill_btn, "btn-delete");
        g_signal_connect(kill_btn, "clicked", G_CALLBACK(on_vm_force_stop), ctx);
        gtk_box_append(GTK_BOX(btn_box), kill_btn);
    }

    /* Snapshot button */
    GtkWidget *snap_btn = gtk_button_new_with_label("Snapshots");
    gtk_widget_add_css_class(snap_btn, "btn-action");
    gtk_widget_add_css_class(snap_btn, "btn-reboot");
    g_signal_connect(snap_btn, "clicked", G_CALLBACK(on_vm_snapshot), ctx);
    gtk_box_append(GTK_BOX(btn_box), snap_btn);

    gtk_box_append(GTK_BOX(row), btn_box);
    return row;
}

/* ── VM Create Dialog ──────────────────────────────────────── */
typedef struct {
    AppData    *app;
    GtkWidget  *name_entry;
    GtkWidget  *vcpu_spin;
    GtkWidget  *ram_spin;
    GtkWidget  *disk_spin;
    GtkWidget  *iso_entry;
    GtkWidget  *dialog;
} VmCreateCtx;

static void on_vm_create_confirm(GtkButton *btn, gpointer data) {
    (void)btn;
    VmCreateCtx *ctx = data;

    const char *name = gtk_editable_get_text(GTK_EDITABLE(ctx->name_entry));
    int vcpus = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ctx->vcpu_spin));
    int ram   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ctx->ram_spin));
    int disk  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ctx->disk_spin));
    const char *iso = gtk_editable_get_text(GTK_EDITABLE(ctx->iso_entry));

    /* Input validation */
    if (strlen(name) == 0) {
        app_log(ctx->app, "WARN", "VM name is required");
        return;
    }
    
    /* Validate name: only alphanumeric, dash, underscore */
    for (const char *p = name; *p; p++) {
        if (!g_ascii_isalnum(*p) && *p != '-' && *p != '_') {
            app_log(ctx->app, "WARN", "VM name can only contain alphanumeric, dash, or underscore");
            return;
        }
    }

    /* Validate ISO file if provided */
    if (iso && strlen(iso) > 0 && access(iso, R_OK) != 0) {
        app_log(ctx->app, "WARN", "ISO file not found or not readable: %s", iso);
        ui_show_error_dialog(ctx->app, "Invalid ISO Path",
            "The specified ISO file does not exist or is not readable.",
            "Check the path and ensure the file exists.");
        return;
    }

    vm_create(ctx->app, name, vcpus, ram, disk, iso);
    vm_backend_refresh(ctx->app);
    ui_refresh_vm_list(ctx->app);
    ui_refresh_dashboard(ctx->app);

    gtk_window_destroy(GTK_WINDOW(ctx->dialog));
    g_free(ctx);
}

void ui_show_vm_create_dialog(AppData *app) {
    VmCreateCtx *ctx = g_new0(VmCreateCtx, 1);
    ctx->app = app;

    GtkWidget *dialog = gtk_window_new();
    ctx->dialog = dialog;
    gtk_window_set_title(GTK_WINDOW(dialog), "Create Virtual Machine");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 420);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    /* Header */
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(header, 20);
    gtk_widget_set_margin_end(header, 20);
    gtk_widget_set_margin_top(header, 20);
    gtk_widget_set_margin_bottom(header, 16);
    
    GtkWidget *title = gtk_label_new("Create Virtual Machine");
    gtk_widget_add_css_class(title, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(header), title);
    
    GtkWidget *subtitle = gtk_label_new("Configure the new virtual machine settings");
    gtk_widget_add_css_class(subtitle, "section-subtitle");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0);
    gtk_box_append(GTK_BOX(header), subtitle);
    
    gtk_box_append(GTK_BOX(main_box), header);

    /* Form */
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    /* Name */
    GtkWidget *name_label = gtk_label_new("Name");
    gtk_widget_add_css_class(name_label, "dialog-label");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_append(GTK_BOX(form), name_label);
    
    ctx->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->name_entry), "vm-name");
    gtk_widget_add_css_class(ctx->name_entry, "dialog-entry");
    gtk_box_append(GTK_BOX(form), ctx->name_entry);

    /* vCPU */
    GtkWidget *vcpu_label = gtk_label_new("Virtual CPUs");
    gtk_widget_add_css_class(vcpu_label, "dialog-label");
    gtk_label_set_xalign(GTK_LABEL(vcpu_label), 0);
    gtk_box_append(GTK_BOX(form), vcpu_label);
    
    ctx->vcpu_spin = gtk_spin_button_new_with_range(1, 16, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->vcpu_spin),
        app->settings.loaded ? app->settings.default_vcpus : 2);
    gtk_box_append(GTK_BOX(form), ctx->vcpu_spin);

    /* RAM */
    GtkWidget *ram_label = gtk_label_new("Memory (MB)");
    gtk_widget_add_css_class(ram_label, "dialog-label");
    gtk_label_set_xalign(GTK_LABEL(ram_label), 0);
    gtk_box_append(GTK_BOX(form), ram_label);

    ctx->ram_spin = gtk_spin_button_new_with_range(256, 32768, 256);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->ram_spin),
        app->settings.loaded ? app->settings.default_ram_mb : 2048);
    gtk_box_append(GTK_BOX(form), ctx->ram_spin);

    /* Disk */
    GtkWidget *disk_label = gtk_label_new("Disk Size (GB)");
    gtk_widget_add_css_class(disk_label, "dialog-label");
    gtk_label_set_xalign(GTK_LABEL(disk_label), 0);
    gtk_box_append(GTK_BOX(form), disk_label);

    ctx->disk_spin = gtk_spin_button_new_with_range(5, 500, 5);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->disk_spin),
        app->settings.loaded ? app->settings.default_disk_gb : 20);
    gtk_box_append(GTK_BOX(form), ctx->disk_spin);

    /* ISO */
    GtkWidget *iso_label = gtk_label_new("ISO Path (Optional)");
    gtk_widget_add_css_class(iso_label, "dialog-label");
    gtk_label_set_xalign(GTK_LABEL(iso_label), 0);
    gtk_box_append(GTK_BOX(form), iso_label);
    
    ctx->iso_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->iso_entry), "/path/to/installer.iso");
    gtk_widget_add_css_class(ctx->iso_entry, "dialog-entry");
    gtk_box_append(GTK_BOX(form), ctx->iso_entry);

    gtk_box_append(GTK_BOX(main_box), form);

    /* Button row */
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(btn_row, 20);
    gtk_widget_set_margin_end(btn_row, 20);
    gtk_widget_set_margin_bottom(btn_row, 20);
    gtk_widget_set_halign(btn_row, GTK_ALIGN_END);

    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    gtk_widget_add_css_class(cancel_btn, "btn-action");
    gtk_widget_add_css_class(cancel_btn, "btn-reboot");
    g_signal_connect_swapped(cancel_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(btn_row), cancel_btn);

    GtkWidget *create_btn = gtk_button_new_with_label("Create");
    gtk_widget_add_css_class(create_btn, "btn-action");
    gtk_widget_add_css_class(create_btn, "btn-create");
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_vm_create_confirm), ctx);
    gtk_box_append(GTK_BOX(btn_row), create_btn);

    gtk_box_append(GTK_BOX(main_box), btn_row);

    gtk_window_set_child(GTK_WINDOW(dialog), main_box);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ── VM search/filter callbacks ─────────────────────────────── */
static void on_vm_search_changed(GtkEditable *entry, gpointer data) {
    AppData *app = data;
    const char *text = gtk_editable_get_text(entry);
    strncpy(app->vm_search, text ? text : "", sizeof(app->vm_search) - 1);
    ui_refresh_vm_list(app);
}

static void on_vm_filter_all(GtkToggleButton *btn, gpointer data) {
    if (!gtk_toggle_button_get_active(btn)) return;
    ui_set_vm_filter((AppData *)data, FILTER_ALL, NULL);
}
static void on_vm_filter_running(GtkToggleButton *btn, gpointer data) {
    if (!gtk_toggle_button_get_active(btn)) return;
    ui_set_vm_filter((AppData *)data, FILTER_RUNNING, NULL);
}
static void on_vm_filter_stopped(GtkToggleButton *btn, gpointer data) {
    if (!gtk_toggle_button_get_active(btn)) return;
    ui_set_vm_filter((AppData *)data, FILTER_STOPPED, NULL);
}

/* ── Build VM Panel ────────────────────────────────────────── */
GtkWidget *ui_build_vm_panel(AppData *app) {
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /* Header row */
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(header, 24);
    gtk_widget_set_margin_end(header, 24);
    gtk_widget_set_margin_top(header, 24);
    gtk_widget_set_margin_bottom(header, 12);

    GtkWidget *title = gtk_label_new("Virtual Machines");
    gtk_widget_add_css_class(title, "section-title");
    gtk_widget_set_hexpand(title, TRUE);
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(header), title);

    app->vm_status_label = gtk_label_new("0 running / 0 total");
    gtk_widget_add_css_class(app->vm_status_label, "section-subtitle");
    gtk_widget_set_valign(app->vm_status_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(header), app->vm_status_label);

    GtkWidget *create_btn = gtk_button_new_with_label("Create");
    gtk_widget_add_css_class(create_btn, "btn-action");
    gtk_widget_add_css_class(create_btn, "btn-create");
    g_signal_connect_swapped(create_btn, "clicked",
                             G_CALLBACK(ui_show_vm_create_dialog), app);
    gtk_box_append(GTK_BOX(header), create_btn);

    gtk_box_append(GTK_BOX(outer_box), header);

    /* Search and filter bar */
    GtkWidget *filter_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_add_css_class(filter_box, "filter-box");
    gtk_widget_set_margin_start(filter_box, 24);
    gtk_widget_set_margin_end(filter_box, 24);
    gtk_widget_set_margin_bottom(filter_box, 12);

    /* Search entry */
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_add_css_class(search_box, "search-box");
    gtk_widget_set_hexpand(search_box, TRUE);

    GtkWidget *search_icon = gtk_image_new_from_icon_name("system-search-symbolic");
    gtk_box_append(GTK_BOX(search_box), search_icon);

    GtkWidget *search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search VMs...");
    gtk_widget_add_css_class(search_entry, "search-entry");
    gtk_widget_set_hexpand(search_entry, TRUE);
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_vm_search_changed), app);
    gtk_box_append(GTK_BOX(search_box), search_entry);

    gtk_box_append(GTK_BOX(filter_box), search_box);

    /* Filter buttons (grouped) */
    GtkWidget *all_btn = gtk_toggle_button_new_with_label("All");
    gtk_widget_add_css_class(all_btn, "filter-btn");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(all_btn), TRUE);
    g_signal_connect(all_btn, "toggled", G_CALLBACK(on_vm_filter_all), app);
    gtk_box_append(GTK_BOX(filter_box), all_btn);

    GtkWidget *running_btn = gtk_toggle_button_new_with_label("Running");
    gtk_widget_add_css_class(running_btn, "filter-btn");
    gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(running_btn), GTK_TOGGLE_BUTTON(all_btn));
    g_signal_connect(running_btn, "toggled", G_CALLBACK(on_vm_filter_running), app);
    gtk_box_append(GTK_BOX(filter_box), running_btn);

    GtkWidget *stopped_btn = gtk_toggle_button_new_with_label("Stopped");
    gtk_widget_add_css_class(stopped_btn, "filter-btn");
    gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(stopped_btn), GTK_TOGGLE_BUTTON(all_btn));
    g_signal_connect(stopped_btn, "toggled", G_CALLBACK(on_vm_filter_stopped), app);
    gtk_box_append(GTK_BOX(filter_box), stopped_btn);

    gtk_box_append(GTK_BOX(outer_box), filter_box);

    /* VM list container */
    app->vm_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(app->vm_list_box, 24);
    gtk_widget_set_margin_end(app->vm_list_box, 24);
    gtk_widget_set_margin_bottom(app->vm_list_box, 24);
    gtk_box_append(GTK_BOX(outer_box), app->vm_list_box);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), outer_box);
    return scroll;
}

/* ── Check if VM matches current filter ────────────────────── */
static bool vm_matches_filter(AppData *app, VmInfo *vm) {
    /* State filter */
    switch (app->vm_filter) {
        case FILTER_RUNNING: if (vm->state != VM_STATE_RUNNING) return false; break;
        case FILTER_STOPPED: if (vm->state != VM_STATE_SHUTOFF) return false; break;
        case FILTER_PAUSED:  if (vm->state != VM_STATE_PAUSED) return false; break;
        default: break;
    }
    /* Search filter */
    if (app->vm_search[0]) {
        if (!strcasestr(vm->name, app->vm_search)) return false;
    }
    return true;
}

/* ── Refresh VM list UI ────────────────────────────────────── */
void ui_refresh_vm_list(AppData *app) {
    if (!app->vm_list_box) return;

    /* Clear old children */
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app->vm_list_box)))
        gtk_box_remove(GTK_BOX(app->vm_list_box), child);

    int shown = 0;
    if (app->vm_count > 0) {
        for (int i = 0; i < app->vm_count; i++) {
            if (vm_matches_filter(app, &app->vms[i])) {
                gtk_box_append(GTK_BOX(app->vm_list_box), make_vm_row(app, &app->vms[i]));
                shown++;
            }
        }
    }

    if (shown == 0) {
        const char *msg = app->vm_count == 0
            ? "No virtual machines found. Create one to get started."
            : "No VMs match the current filter.";
        GtkWidget *empty = gtk_label_new(msg);
        gtk_widget_add_css_class(empty, "section-subtitle");
        gtk_box_append(GTK_BOX(app->vm_list_box), empty);
    }

    char status[64];
    snprintf(status, sizeof(status), "%d running / %d total",
             app->stats.running_vms, app->stats.total_vms);
    gtk_label_set_text(GTK_LABEL(app->vm_status_label), status);
}

/* ── Set VM filter ─────────────────────────────────────────── */
void ui_set_vm_filter(AppData *app, FilterState filter, const char *search) {
    app->vm_filter = filter;
    if (search)
        strncpy(app->vm_search, search, sizeof(app->vm_search) - 1);
    else
        app->vm_search[0] = '\0';
    ui_refresh_vm_list(app);
}

/* ──────────────────────────────────────────────────────────── */
/* ── Container Panel ───────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

static void on_ct_start(GtkButton *btn, gpointer data) {
    (void)btn; ActionCtx *ctx = data;
    ct_start(ctx->app, ctx->name);
    ct_backend_refresh(ctx->app);
    ui_refresh_ct_list(ctx->app);
}
static void on_ct_stop(GtkButton *btn, gpointer data) {
    (void)btn; ActionCtx *ctx = data;
    ct_stop(ctx->app, ctx->name);
    ct_backend_refresh(ctx->app);
    ui_refresh_ct_list(ctx->app);
}
static void on_ct_force_stop_confirmed(gpointer data) {
    ConfirmActionCtx *ctx = data;
    ct_force_stop(ctx->app, ctx->name);
    ct_backend_refresh(ctx->app);
    ui_refresh_ct_list(ctx->app);
    g_free(ctx);
}

static void on_ct_force_stop(GtkButton *btn, gpointer data) {
    (void)btn; ActionCtx *ctx = data;
    
    ConfirmActionCtx *confirm_ctx = g_new0(ConfirmActionCtx, 1);
    confirm_ctx->app = ctx->app;
    g_strlcpy(confirm_ctx->name, ctx->name, sizeof(confirm_ctx->name));
    
    char message[512];
    snprintf(message, sizeof(message), "Force stop will immediately kill container '%s'. Continue?", ctx->name);
    
    ui_show_confirm_dialog(ctx->app, "Force Stop Container", message,
                          G_CALLBACK(on_ct_force_stop_confirmed), confirm_ctx);
}
static void on_ct_restart(GtkButton *btn, gpointer data) {
    (void)btn; ActionCtx *ctx = data;
    ct_restart(ctx->app, ctx->name);
    ct_backend_refresh(ctx->app);
    ui_refresh_ct_list(ctx->app);
}
static void on_ct_delete_confirmed(gpointer data) {
    ConfirmActionCtx *ctx = data;
    ct_delete(ctx->app, ctx->name);
    ct_backend_refresh(ctx->app);
    ui_refresh_ct_list(ctx->app);
    ui_refresh_dashboard(ctx->app);
    g_free(ctx);
}

static void on_ct_delete(GtkButton *btn, gpointer data) {
    (void)btn; ActionCtx *ctx = data;
    
    ConfirmActionCtx *confirm_ctx = g_new0(ConfirmActionCtx, 1);
    confirm_ctx->app = ctx->app;
    g_strlcpy(confirm_ctx->name, ctx->name, sizeof(confirm_ctx->name));
    
    char message[512];
    snprintf(message, sizeof(message), "Are you sure you want to delete container '%s'?", ctx->name);
    
    ui_show_confirm_dialog(ctx->app, "Delete Container", message,
                          G_CALLBACK(on_ct_delete_confirmed), confirm_ctx);
}

static void on_ct_details(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    for (int i = 0; i < ctx->app->ct_count; i++) {
        if (strcmp(ctx->app->containers[i].name, ctx->name) == 0) {
            ui_show_ct_details_dialog(ctx->app, &ctx->app->containers[i]);
            break;
        }
    }
}

static void on_ct_console(GtkButton *btn, gpointer data) {
    (void)btn;
    ActionCtx *ctx = data;
    ui_open_ct_console(ctx->app, ctx->name);
}

static GtkWidget *make_ct_row(AppData *app, CtInfo *ct) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(row, "resource-row");

    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(info_box, TRUE);

    GtkWidget *name_lbl = gtk_label_new(ct->name);
    gtk_widget_add_css_class(name_lbl, "resource-name");
    gtk_label_set_xalign(GTK_LABEL(name_lbl), 0);
    gtk_box_append(GTK_BOX(info_box), name_lbl);

    char detail[256];
    snprintf(detail, sizeof(detail), "%s  •  %s%s%s",
             ct->type[0] ? ct->type : "container",
             ct->ipv4[0] ? "IP: " : "",
             ct->ipv4[0] ? ct->ipv4 : "No IP",
             "");
    GtkWidget *detail_lbl = gtk_label_new(detail);
    gtk_widget_add_css_class(detail_lbl, "resource-detail");
    gtk_label_set_xalign(GTK_LABEL(detail_lbl), 0);
    gtk_box_append(GTK_BOX(info_box), detail_lbl);

    gtk_box_append(GTK_BOX(row), info_box);

    /* Badge */
    GtkWidget *badge = gtk_label_new(ct_state_str(ct->state));
    gtk_widget_add_css_class(badge, "state-badge");
    gtk_widget_add_css_class(badge, ct_state_css(ct->state));
    gtk_widget_set_valign(badge, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(row), badge);

    /* Actions */
    ActionCtx *ctx = g_new0(ActionCtx, 1);
    ctx->app = app;
    strncpy(ctx->name, ct->name, 255);
    ctx->name[255] = '\0';

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_valign(btn_box, GTK_ALIGN_CENTER);
    g_object_set_data_full(G_OBJECT(btn_box), "action-ctx", ctx, on_ctx_destroy);

    /* Details button - always visible */
    GtkWidget *details_btn = gtk_button_new_with_label("Details");
    gtk_widget_add_css_class(details_btn, "btn-action");
    gtk_widget_add_css_class(details_btn, "btn-reboot");
    g_signal_connect(details_btn, "clicked", G_CALLBACK(on_ct_details), ctx);
    gtk_box_append(GTK_BOX(btn_box), details_btn);

    if (ct->state == CT_STATE_STOPPED) {
        GtkWidget *b = gtk_button_new_with_label("Start");
        gtk_widget_add_css_class(b, "btn-action"); gtk_widget_add_css_class(b, "btn-start");
        g_signal_connect(b, "clicked", G_CALLBACK(on_ct_start), ctx);
        gtk_box_append(GTK_BOX(btn_box), b);

        GtkWidget *d = gtk_button_new_with_label("Delete");
        gtk_widget_add_css_class(d, "btn-action"); gtk_widget_add_css_class(d, "btn-delete");
        g_signal_connect(d, "clicked", G_CALLBACK(on_ct_delete), ctx);
        gtk_box_append(GTK_BOX(btn_box), d);
    } else if (ct->state == CT_STATE_RUNNING) {
        GtkWidget *console_btn = gtk_button_new_with_label("Shell");
        gtk_widget_add_css_class(console_btn, "btn-action");
        gtk_widget_add_css_class(console_btn, "btn-create");
        g_signal_connect(console_btn, "clicked", G_CALLBACK(on_ct_console), ctx);
        gtk_box_append(GTK_BOX(btn_box), console_btn);
        
        GtkWidget *b1 = gtk_button_new_with_label("Stop");
        gtk_widget_add_css_class(b1, "btn-action"); gtk_widget_add_css_class(b1, "btn-stop");
        g_signal_connect(b1, "clicked", G_CALLBACK(on_ct_stop), ctx);
        gtk_box_append(GTK_BOX(btn_box), b1);

        GtkWidget *b2 = gtk_button_new_with_label("Restart");
        gtk_widget_add_css_class(b2, "btn-action"); gtk_widget_add_css_class(b2, "btn-reboot");
        g_signal_connect(b2, "clicked", G_CALLBACK(on_ct_restart), ctx);
        gtk_box_append(GTK_BOX(btn_box), b2);

        GtkWidget *b3 = gtk_button_new_with_label("Kill");
        gtk_widget_add_css_class(b3, "btn-action"); gtk_widget_add_css_class(b3, "btn-delete");
        g_signal_connect(b3, "clicked", G_CALLBACK(on_ct_force_stop), ctx);
        gtk_box_append(GTK_BOX(btn_box), b3);
    }

    gtk_box_append(GTK_BOX(row), btn_box);
    return row;
}

/* ── CT search/filter callbacks ─────────────────────────────── */
static void on_ct_search_changed(GtkEditable *entry, gpointer data) {
    AppData *app = data;
    const char *text = gtk_editable_get_text(entry);
    strncpy(app->ct_search, text ? text : "", sizeof(app->ct_search) - 1);
    ui_refresh_ct_list(app);
}

static void on_ct_filter_all(GtkToggleButton *btn, gpointer data) {
    if (!gtk_toggle_button_get_active(btn)) return;
    ui_set_ct_filter((AppData *)data, FILTER_ALL, NULL);
}
static void on_ct_filter_running(GtkToggleButton *btn, gpointer data) {
    if (!gtk_toggle_button_get_active(btn)) return;
    ui_set_ct_filter((AppData *)data, FILTER_RUNNING, NULL);
}
static void on_ct_filter_stopped(GtkToggleButton *btn, gpointer data) {
    if (!gtk_toggle_button_get_active(btn)) return;
    ui_set_ct_filter((AppData *)data, FILTER_STOPPED, NULL);
}

/* ── Container Create Dialog ───────────────────────────────── */
typedef struct {
    AppData   *app;
    GtkWidget *name_entry;
    GtkWidget *image_entry;
    GtkWidget *dialog;
} CtCreateCtx;

static void on_ct_create_confirm(GtkButton *btn, gpointer data) {
    (void)btn;
    CtCreateCtx *ctx = data;
    const char *name  = gtk_editable_get_text(GTK_EDITABLE(ctx->name_entry));
    const char *image = gtk_editable_get_text(GTK_EDITABLE(ctx->image_entry));

    /* Input validation */
    if (strlen(name) == 0 || strlen(image) == 0) {
        app_log(ctx->app, "WARN", "Container name and image are required");
        return;
    }
    
    /* Validate name: only alphanumeric, dash, underscore */
    for (const char *p = name; *p; p++) {
        if (!g_ascii_isalnum(*p) && *p != '-' && *p != '_') {
            app_log(ctx->app, "WARN", "Container name can only contain alphanumeric, dash, or underscore");
            return;
        }
    }

    ct_create(ctx->app, name, image);
    /* Use non-blocking delayed refresh - one-shot timer via g_idle_add wrapper */
    g_timeout_add_seconds(1, (GSourceFunc)(void(*)(void))ct_backend_refresh, ctx->app);
    g_timeout_add_seconds(2, (GSourceFunc)(void(*)(void))ui_refresh_ct_list, ctx->app);

    gtk_window_destroy(GTK_WINDOW(ctx->dialog));
    g_free(ctx);
}

void ui_show_ct_create_dialog(AppData *app) {
    CtCreateCtx *ctx = g_new0(CtCreateCtx, 1);
    ctx->app = app;

    GtkWidget *dialog = gtk_window_new();
    ctx->dialog = dialog;
    gtk_window_set_title(GTK_WINDOW(dialog), "Create Container");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 320);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /* Header */
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(header, 20);
    gtk_widget_set_margin_end(header, 20);
    gtk_widget_set_margin_top(header, 20);
    gtk_widget_set_margin_bottom(header, 16);

    GtkWidget *title = gtk_label_new("Create Container");
    gtk_widget_add_css_class(title, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(header), title);

    GtkWidget *subtitle = gtk_label_new("Configure the new container settings");
    gtk_widget_add_css_class(subtitle, "section-subtitle");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0);
    gtk_box_append(GTK_BOX(header), subtitle);

    gtk_box_append(GTK_BOX(main_box), header);

    /* Form */
    GtkWidget *form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(form, 20);
    gtk_widget_set_margin_end(form, 20);
    gtk_widget_set_margin_bottom(form, 20);

    /* Name */
    GtkWidget *name_label = gtk_label_new("Name");
    gtk_widget_add_css_class(name_label, "dialog-label");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_append(GTK_BOX(form), name_label);

    ctx->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->name_entry), "container-name");
    gtk_widget_add_css_class(ctx->name_entry, "dialog-entry");
    gtk_box_append(GTK_BOX(form), ctx->name_entry);

    /* Image */
    GtkWidget *image_label = gtk_label_new("Image");
    gtk_widget_add_css_class(image_label, "dialog-label");
    gtk_label_set_xalign(GTK_LABEL(image_label), 0);
    gtk_box_append(GTK_BOX(form), image_label);

    ctx->image_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->image_entry), "ubuntu/24.04");
    gtk_widget_add_css_class(ctx->image_entry, "dialog-entry");
    gtk_box_append(GTK_BOX(form), ctx->image_entry);

    GtkWidget *hint = gtk_label_new("Available: ubuntu/24.04, debian/12, alpine/3.19");
    gtk_widget_add_css_class(hint, "dialog-hint");
    gtk_label_set_xalign(GTK_LABEL(hint), 0);
    gtk_box_append(GTK_BOX(form), hint);

    gtk_box_append(GTK_BOX(main_box), form);

    /* Button row */
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(btn_row, 20);
    gtk_widget_set_margin_end(btn_row, 20);
    gtk_widget_set_margin_bottom(btn_row, 20);
    gtk_widget_set_halign(btn_row, GTK_ALIGN_END);

    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    gtk_widget_add_css_class(cancel_btn, "btn-action");
    gtk_widget_add_css_class(cancel_btn, "btn-reboot");
    g_signal_connect_swapped(cancel_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(btn_row), cancel_btn);

    GtkWidget *create_btn = gtk_button_new_with_label("Create");
    gtk_widget_add_css_class(create_btn, "btn-action");
    gtk_widget_add_css_class(create_btn, "btn-create");
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_ct_create_confirm), ctx);
    gtk_box_append(GTK_BOX(btn_row), create_btn);

    gtk_box_append(GTK_BOX(main_box), btn_row);

    gtk_window_set_child(GTK_WINDOW(dialog), main_box);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ── Build Container Panel ─────────────────────────────────── */
GtkWidget *ui_build_ct_panel(AppData *app) {
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_start(header, 24);
    gtk_widget_set_margin_end(header, 24);
    gtk_widget_set_margin_top(header, 24);
    gtk_widget_set_margin_bottom(header, 12);

    GtkWidget *title = gtk_label_new("Containers");
    gtk_widget_add_css_class(title, "section-title");
    gtk_widget_set_hexpand(title, TRUE);
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(header), title);

    app->ct_status_label = gtk_label_new("0 running / 0 total");
    gtk_widget_add_css_class(app->ct_status_label, "section-subtitle");
    gtk_widget_set_valign(app->ct_status_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(header), app->ct_status_label);

    GtkWidget *create_btn = gtk_button_new_with_label("Create");
    gtk_widget_add_css_class(create_btn, "btn-action");
    gtk_widget_add_css_class(create_btn, "btn-create");
    g_signal_connect_swapped(create_btn, "clicked",
                             G_CALLBACK(ui_show_ct_create_dialog), app);
    gtk_box_append(GTK_BOX(header), create_btn);

    gtk_box_append(GTK_BOX(outer_box), header);

    /* Search and filter bar */
    GtkWidget *filter_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_add_css_class(filter_box, "filter-box");
    gtk_widget_set_margin_start(filter_box, 24);
    gtk_widget_set_margin_end(filter_box, 24);
    gtk_widget_set_margin_bottom(filter_box, 12);

    /* Search entry */
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_add_css_class(search_box, "search-box");
    gtk_widget_set_hexpand(search_box, TRUE);

    GtkWidget *search_icon = gtk_image_new_from_icon_name("system-search-symbolic");
    gtk_box_append(GTK_BOX(search_box), search_icon);

    GtkWidget *ct_search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ct_search_entry), "Search containers...");
    gtk_widget_add_css_class(ct_search_entry, "search-entry");
    gtk_widget_set_hexpand(ct_search_entry, TRUE);
    g_signal_connect(ct_search_entry, "changed", G_CALLBACK(on_ct_search_changed), app);
    gtk_box_append(GTK_BOX(search_box), ct_search_entry);

    gtk_box_append(GTK_BOX(filter_box), search_box);

    /* Filter buttons (grouped) */
    GtkWidget *ct_all_btn = gtk_toggle_button_new_with_label("All");
    gtk_widget_add_css_class(ct_all_btn, "filter-btn");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ct_all_btn), TRUE);
    g_signal_connect(ct_all_btn, "toggled", G_CALLBACK(on_ct_filter_all), app);
    gtk_box_append(GTK_BOX(filter_box), ct_all_btn);

    GtkWidget *ct_running_btn = gtk_toggle_button_new_with_label("Running");
    gtk_widget_add_css_class(ct_running_btn, "filter-btn");
    gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(ct_running_btn), GTK_TOGGLE_BUTTON(ct_all_btn));
    g_signal_connect(ct_running_btn, "toggled", G_CALLBACK(on_ct_filter_running), app);
    gtk_box_append(GTK_BOX(filter_box), ct_running_btn);

    GtkWidget *ct_stopped_btn = gtk_toggle_button_new_with_label("Stopped");
    gtk_widget_add_css_class(ct_stopped_btn, "filter-btn");
    gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(ct_stopped_btn), GTK_TOGGLE_BUTTON(ct_all_btn));
    g_signal_connect(ct_stopped_btn, "toggled", G_CALLBACK(on_ct_filter_stopped), app);
    gtk_box_append(GTK_BOX(filter_box), ct_stopped_btn);

    gtk_box_append(GTK_BOX(outer_box), filter_box);

    app->ct_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(app->ct_list_box, 24);
    gtk_widget_set_margin_end(app->ct_list_box, 24);
    gtk_widget_set_margin_bottom(app->ct_list_box, 24);
    gtk_box_append(GTK_BOX(outer_box), app->ct_list_box);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), outer_box);
    return scroll;
}

/* ── Check if container matches current filter ─────────────── */
static bool ct_matches_filter(AppData *app, CtInfo *ct) {
    switch (app->ct_filter) {
        case FILTER_RUNNING: if (ct->state != CT_STATE_RUNNING) return false; break;
        case FILTER_STOPPED: if (ct->state != CT_STATE_STOPPED) return false; break;
        case FILTER_PAUSED:  if (ct->state != CT_STATE_FROZEN) return false; break;
        default: break;
    }
    if (app->ct_search[0]) {
        if (!strcasestr(ct->name, app->ct_search)) return false;
    }
    return true;
}

void ui_refresh_ct_list(AppData *app) {
    if (!app->ct_list_box) return;

    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(app->ct_list_box)))
        gtk_box_remove(GTK_BOX(app->ct_list_box), child);

    if (!app->incus_available) {
        GtkWidget *msg = gtk_label_new(
            "Incus/LXD not detected. Install Incus and ensure the socket is accessible.");
        gtk_widget_add_css_class(msg, "section-subtitle");
        gtk_box_append(GTK_BOX(app->ct_list_box), msg);
    } else {
        int shown = 0;
        for (int i = 0; i < app->ct_count; i++) {
            if (ct_matches_filter(app, &app->containers[i])) {
                gtk_box_append(GTK_BOX(app->ct_list_box),
                               make_ct_row(app, &app->containers[i]));
                shown++;
            }
        }
        if (shown == 0) {
            const char *msg_text = app->ct_count == 0
                ? "No containers found. Create one to get started."
                : "No containers match the current filter.";
            GtkWidget *empty = gtk_label_new(msg_text);
            gtk_widget_add_css_class(empty, "section-subtitle");
            gtk_box_append(GTK_BOX(app->ct_list_box), empty);
        }
    }

    char status[64];
    snprintf(status, sizeof(status), "%d running / %d total",
             app->stats.running_containers, app->stats.total_containers);
    gtk_label_set_text(GTK_LABEL(app->ct_status_label), status);
}

/* ── Set container filter ──────────────────────────────────── */
void ui_set_ct_filter(AppData *app, FilterState filter, const char *search) {
    app->ct_filter = filter;
    if (search)
        strncpy(app->ct_search, search, sizeof(app->ct_search) - 1);
    else
        app->ct_search[0] = '\0';
    ui_refresh_ct_list(app);
}

/* ──────────────────────────────────────────────────────────── */
/* ── Log Panel ─────────────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

static void on_log_clear(GtkButton *btn, gpointer data) {
    (void)btn;
    AppData *app = data;
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(app->log_buffer, &start);
    gtk_text_buffer_get_end_iter(app->log_buffer, &end);
    gtk_text_buffer_delete(app->log_buffer, &start, &end);
    app_log(app, "INFO", "Log cleared");
}

static void on_log_export(GtkButton *btn, gpointer data) {
    (void)btn;
    AppData *app = data;

    char path[512];
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(path, sizeof(path), "%s/vmmanager-log-%04d%02d%02d-%02d%02d%02d.txt",
             home, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(app->log_buffer, &start);
    gtk_text_buffer_get_end_iter(app->log_buffer, &end);
    char *text = gtk_text_buffer_get_text(app->log_buffer, &start, &end, FALSE);

    GError *error = NULL;
    if (g_file_set_contents(path, text, -1, &error)) {
        app_log(app, "INFO", "Log exported to %s", path);
    } else {
        app_log(app, "ERROR", "Failed to export log: %s", error->message);
        g_error_free(error);
    }
    g_free(text);
}

GtkWidget *ui_build_log_panel(AppData *app) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);
    gtk_widget_set_margin_top(box, 24);
    gtk_widget_set_margin_bottom(box, 24);

    /* Header row with title and action buttons */
    GtkWidget *header_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_hexpand(title_box, TRUE);

    GtkWidget *title = gtk_label_new("Activity Log");
    gtk_widget_add_css_class(title, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(title_box), title);

    GtkWidget *subtitle = gtk_label_new("Application events and operation history");
    gtk_widget_add_css_class(subtitle, "section-subtitle");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0);
    gtk_box_append(GTK_BOX(title_box), subtitle);

    gtk_box_append(GTK_BOX(header_row), title_box);

    GtkWidget *export_btn = gtk_button_new_with_label("Export");
    gtk_widget_add_css_class(export_btn, "btn-action");
    gtk_widget_add_css_class(export_btn, "btn-start");
    gtk_widget_set_valign(export_btn, GTK_ALIGN_CENTER);
    g_signal_connect(export_btn, "clicked", G_CALLBACK(on_log_export), app);
    gtk_box_append(GTK_BOX(header_row), export_btn);

    GtkWidget *clear_btn = gtk_button_new_with_label("Clear");
    gtk_widget_add_css_class(clear_btn, "btn-action");
    gtk_widget_add_css_class(clear_btn, "btn-delete");
    gtk_widget_set_valign(clear_btn, GTK_ALIGN_CENTER);
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_log_clear), app);
    gtk_box_append(GTK_BOX(header_row), clear_btn);

    gtk_box_append(GTK_BOX(box), header_row);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);

    app->log_buffer = gtk_text_buffer_new(NULL);
    app->log_text_view = gtk_text_view_new_with_buffer(app->log_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->log_text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(app->log_text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->log_text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(app->log_text_view), TRUE);
    gtk_widget_add_css_class(app->log_text_view, "log-view");

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app->log_text_view);
    gtk_box_append(GTK_BOX(box), scroll);

    return box;
}

/* ──────────────────────────────────────────────────────────── */
/* ── Dashboard Refresh ─────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

void ui_refresh_dashboard(AppData *app) {
    sys_get_stats(app);

    double cpu = app->stats.cpu_usage_percent;
    if (cpu > 100.0) cpu = 100.0;
    if (cpu < 0.0)   cpu = 0.0;

    char buf[64];

    /* CPU */
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->dash_cpu_bar), cpu / 100.0);
    snprintf(buf, sizeof(buf), "%.1f%%", cpu);
    gtk_label_set_text(GTK_LABEL(app->dash_cpu_label), buf);

    /* RAM */
    double ram_pct = (app->stats.total_ram_mb > 0)
        ? (double)app->stats.used_ram_mb / app->stats.total_ram_mb : 0.0;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->dash_ram_bar), ram_pct);
    snprintf(buf, sizeof(buf), "%" PRId64 " / %" PRId64 " MB",
             app->stats.used_ram_mb, app->stats.total_ram_mb);
    gtk_label_set_text(GTK_LABEL(app->dash_ram_label), buf);

    /* Disk */
    double disk_pct = (app->stats.total_disk_mb > 0)
        ? (double)app->stats.used_disk_mb / app->stats.total_disk_mb : 0.0;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->dash_disk_bar), disk_pct);
    snprintf(buf, sizeof(buf), "%" PRId64 " / %" PRId64 " MB",
             app->stats.used_disk_mb, app->stats.total_disk_mb);
    gtk_label_set_text(GTK_LABEL(app->dash_disk_label), buf);

    /* Counts */
    snprintf(buf, sizeof(buf), "%d", app->stats.running_vms);
    gtk_label_set_text(GTK_LABEL(app->dash_vm_running_label), buf);
    snprintf(buf, sizeof(buf), "%d", app->stats.total_vms);
    gtk_label_set_text(GTK_LABEL(app->dash_vm_total_label), buf);

    snprintf(buf, sizeof(buf), "%d", app->stats.running_containers);
    gtk_label_set_text(GTK_LABEL(app->dash_ct_running_label), buf);
    snprintf(buf, sizeof(buf), "%d", app->stats.total_containers);
    gtk_label_set_text(GTK_LABEL(app->dash_ct_total_label), buf);
}

/* ── Refresh everything ────────────────────────────────────── */
void ui_refresh_all(AppData *app) {
    vm_backend_refresh(app);
    ct_backend_refresh(app);
    ui_refresh_dashboard(app);
    ui_refresh_vm_list(app);
    ui_refresh_ct_list(app);
}

gboolean ui_auto_refresh(gpointer data) {
    AppData *app = (AppData *)data;
    ui_refresh_dashboard(app);
    return G_SOURCE_CONTINUE;
}

/* ──────────────────────────────────────────────────────────── */
/* ── Sidebar + Navigation ──────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

static void on_nav_clicked(GtkButton *btn, gpointer data) {
    AppData *app = (AppData *)data;
    const char *page = g_object_get_data(G_OBJECT(btn), "page");

    gtk_stack_set_visible_child_name(GTK_STACK(app->main_stack), page);

    /* Refresh current panel - backend first, then UI */
    if (strcmp(page, "vm") == 0) {
        vm_backend_refresh(app);
        ui_refresh_vm_list(app);
    } else if (strcmp(page, "ct") == 0) {
        ct_backend_refresh(app);
        ui_refresh_ct_list(app);
    } else if (strcmp(page, "dashboard") == 0) {
        ui_refresh_dashboard(app);
    }
}

GtkWidget *ui_build_sidebar(AppData *app) {
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(sidebar, "sidebar");
    gtk_widget_set_size_request(sidebar, 200, -1);

    /* Logo/title */
    GtkWidget *title = gtk_label_new("VM MANAGER");
    gtk_widget_add_css_class(title, "sidebar-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(sidebar), title);

    /* Nav buttons - professional labels without emojis */
    const char *labels[] = { "Dashboard", "Virtual Machines",
                              "Containers", "Activity Log" };
    const char *pages[]  = { "dashboard", "vm", "ct", "log" };

    for (int i = 0; i < 4; i++) {
        GtkWidget *btn = gtk_button_new_with_label(labels[i]);
        gtk_widget_add_css_class(btn, "sidebar-btn");
        g_object_set_data(G_OBJECT(btn), "page", (gpointer)pages[i]);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_nav_clicked), app);
        gtk_box_append(GTK_BOX(sidebar), btn);
    }

    /* Spacer */
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_vexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(sidebar), spacer);

    /* Settings button */
    GtkWidget *settings_btn = gtk_button_new_with_label("Settings");
    gtk_widget_add_css_class(settings_btn, "sidebar-btn");
    g_signal_connect_swapped(settings_btn, "clicked",
                             G_CALLBACK(ui_show_settings_dialog), app);
    gtk_box_append(GTK_BOX(sidebar), settings_btn);

    /* Keyboard shortcut hints */
    GtkWidget *shortcuts = gtk_label_new("F5 Refresh | Ctrl+, Settings");
    gtk_widget_add_css_class(shortcuts, "sidebar-version");
    gtk_label_set_xalign(GTK_LABEL(shortcuts), 0);
    gtk_box_append(GTK_BOX(sidebar), shortcuts);

    /* Version */
    GtkWidget *ver = gtk_label_new("Version " APP_VERSION);
    gtk_widget_add_css_class(ver, "sidebar-version");
    gtk_label_set_xalign(GTK_LABEL(ver), 0);
    gtk_box_append(GTK_BOX(sidebar), ver);

    return sidebar;
}

/* ──────────────────────────────────────────────────────────── */
/* ── Build Main Window ─────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

void ui_build_window(AppData *app) {
    apply_css();

    app->window = gtk_application_window_new(app->app);
    gtk_window_set_title(GTK_WINDOW(app->window), APP_NAME);
    gtk_window_set_default_size(GTK_WINDOW(app->window), 1200, 750);

    /* Main horizontal layout: sidebar | content */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    /* Sidebar */
    gtk_box_append(GTK_BOX(hbox), ui_build_sidebar(app));

    /* Content stack */
    app->main_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app->main_stack),
                                  GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(app->main_stack), 200);
    gtk_widget_set_hexpand(app->main_stack, TRUE);
    gtk_widget_set_vexpand(app->main_stack, TRUE);

    gtk_stack_add_named(GTK_STACK(app->main_stack),
                        ui_build_dashboard(app), "dashboard");
    gtk_stack_add_named(GTK_STACK(app->main_stack),
                        ui_build_vm_panel(app), "vm");
    gtk_stack_add_named(GTK_STACK(app->main_stack),
                        ui_build_ct_panel(app), "ct");
    gtk_stack_add_named(GTK_STACK(app->main_stack),
                        ui_build_log_panel(app), "log");

    gtk_box_append(GTK_BOX(hbox), app->main_stack);
    gtk_window_set_child(GTK_WINDOW(app->window), hbox);

    /* Setup keyboard shortcuts */
    ui_setup_shortcuts(app);

    /* Load settings */
    settings_load(app);

    /* Initial data load */
    vm_backend_refresh(app);
    ct_backend_refresh(app);
    ui_refresh_dashboard(app);
    ui_refresh_vm_list(app);
    ui_refresh_ct_list(app);

    /* Auto-refresh timer for dashboard (use settings interval) */
    int interval = app->settings.loaded ? app->settings.refresh_interval_ms : REFRESH_INTERVAL;
    if (app->settings.auto_refresh) {
        app->refresh_timer = g_timeout_add(interval, ui_auto_refresh, app);
    }

    gtk_window_present(GTK_WINDOW(app->window));
}
