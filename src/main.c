/*
 * main.c - VMManager entry point
 *
 * A native GTK4 desktop application for managing QEMU/KVM virtual machines
 * and Incus/LXD containers from a single unified interface.
 *
 * Build:
 *   gcc -o vmmanager src/main.c src/ui/window.c src/backend/vm_manager.c \
 *       src/backend/ct_manager.c src/utils/system_info.c \
 *       $(pkg-config --cflags --libs gtk4 libvirt libcurl json-glib-1.0) \
 *       -Iinclude -Wall -O2
 *
 * Run:
 *   ./vmmanager
 *
 * Author: Dharun (TripleETech)
 * License: GPL-3.0
 */

#include "vmmanager.h"

/* ── Logging ───────────────────────────────────────────────── */
void app_log(AppData *app, const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *message = g_strdup_vprintf(fmt, args);
    va_end(args);

    if (!message) return;

    /* Timestamp */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", t);

    /* Print to stderr */
    fprintf(stderr, "[%s] [%s] %s\n", timestamp, level, message);

    /* Append to log buffer if available */
    if (app && app->log_buffer) {
        char *line = g_strdup_printf("[%s] [%s] %s\n", timestamp, level, message);

        GtkTextIter end;
        gtk_text_buffer_get_end_iter(app->log_buffer, &end);
        gtk_text_buffer_insert(app->log_buffer, &end, line, -1);

        /* Auto-scroll to bottom */
        if (app->log_text_view) {
            gtk_text_buffer_get_end_iter(app->log_buffer, &end);
            GtkTextMark *mark = gtk_text_buffer_get_mark(app->log_buffer, "end_mark");
            if (!mark) {
                mark = gtk_text_buffer_create_mark(app->log_buffer, "end_mark", &end, FALSE);
            } else {
                gtk_text_buffer_move_mark(app->log_buffer, mark, &end);
            }
            gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(app->log_text_view), mark);
        }
        
        g_free(line);
    }
    
    g_free(message);
}

/* ── Application activate ──────────────────────────────────── */
static void on_activate(GtkApplication *gtk_app, gpointer user_data) {
    (void)gtk_app;
    AppData *app = (AppData *)user_data;

    /* Build the UI first so log buffer is available */
    ui_build_window(app);

    app_log(app, "INFO", "VMManager %s started", APP_VERSION);

    /* Connect backends */
    if (vm_backend_connect(app)) {
        app_log(app, "INFO", "libvirt backend ready");
    } else {
        app_log(app, "WARN", "libvirt connection failed — VM management disabled");
        app_log(app, "INFO", "Ensure libvirtd is running: sudo systemctl start libvirtd");
    }

    if (ct_backend_connect(app)) {
        app_log(app, "INFO", "Container backend ready");
    } else {
        app_log(app, "WARN", "No container socket found — container management disabled");
        app_log(app, "INFO", "Install Incus: https://linuxcontainers.org/incus/");
    }

    /* Prime the CPU reader (first read is always 0) */
    sys_get_cpu_usage();

    /* Full refresh */
    ui_refresh_all(app);

    app_log(app, "INFO", "Ready. Found %d VMs, %d containers.",
            app->vm_count, app->ct_count);
}

/* ── Cleanup on shutdown ───────────────────────────────────── */
static void on_shutdown(GtkApplication *gtk_app, gpointer user_data) {
    (void)gtk_app;
    AppData *app = (AppData *)user_data;

    if (app->refresh_timer) {
        g_source_remove(app->refresh_timer);
        app->refresh_timer = 0;
    }

    vm_backend_disconnect(app);

    if (app->vms) { free(app->vms); app->vms = NULL; }
    if (app->containers) { free(app->containers); app->containers = NULL; }

    curl_global_cleanup();
}

/* ── Main ──────────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    AppData app = {0};

    app.app = gtk_application_new(APP_ID, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app.app, "activate", G_CALLBACK(on_activate), &app);
    g_signal_connect(app.app, "shutdown", G_CALLBACK(on_shutdown), &app);

    int status = g_application_run(G_APPLICATION(app.app), argc, argv);

    g_object_unref(app.app);
    return status;
}
