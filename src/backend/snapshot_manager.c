/*
 * snapshot_manager.c - VM snapshot management for VMManager
 */

#include "../include/vmmanager.h"

/* ── Create Snapshot ────────────────────────────────────────── */
bool vm_snapshot_create(AppData *app, const char *vm_name, const char *snapshot_name) {
    if (!app->virt_conn) {
        app_log(app, "ERROR", "Cannot create snapshot: libvirt not connected");
        return false;
    }
    
    if (!vm_name || !snapshot_name) {
        app_log(app, "ERROR", "VM name and snapshot name required");
        return false;
    }
    
    virDomainPtr dom = virDomainLookupByName(app->virt_conn, vm_name);
    if (!dom) {
        app_log(app, "ERROR", "VM '%s' not found", vm_name);
        return false;
    }
    
    /* Build snapshot XML */
    GString *xml = g_string_new(NULL);
    g_string_append(xml, "<domainsnapshot>\n");
    g_string_append_printf(xml, "  <name>%s</name>\n", snapshot_name);
    g_string_append_printf(xml, "  <description>Created by VMManager</description>\n");
    g_string_append(xml, "</domainsnapshot>\n");
    
    virDomainSnapshotPtr snap = virDomainSnapshotCreateXML(
        dom, xml->str, VIR_DOMAIN_SNAPSHOT_CREATE_DISK_ONLY);
    
    g_string_free(xml, TRUE);
    virDomainFree(dom);
    
    if (!snap) {
        virErrorPtr err = virGetLastError();
        app_log(app, "ERROR", "Failed to create snapshot '%s': %s", 
                snapshot_name, err ? err->message : "Unknown error");
        return false;
    }
    
    virDomainSnapshotFree(snap);
    app_log(app, "INFO", "Created snapshot '%s' for VM '%s'", snapshot_name, vm_name);
    return true;
}

/* ── Revert to Snapshot ─────────────────────────────────────── */
bool vm_snapshot_revert(AppData *app, const char *vm_name, const char *snapshot_name) {
    if (!app->virt_conn) {
        app_log(app, "ERROR", "Cannot revert snapshot: libvirt not connected");
        return false;
    }
    
    virDomainPtr dom = virDomainLookupByName(app->virt_conn, vm_name);
    if (!dom) {
        app_log(app, "ERROR", "VM '%s' not found", vm_name);
        return false;
    }
    
    virDomainSnapshotPtr snap = virDomainSnapshotLookupByName(dom, snapshot_name, 0);
    if (!snap) {
        app_log(app, "ERROR", "Snapshot '%s' not found for VM '%s'", snapshot_name, vm_name);
        virDomainFree(dom);
        return false;
    }
    
    int ret = virDomainRevertToSnapshot(snap, 0);
    
    virDomainSnapshotFree(snap);
    virDomainFree(dom);
    
    if (ret < 0) {
        virErrorPtr err = virGetLastError();
        app_log(app, "ERROR", "Failed to revert to snapshot '%s': %s",
                snapshot_name, err ? err->message : "Unknown error");
        return false;
    }
    
    app_log(app, "INFO", "Reverted VM '%s' to snapshot '%s'", vm_name, snapshot_name);
    return true;
}

/* ── Delete Snapshot ─────────────────────────────────────────── */
bool vm_snapshot_delete(AppData *app, const char *vm_name, const char *snapshot_name) {
    if (!app->virt_conn) {
        app_log(app, "ERROR", "Cannot delete snapshot: libvirt not connected");
        return false;
    }
    
    virDomainPtr dom = virDomainLookupByName(app->virt_conn, vm_name);
    if (!dom) {
        app_log(app, "ERROR", "VM '%s' not found", vm_name);
        return false;
    }
    
    virDomainSnapshotPtr snap = virDomainSnapshotLookupByName(dom, snapshot_name, 0);
    if (!snap) {
        app_log(app, "ERROR", "Snapshot '%s' not found for VM '%s'", snapshot_name, vm_name);
        virDomainFree(dom);
        return false;
    }
    
    int ret = virDomainSnapshotDelete(snap, 0);
    
    virDomainSnapshotFree(snap);
    virDomainFree(dom);
    
    if (ret < 0) {
        virErrorPtr err = virGetLastError();
        app_log(app, "ERROR", "Failed to delete snapshot '%s': %s",
                snapshot_name, err ? err->message : "Unknown error");
        return false;
    }
    
    app_log(app, "INFO", "Deleted snapshot '%s' for VM '%s'", snapshot_name, vm_name);
    return true;
}

/* ── List Snapshots ──────────────────────────────────────────── */
VmSnapshot *vm_snapshot_list(AppData *app, const char *vm_name, int *count) {
    *count = 0;
    
    if (!app->virt_conn) {
        app_log(app, "ERROR", "Cannot list snapshots: libvirt not connected");
        return NULL;
    }
    
    virDomainPtr dom = virDomainLookupByName(app->virt_conn, vm_name);
    if (!dom) {
        app_log(app, "ERROR", "VM '%s' not found", vm_name);
        return NULL;
    }
    
    /* Get number of snapshots */
    int num = virDomainSnapshotNum(dom, 0);
    if (num <= 0) {
        virDomainFree(dom);
        return NULL;
    }
    
    /* Get snapshot names */
    char **names = calloc(num, sizeof(char *));
    if (!names) {
        virDomainFree(dom);
        return NULL;
    }
    
    num = virDomainSnapshotListNames(dom, names, num, 0);
    if (num < 0) {
        free(names);
        virDomainFree(dom);
        return NULL;
    }
    
    VmSnapshot *snapshots = calloc(num, sizeof(VmSnapshot));
    if (!snapshots) {
        for (int i = 0; i < num; i++) free(names[i]);
        free(names);
        virDomainFree(dom);
        return NULL;
    }
    
    for (int i = 0; i < num; i++) {
        strncpy(snapshots[i].name, names[i], sizeof(snapshots[i].name) - 1);
        
        /* Get snapshot info via XML */
        virDomainSnapshotPtr snap = virDomainSnapshotLookupByName(dom, names[i], 0);
        if (snap) {
            char *xml_desc = virDomainSnapshotGetXMLDesc(snap, 0);
            if (xml_desc) {
                /* Parse state from XML */
                strncpy(snapshots[i].state, "Unknown", sizeof(snapshots[i].state) - 1);
                if (strstr(xml_desc, "<state>running</state>"))
                    strncpy(snapshots[i].state, "Running", sizeof(snapshots[i].state) - 1);
                else if (strstr(xml_desc, "<state>shutoff</state>"))
                    strncpy(snapshots[i].state, "Stopped", sizeof(snapshots[i].state) - 1);
                else if (strstr(xml_desc, "<state>paused</state>"))
                    strncpy(snapshots[i].state, "Paused", sizeof(snapshots[i].state) - 1);

                /* Parse creation time from XML */
                char *time_start = strstr(xml_desc, "<creationTime>");
                if (time_start) {
                    time_start += 14; /* Skip tag */
                    char *time_end = strstr(time_start, "</creationTime>");
                    if (time_end) {
                        *time_end = '\0';
                        time_t timestamp = atol(time_start);
                        struct tm *t = localtime(&timestamp);
                        strftime(snapshots[i].created_at, sizeof(snapshots[i].created_at),
                                "%Y-%m-%d %H:%M:%S", t);
                    }
                }
                free(xml_desc);
            }
            
            virDomainSnapshotFree(snap);
        }
        
        free(names[i]);
    }
    
    free(names);
    virDomainFree(dom);
    
    *count = num;
    return snapshots;
}

/* ── Snapshot Dialog ─────────────────────────────────────────── */
typedef struct {
    AppData    *app;
    char        vm_name[256];
    GtkWidget  *dialog;
    GtkWidget  *list_box;
    GtkWidget  *name_entry;
} SnapshotCtx;

typedef struct {
    SnapshotCtx *sctx;
    char         snap_name[128];
} SnapAction;

static void refresh_snapshot_list(SnapshotCtx *ctx);

static void snap_action_free(gpointer data, GClosure *closure) {
    (void)closure;
    g_free(data);
}

static void on_snapshot_revert(GtkButton *btn, gpointer data) {
    (void)btn;
    SnapAction *sa = data;
    if (vm_snapshot_revert(sa->sctx->app, sa->sctx->vm_name, sa->snap_name)) {
        refresh_snapshot_list(sa->sctx);
    }
}

static void on_snapshot_delete(GtkButton *btn, gpointer data) {
    (void)btn;
    SnapAction *sa = data;
    if (vm_snapshot_delete(sa->sctx->app, sa->sctx->vm_name, sa->snap_name)) {
        refresh_snapshot_list(sa->sctx);
    }
}

static void refresh_snapshot_list(SnapshotCtx *ctx) {
    /* Clear existing list */
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(ctx->list_box)))
        gtk_box_remove(GTK_BOX(ctx->list_box), child);

    /* Get snapshots */
    int count = 0;
    VmSnapshot *snapshots = vm_snapshot_list(ctx->app, ctx->vm_name, &count);

    if (count == 0) {
        GtkWidget *empty = gtk_label_new("No snapshots found");
        gtk_widget_add_css_class(empty, "section-subtitle");
        gtk_box_append(GTK_BOX(ctx->list_box), empty);
        return;
    }

    for (int i = 0; i < count; i++) {
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        gtk_widget_add_css_class(row, "resource-row");

        /* Info */
        GtkWidget *info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_widget_set_hexpand(info, TRUE);

        GtkWidget *name = gtk_label_new(snapshots[i].name);
        gtk_widget_add_css_class(name, "resource-name");
        gtk_label_set_xalign(GTK_LABEL(name), 0);
        gtk_box_append(GTK_BOX(info), name);

        char detail[128];
        snprintf(detail, sizeof(detail), "%s • %s",
                 snapshots[i].state, snapshots[i].created_at);
        GtkWidget *detail_lbl = gtk_label_new(detail);
        gtk_widget_add_css_class(detail_lbl, "resource-detail");
        gtk_label_set_xalign(GTK_LABEL(detail_lbl), 0);
        gtk_box_append(GTK_BOX(info), detail_lbl);

        gtk_box_append(GTK_BOX(row), info);

        /* Actions */
        GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

        SnapAction *revert_data = g_new0(SnapAction, 1);
        revert_data->sctx = ctx;
        strncpy(revert_data->snap_name, snapshots[i].name, sizeof(revert_data->snap_name) - 1);

        GtkWidget *revert_btn = gtk_button_new_with_label("Revert");
        gtk_widget_add_css_class(revert_btn, "btn-action");
        gtk_widget_add_css_class(revert_btn, "btn-start");
        g_signal_connect_data(revert_btn, "clicked",
            G_CALLBACK(on_snapshot_revert), revert_data, (GClosureNotify)snap_action_free, 0);
        gtk_box_append(GTK_BOX(btn_box), revert_btn);

        SnapAction *delete_data = g_new0(SnapAction, 1);
        delete_data->sctx = ctx;
        strncpy(delete_data->snap_name, snapshots[i].name, sizeof(delete_data->snap_name) - 1);

        GtkWidget *delete_btn = gtk_button_new_with_label("Delete");
        gtk_widget_add_css_class(delete_btn, "btn-action");
        gtk_widget_add_css_class(delete_btn, "btn-delete");
        g_signal_connect_data(delete_btn, "clicked",
            G_CALLBACK(on_snapshot_delete), delete_data, (GClosureNotify)snap_action_free, 0);
        gtk_box_append(GTK_BOX(btn_box), delete_btn);

        gtk_box_append(GTK_BOX(row), btn_box);
        gtk_box_append(GTK_BOX(ctx->list_box), row);
    }

    free(snapshots);
}

static void on_create_snapshot(GtkButton *btn, gpointer data) {
    (void)btn;
    SnapshotCtx *ctx = data;
    
    const char *name = gtk_editable_get_text(GTK_EDITABLE(ctx->name_entry));
    if (strlen(name) == 0) {
        app_log(ctx->app, "WARN", "Snapshot name is required");
        return;
    }
    
    if (vm_snapshot_create(ctx->app, ctx->vm_name, name)) {
        gtk_editable_set_text(GTK_EDITABLE(ctx->name_entry), "");
        refresh_snapshot_list(ctx);
    }
}

void ui_show_snapshot_dialog(AppData *app, const char *vm_name) {
    SnapshotCtx *ctx = g_new0(SnapshotCtx, 1);
    ctx->app = app;
    strncpy(ctx->vm_name, vm_name, sizeof(ctx->vm_name) - 1);
    
    GtkWidget *dialog = gtk_window_new();
    ctx->dialog = dialog;
    gtk_window_set_title(GTK_WINDOW(dialog), "VM Snapshots");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(main_box, 24);
    gtk_widget_set_margin_end(main_box, 24);
    gtk_widget_set_margin_top(main_box, 24);
    gtk_widget_set_margin_bottom(main_box, 24);
    
    /* Header */
    char title[256];
    snprintf(title, sizeof(title), "Snapshots: %s", vm_name);
    GtkWidget *title_lbl = gtk_label_new(title);
    gtk_widget_add_css_class(title_lbl, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title_lbl), 0);
    gtk_box_append(GTK_BOX(main_box), title_lbl);
    
    /* Create new snapshot */
    GtkWidget *create_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    
    ctx->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->name_entry), "New snapshot name");
    gtk_widget_set_hexpand(ctx->name_entry, TRUE);
    gtk_box_append(GTK_BOX(create_box), ctx->name_entry);
    
    GtkWidget *create_btn = gtk_button_new_with_label("Create");
    gtk_widget_add_css_class(create_btn, "btn-action");
    gtk_widget_add_css_class(create_btn, "btn-create");
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_snapshot), ctx);
    gtk_box_append(GTK_BOX(create_box), create_btn);
    
    gtk_box_append(GTK_BOX(main_box), create_box);
    
    /* Snapshot list */
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    
    ctx->list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), ctx->list_box);
    gtk_box_append(GTK_BOX(main_box), scroll);
    
    /* Close button */
    GtkWidget *close_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(close_row, GTK_ALIGN_END);
    
    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    gtk_widget_add_css_class(close_btn, "btn-action");
    gtk_widget_add_css_class(close_btn, "btn-stop");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(close_row), close_btn);
    
    gtk_box_append(GTK_BOX(main_box), close_row);
    
    gtk_window_set_child(GTK_WINDOW(dialog), main_box);
    gtk_window_present(GTK_WINDOW(dialog));
    
    /* Load snapshots */
    refresh_snapshot_list(ctx);
}
