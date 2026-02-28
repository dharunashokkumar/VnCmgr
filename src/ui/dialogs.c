/*
 * dialogs.c - Dialog boxes, loading indicators, and UI utilities for VMManager
 */

#include "../include/vmmanager.h"

/* ── Additional CSS for dialogs and loading ─────────────────── */
static const char *DIALOG_CSS =
    "/* Loading overlay */\n"
    ".loading-overlay { background-color: rgba(0, 0, 0, 0.5); }\n"
    ".loading-box { background-color: #ffffff; border-radius: 8px; padding: 24px; }\n"
    ".loading-label { font-size: 14px; color: #42526e; margin-top: 12px; }\n"
    "\n"
    "/* Error dialog */\n"
    ".error-title { font-size: 16px; font-weight: 600; color: #bf2600; }\n"
    ".error-message { font-size: 13px; color: #42526e; }\n"
    ".error-suggestion { font-size: 12px; color: #0066cc; background-color: #e3fcef; "
    "                    border-radius: 4px; padding: 8px 12px; }\n"
    "\n"
    "/* Confirm dialog */\n"
    ".confirm-title { font-size: 16px; font-weight: 600; color: #212121; }\n"
    ".confirm-message { font-size: 13px; color: #42526e; }\n"
    ".confirm-warning { font-size: 12px; color: #bf2600; background-color: #ffebe6; "
    "                   border-radius: 4px; padding: 8px 12px; }\n"
    "\n"
    "/* Details dialog */\n"
    ".details-header { font-size: 18px; font-weight: 600; color: #212121; }\n"
    ".details-section { font-size: 12px; color: #707070; font-weight: 500; "
    "                   text-transform: uppercase; margin-top: 16px; }\n"
    ".details-row { padding: 8px 0; border-bottom: 1px solid #e0e0e0; }\n"
    ".details-label { font-size: 12px; color: #707070; }\n"
    ".details-value { font-size: 13px; color: #212121; font-weight: 500; }\n"
    "\n"
    "/* Search entry */\n"
    ".search-entry { min-height: 32px; padding: 4px 12px; border-radius: 4px; "
    "                border: 1px solid #dfe1e6; background-color: #fafafa; }\n"
    ".search-entry:focus { border-color: #0066cc; background-color: #ffffff; }\n"
    "\n"
    "/* Filter buttons */\n"
    ".filter-btn { padding: 4px 12px; border-radius: 4px; font-size: 12px; "
    "              background-color: #f4f5f7; color: #42526e; }\n"
    ".filter-btn:checked { background-color: #0066cc; color: #ffffff; }\n"
    "\n"
    "/* Settings dialog */\n"
    ".settings-section { font-size: 14px; font-weight: 600; color: #212121; "
    "                    margin-top: 16px; margin-bottom: 8px; }\n"
    ".settings-row { padding: 8px 0; }\n"
    ".settings-label { font-size: 13px; color: #42526e; }\n"
    ".settings-hint { font-size: 11px; color: #707070; }\n";

/* ── Apply additional CSS ───────────────────────────────────── */
static void apply_dialog_css(void) {
    static bool applied = false;
    if (applied) return;
    applied = true;
    
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, DIALOG_CSS);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

/* ──────────────────────────────────────────────────────────── */
/* ── Loading Indicator ─────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

LoadingIndicator *ui_loading_start(AppData *app, GtkWidget *parent, const char *message) {
    (void)app;
    apply_dialog_css();
    
    LoadingIndicator *loader = g_new0(LoadingIndicator, 1);
    if (!loader) return NULL;
    
    loader->parent = parent;
    loader->active = true;
    
    /* Create overlay */
    loader->overlay = gtk_overlay_new();
    
    /* Create loading box */
    loader->spinner = gtk_spinner_new();
    gtk_widget_set_size_request(loader->spinner, 32, 32);
    gtk_spinner_set_spinning(GTK_SPINNER(loader->spinner), TRUE);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(box, "loading-box");
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    
    gtk_box_append(GTK_BOX(box), loader->spinner);
    
    loader->label = gtk_label_new(message ? message : "Loading...");
    gtk_widget_add_css_class(loader->label, "loading-label");
    gtk_box_append(GTK_BOX(box), loader->label);
    
    /* Create overlay background */
    GtkWidget *background = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(background, "loading-overlay");
    gtk_widget_set_hexpand(background, TRUE);
    gtk_widget_set_vexpand(background, TRUE);
    
    gtk_overlay_add_overlay(GTK_OVERLAY(loader->overlay), background);
    gtk_overlay_add_overlay(GTK_OVERLAY(loader->overlay), box);
    
    /* Add overlay to parent */
    if (parent && GTK_IS_BOX(parent)) {
        /* Insert at the beginning */
        gtk_box_prepend(GTK_BOX(parent), loader->overlay);
        gtk_widget_set_hexpand(loader->overlay, TRUE);
        gtk_widget_set_vexpand(loader->overlay, TRUE);
    }
    
    return loader;
}

void ui_loading_end(LoadingIndicator *loader) {
    if (!loader) return;
    
    loader->active = false;
    
    if (loader->spinner) {
        gtk_spinner_set_spinning(GTK_SPINNER(loader->spinner), FALSE);
    }
    
    if (loader->overlay) {
        /* Remove overlay from parent */
        GtkWidget *parent = gtk_widget_get_parent(loader->overlay);
        if (parent && GTK_IS_BOX(parent)) {
            gtk_box_remove(GTK_BOX(parent), loader->overlay);
        }
    }
    
    g_free(loader);
}

void ui_loading_update(LoadingIndicator *loader, const char *message) {
    if (!loader || !loader->active || !loader->label) return;
    gtk_label_set_text(GTK_LABEL(loader->label), message);
}

/* ──────────────────────────────────────────────────────────── */
/* ── Error Dialog ──────────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

void ui_show_error_dialog(AppData *app, const char *title, 
                          const char *message, const char *suggestion) {
    apply_dialog_css();
    
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title ? title : "Error");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 200);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(main_box, 24);
    gtk_widget_set_margin_end(main_box, 24);
    gtk_widget_set_margin_top(main_box, 24);
    gtk_widget_set_margin_bottom(main_box, 24);
    
    /* Title */
    GtkWidget *title_lbl = gtk_label_new(title ? title : "An error occurred");
    gtk_widget_add_css_class(title_lbl, "error-title");
    gtk_label_set_xalign(GTK_LABEL(title_lbl), 0);
    gtk_box_append(GTK_BOX(main_box), title_lbl);
    
    /* Message */
    GtkWidget *msg_lbl = gtk_label_new(message);
    gtk_widget_add_css_class(msg_lbl, "error-message");
    gtk_label_set_xalign(GTK_LABEL(msg_lbl), 0);
    gtk_label_set_wrap(GTK_LABEL(msg_lbl), TRUE);
    gtk_box_append(GTK_BOX(main_box), msg_lbl);
    
    /* Suggestion */
    if (suggestion && strlen(suggestion) > 0) {
        GtkWidget *suggestion_lbl = gtk_label_new(suggestion);
        gtk_widget_add_css_class(suggestion_lbl, "error-suggestion");
        gtk_label_set_xalign(GTK_LABEL(suggestion_lbl), 0);
        gtk_label_set_wrap(GTK_LABEL(suggestion_lbl), TRUE);
        gtk_box_append(GTK_BOX(main_box), suggestion_lbl);
    }
    
    /* Close button */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    
    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    gtk_widget_add_css_class(close_btn, "btn-action");
    gtk_widget_add_css_class(close_btn, "btn-reboot");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(btn_box), close_btn);
    
    gtk_box_append(GTK_BOX(main_box), btn_box);
    
    gtk_window_set_child(GTK_WINDOW(dialog), main_box);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ──────────────────────────────────────────────────────────── */
/* ── Info Dialog ───────────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

void ui_show_info_dialog(AppData *app, const char *title, const char *message) {
    apply_dialog_css();
    
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), title ? title : "Information");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 350, 150);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(main_box, 24);
    gtk_widget_set_margin_end(main_box, 24);
    gtk_widget_set_margin_top(main_box, 24);
    gtk_widget_set_margin_bottom(main_box, 24);
    
    /* Title */
    GtkWidget *title_lbl = gtk_label_new(title);
    gtk_widget_add_css_class(title_lbl, "section-title");
    gtk_label_set_xalign(GTK_LABEL(title_lbl), 0);
    gtk_box_append(GTK_BOX(main_box), title_lbl);
    
    /* Message */
    GtkWidget *msg_lbl = gtk_label_new(message);
    gtk_widget_add_css_class(msg_lbl, "section-subtitle");
    gtk_label_set_xalign(GTK_LABEL(msg_lbl), 0);
    gtk_label_set_wrap(GTK_LABEL(msg_lbl), TRUE);
    gtk_box_append(GTK_BOX(main_box), msg_lbl);
    
    /* OK button */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    
    GtkWidget *ok_btn = gtk_button_new_with_label("OK");
    gtk_widget_add_css_class(ok_btn, "btn-action");
    gtk_widget_add_css_class(ok_btn, "btn-create");
    g_signal_connect_swapped(ok_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(btn_box), ok_btn);
    
    gtk_box_append(GTK_BOX(main_box), btn_box);
    
    gtk_window_set_child(GTK_WINDOW(dialog), main_box);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ──────────────────────────────────────────────────────────── */
/* ── Confirmation Dialog ───────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

typedef struct {
    GtkWidget  *dialog;
    GCallback   on_confirm;
    gpointer    user_data;
    AppData    *app;
} ConfirmCtx;

static void on_confirm_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    ConfirmCtx *ctx = data;
    
    if (ctx->on_confirm) {
        ((void(*)(gpointer))ctx->on_confirm)(ctx->user_data);
    }
    
    gtk_window_destroy(GTK_WINDOW(ctx->dialog));
    g_free(ctx);
}

static void on_cancel_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    ConfirmCtx *ctx = data;
    gtk_window_destroy(GTK_WINDOW(ctx->dialog));
    g_free(ctx);
}

void ui_show_confirm_dialog(AppData *app, const char *title,
                            const char *message, GCallback on_confirm,
                            gpointer user_data) {
    apply_dialog_css();
    
    ConfirmCtx *ctx = g_new0(ConfirmCtx, 1);
    ctx->app = app;
    ctx->on_confirm = on_confirm;
    ctx->user_data = user_data;
    
    GtkWidget *dialog = gtk_window_new();
    ctx->dialog = dialog;
    gtk_window_set_title(GTK_WINDOW(dialog), title ? title : "Confirm");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 180);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(main_box, 24);
    gtk_widget_set_margin_end(main_box, 24);
    gtk_widget_set_margin_top(main_box, 24);
    gtk_widget_set_margin_bottom(main_box, 24);
    
    /* Title */
    GtkWidget *title_lbl = gtk_label_new(title);
    gtk_widget_add_css_class(title_lbl, "confirm-title");
    gtk_label_set_xalign(GTK_LABEL(title_lbl), 0);
    gtk_box_append(GTK_BOX(main_box), title_lbl);
    
    /* Message */
    GtkWidget *msg_lbl = gtk_label_new(message);
    gtk_widget_add_css_class(msg_lbl, "confirm-message");
    gtk_label_set_xalign(GTK_LABEL(msg_lbl), 0);
    gtk_label_set_wrap(GTK_LABEL(msg_lbl), TRUE);
    gtk_box_append(GTK_BOX(main_box), msg_lbl);
    
    /* Warning */
    GtkWidget *warning_lbl = gtk_label_new("This action cannot be undone.");
    gtk_widget_add_css_class(warning_lbl, "confirm-warning");
    gtk_label_set_xalign(GTK_LABEL(warning_lbl), 0);
    gtk_box_append(GTK_BOX(main_box), warning_lbl);
    
    /* Buttons */
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    
    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    gtk_widget_add_css_class(cancel_btn, "btn-action");
    gtk_widget_add_css_class(cancel_btn, "btn-reboot");
    g_signal_connect(cancel_btn, "clicked", G_CALLBACK(on_cancel_clicked), ctx);
    gtk_box_append(GTK_BOX(btn_box), cancel_btn);
    
    GtkWidget *confirm_btn = gtk_button_new_with_label("Confirm");
    gtk_widget_add_css_class(confirm_btn, "btn-action");
    gtk_widget_add_css_class(confirm_btn, "btn-delete");
    g_signal_connect(confirm_btn, "clicked", G_CALLBACK(on_confirm_clicked), ctx);
    gtk_box_append(GTK_BOX(btn_box), confirm_btn);
    
    gtk_box_append(GTK_BOX(main_box), btn_box);
    
    gtk_window_set_child(GTK_WINDOW(dialog), main_box);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ──────────────────────────────────────────────────────────── */
/* ── VM Details Dialog ─────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

static void add_detail_row(GtkWidget *box, const char *label, const char *value) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(row, "details-row");
    
    GtkWidget *lbl = gtk_label_new(label);
    gtk_widget_add_css_class(lbl, "details-label");
    gtk_label_set_xalign(GTK_LABEL(lbl), 0);
    gtk_widget_set_size_request(lbl, 120, -1);
    gtk_box_append(GTK_BOX(row), lbl);
    
    GtkWidget *val = gtk_label_new(value);
    gtk_widget_add_css_class(val, "details-value");
    gtk_label_set_xalign(GTK_LABEL(val), 0);
    gtk_box_append(GTK_BOX(row), val);
    
    gtk_box_append(GTK_BOX(box), row);
}

static void add_section_header(GtkWidget *box, const char *title) {
    GtkWidget *lbl = gtk_label_new(title);
    gtk_widget_add_css_class(lbl, "details-section");
    gtk_label_set_xalign(GTK_LABEL(lbl), 0);
    gtk_box_append(GTK_BOX(box), lbl);
}

void ui_show_vm_details_dialog(AppData *app, VmInfo *vm) {
    if (!vm) return;
    apply_dialog_css();
    
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), vm->name);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 450);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(main_box, 24);
    gtk_widget_set_margin_end(main_box, 24);
    gtk_widget_set_margin_top(main_box, 24);
    gtk_widget_set_margin_bottom(main_box, 24);
    
    /* Header */
    GtkWidget *header = gtk_label_new(vm->name);
    gtk_widget_add_css_class(header, "details-header");
    gtk_label_set_xalign(GTK_LABEL(header), 0);
    gtk_box_append(GTK_BOX(main_box), header);
    
    /* Status badge */
    GtkWidget *badge = gtk_label_new(vm_state_str(vm->state));
    gtk_widget_add_css_class(badge, "state-badge");
    gtk_widget_add_css_class(badge, vm_state_css(vm->state));
    gtk_label_set_xalign(GTK_LABEL(badge), 0);
    gtk_box_append(GTK_BOX(main_box), badge);
    
    /* Basic info section */
    add_section_header(main_box, "BASIC INFORMATION");
    
    char buf[256];
    
    add_detail_row(main_box, "UUID", vm->uuid);
    add_detail_row(main_box, "State", vm_state_str(vm->state));
    snprintf(buf, sizeof(buf), "%s", vm->autostart ? "Enabled" : "Disabled");
    add_detail_row(main_box, "Autostart", buf);
    
    /* Resources section */
    add_section_header(main_box, "RESOURCES");
    
    snprintf(buf, sizeof(buf), "%d", vm->vcpus);
    add_detail_row(main_box, "vCPUs", buf);
    
    snprintf(buf, sizeof(buf), "%lu MB", vm->max_mem_kb / 1024);
    add_detail_row(main_box, "Max Memory", buf);
    
    if (vm->state == VM_STATE_RUNNING) {
        snprintf(buf, sizeof(buf), "%lu MB", vm->used_mem_kb / 1024);
        add_detail_row(main_box, "Used Memory", buf);
    }
    
    /* Actions section */
    add_section_header(main_box, "ACTIONS");
    
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    
    if (vm->state == VM_STATE_RUNNING) {
        GtkWidget *console_btn = gtk_button_new_with_label("Open Console");
        gtk_widget_add_css_class(console_btn, "btn-action");
        gtk_widget_add_css_class(console_btn, "btn-create");
        // TODO: Connect to console opening
        gtk_box_append(GTK_BOX(btn_row), console_btn);
    }
    
    GtkWidget *snapshot_btn = gtk_button_new_with_label("Snapshots");
    gtk_widget_add_css_class(snapshot_btn, "btn-action");
    gtk_widget_add_css_class(snapshot_btn, "btn-reboot");
    // TODO: Connect to snapshot dialog
    gtk_box_append(GTK_BOX(btn_row), snapshot_btn);
    
    gtk_box_append(GTK_BOX(main_box), btn_row);
    
    /* Close button */
    GtkWidget *close_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(close_row, GTK_ALIGN_END);
    gtk_widget_set_margin_top(close_row, 16);
    
    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    gtk_widget_add_css_class(close_btn, "btn-action");
    gtk_widget_add_css_class(close_btn, "btn-reboot");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(close_row), close_btn);
    
    gtk_box_append(GTK_BOX(main_box), close_row);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), main_box);
    gtk_window_set_child(GTK_WINDOW(dialog), scroll);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ──────────────────────────────────────────────────────────── */
/* ── Container Details Dialog ──────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

void ui_show_ct_details_dialog(AppData *app, CtInfo *ct) {
    if (!ct) return;
    apply_dialog_css();
    
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), ct->name);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(main_box, 24);
    gtk_widget_set_margin_end(main_box, 24);
    gtk_widget_set_margin_top(main_box, 24);
    gtk_widget_set_margin_bottom(main_box, 24);
    
    /* Header */
    GtkWidget *header = gtk_label_new(ct->name);
    gtk_widget_add_css_class(header, "details-header");
    gtk_label_set_xalign(GTK_LABEL(header), 0);
    gtk_box_append(GTK_BOX(main_box), header);
    
    /* Status badge */
    GtkWidget *badge = gtk_label_new(ct_state_str(ct->state));
    gtk_widget_add_css_class(badge, "state-badge");
    gtk_widget_add_css_class(badge, ct_state_css(ct->state));
    gtk_label_set_xalign(GTK_LABEL(badge), 0);
    gtk_box_append(GTK_BOX(main_box), badge);
    
    /* Basic info section */
    add_section_header(main_box, "BASIC INFORMATION");
    
    char buf[256];
    
    add_detail_row(main_box, "Type", ct->type[0] ? ct->type : "container");
    add_detail_row(main_box, "State", ct_state_str(ct->state));
    add_detail_row(main_box, "IP Address", ct->ipv4[0] ? ct->ipv4 : "Not assigned");
    add_detail_row(main_box, "Image", ct->image[0] ? ct->image : "Unknown");
    add_detail_row(main_box, "Created", ct->created[0] ? ct->created : "Unknown");
    
    /* Actions section */
    add_section_header(main_box, "ACTIONS");
    
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    
    if (ct->state == CT_STATE_RUNNING) {
        GtkWidget *console_btn = gtk_button_new_with_label("Open Shell");
        gtk_widget_add_css_class(console_btn, "btn-action");
        gtk_widget_add_css_class(console_btn, "btn-create");
        // TODO: Connect to console opening
        gtk_box_append(GTK_BOX(btn_row), console_btn);
    }
    
    gtk_box_append(GTK_BOX(main_box), btn_row);
    
    /* Close button */
    GtkWidget *close_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(close_row, GTK_ALIGN_END);
    gtk_widget_set_margin_top(close_row, 16);
    
    GtkWidget *close_btn = gtk_button_new_with_label("Close");
    gtk_widget_add_css_class(close_btn, "btn-action");
    gtk_widget_add_css_class(close_btn, "btn-reboot");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_box_append(GTK_BOX(close_row), close_btn);
    
    gtk_box_append(GTK_BOX(main_box), close_row);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), main_box);
    gtk_window_set_child(GTK_WINDOW(dialog), scroll);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ──────────────────────────────────────────────────────────── */
/* ── Settings Dialog ───────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

typedef struct {
    AppData    *app;
    GtkWidget  *dialog;
    GtkWidget  *refresh_spin;
    GtkWidget  *confirm_check;
    GtkWidget  *auto_refresh_check;
    GtkWidget  *vcpu_spin;
    GtkWidget  *ram_spin;
    GtkWidget  *disk_spin;
    GtkWidget  *image_entry;
} SettingsCtx;

static void on_settings_save(GtkButton *btn, gpointer data) {
    (void)btn;
    SettingsCtx *ctx = data;
    
    /* Update settings from UI */
    ctx->app->stats.cpu_usage_percent = 0; // Just to access app
    
    /* Save to config file */
    settings_save(ctx->app);
    
    app_log(ctx->app, "INFO", "Settings saved");
    
    gtk_window_destroy(GTK_WINDOW(ctx->dialog));
    g_free(ctx);
}

void ui_show_settings_dialog(AppData *app) {
    apply_dialog_css();
    
    SettingsCtx *ctx = g_new0(SettingsCtx, 1);
    ctx->app = app;
    
    GtkWidget *dialog = gtk_window_new();
    ctx->dialog = dialog;
    gtk_window_set_title(GTK_WINDOW(dialog), "Settings");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 450, 500);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app->window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(main_box, 24);
    gtk_widget_set_margin_end(main_box, 24);
    gtk_widget_set_margin_top(main_box, 24);
    gtk_widget_set_margin_bottom(main_box, 24);
    
    /* Header */
    GtkWidget *title = gtk_label_new("Settings");
    gtk_widget_add_css_class(title, "details-header");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    gtk_box_append(GTK_BOX(main_box), title);
    
    /* General section */
    add_section_header(main_box, "GENERAL");
    
    /* Refresh interval */
    GtkWidget *refresh_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *refresh_lbl = gtk_label_new("Refresh interval (seconds):");
    gtk_widget_add_css_class(refresh_lbl, "settings-label");
    gtk_label_set_xalign(GTK_LABEL(refresh_lbl), 0);
    gtk_widget_set_hexpand(refresh_lbl, TRUE);
    gtk_box_append(GTK_BOX(refresh_box), refresh_lbl);
    
    ctx->refresh_spin = gtk_spin_button_new_with_range(1, 60, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->refresh_spin), 
                               REFRESH_INTERVAL / 1000);
    gtk_box_append(GTK_BOX(refresh_box), ctx->refresh_spin);
    gtk_box_append(GTK_BOX(main_box), refresh_box);
    
    /* Confirm destructive actions */
    ctx->confirm_check = gtk_check_button_new_with_label(
        "Confirm destructive actions (delete, force stop)");
    gtk_widget_add_css_class(ctx->confirm_check, "settings-label");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(ctx->confirm_check), TRUE);
    gtk_box_append(GTK_BOX(main_box), ctx->confirm_check);
    
    /* Auto refresh */
    ctx->auto_refresh_check = gtk_check_button_new_with_label(
        "Auto-refresh dashboard");
    gtk_widget_add_css_class(ctx->auto_refresh_check, "settings-label");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(ctx->auto_refresh_check), TRUE);
    gtk_box_append(GTK_BOX(main_box), ctx->auto_refresh_check);
    
    /* VM Defaults section */
    add_section_header(main_box, "VM DEFAULTS");
    
    /* Default vCPUs */
    GtkWidget *vcpu_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *vcpu_lbl = gtk_label_new("Default vCPUs:");
    gtk_widget_add_css_class(vcpu_lbl, "settings-label");
    gtk_widget_set_hexpand(vcpu_lbl, TRUE);
    gtk_box_append(GTK_BOX(vcpu_box), vcpu_lbl);
    
    ctx->vcpu_spin = gtk_spin_button_new_with_range(1, 16, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->vcpu_spin), 2);
    gtk_box_append(GTK_BOX(vcpu_box), ctx->vcpu_spin);
    gtk_box_append(GTK_BOX(main_box), vcpu_box);
    
    /* Default RAM */
    GtkWidget *ram_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *ram_lbl = gtk_label_new("Default RAM (MB):");
    gtk_widget_add_css_class(ram_lbl, "settings-label");
    gtk_widget_set_hexpand(ram_lbl, TRUE);
    gtk_box_append(GTK_BOX(ram_box), ram_lbl);
    
    ctx->ram_spin = gtk_spin_button_new_with_range(256, 32768, 256);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->ram_spin), 2048);
    gtk_box_append(GTK_BOX(ram_box), ctx->ram_spin);
    gtk_box_append(GTK_BOX(main_box), ram_box);
    
    /* Default Disk */
    GtkWidget *disk_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *disk_lbl = gtk_label_new("Default Disk (GB):");
    gtk_widget_add_css_class(disk_lbl, "settings-label");
    gtk_widget_set_hexpand(disk_lbl, TRUE);
    gtk_box_append(GTK_BOX(disk_box), disk_lbl);
    
    ctx->disk_spin = gtk_spin_button_new_with_range(5, 500, 5);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ctx->disk_spin), 20);
    gtk_box_append(GTK_BOX(disk_box), ctx->disk_spin);
    gtk_box_append(GTK_BOX(main_box), disk_box);
    
    /* Container Defaults section */
    add_section_header(main_box, "CONTAINER DEFAULTS");
    
    GtkWidget *image_lbl = gtk_label_new("Default Image:");
    gtk_widget_add_css_class(image_lbl, "settings-label");
    gtk_label_set_xalign(GTK_LABEL(image_lbl), 0);
    gtk_box_append(GTK_BOX(main_box), image_lbl);
    
    ctx->image_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ctx->image_entry), "ubuntu/24.04");
    gtk_editable_set_text(GTK_EDITABLE(ctx->image_entry), "ubuntu/24.04");
    gtk_box_append(GTK_BOX(main_box), ctx->image_entry);
    
    GtkWidget *hint = gtk_label_new("Examples: ubuntu/24.04, debian/12, alpine/3.19");
    gtk_widget_add_css_class(hint, "settings-hint");
    gtk_label_set_xalign(GTK_LABEL(hint), 0);
    gtk_box_append(GTK_BOX(main_box), hint);
    
    /* Buttons */
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(btn_row, GTK_ALIGN_END);
    gtk_widget_set_margin_top(btn_row, 16);
    
    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    gtk_widget_add_css_class(cancel_btn, "btn-action");
    gtk_widget_add_css_class(cancel_btn, "btn-reboot");
    g_signal_connect_swapped(cancel_btn, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    g_signal_connect_swapped(cancel_btn, "clicked", G_CALLBACK(g_free), ctx);
    gtk_box_append(GTK_BOX(btn_row), cancel_btn);
    
    GtkWidget *save_btn = gtk_button_new_with_label("Save");
    gtk_widget_add_css_class(save_btn, "btn-action");
    gtk_widget_add_css_class(save_btn, "btn-create");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_settings_save), ctx);
    gtk_box_append(GTK_BOX(btn_row), save_btn);
    
    gtk_box_append(GTK_BOX(main_box), btn_row);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), main_box);
    gtk_window_set_child(GTK_WINDOW(dialog), scroll);
    gtk_window_present(GTK_WINDOW(dialog));
}

/* ──────────────────────────────────────────────────────────── */
/* ── Settings Load/Save ────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

void settings_load(AppData *app) {
    /* Default settings */
    static AppSettings default_settings = {
        .refresh_interval_ms = REFRESH_INTERVAL,
        .confirm_destructive_actions = true,
        .auto_refresh = true,
        .default_iso_path = "",
        .default_image = "ubuntu/24.04",
        .default_vcpus = 2,
        .default_ram_mb = 2048,
        .default_disk_gb = 20
    };
    
    /* Try to load from config file */
    char config_path[512];
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    
    snprintf(config_path, sizeof(config_path), "%s/.config/vmmanager/settings.json", home);
    
    GFile *file = g_file_new_for_path(config_path);
    if (g_file_query_exists(file, NULL)) {
        GError *error = NULL;
        char *contents = NULL;
        gsize length = 0;
        
        if (g_file_load_contents(file, NULL, &contents, &length, NULL, &error)) {
            JsonParser *parser = json_parser_new();
            if (json_parser_load_from_data(parser, contents, length, NULL)) {
                JsonObject *obj = json_node_get_object(json_parser_get_root(parser));
                
                /* Parse settings */
                if (json_object_has_member(obj, "refresh_interval")) {
                    default_settings.refresh_interval_ms = 
                        json_object_get_int_member(obj, "refresh_interval") * 1000;
                }
                if (json_object_has_member(obj, "confirm_destructive")) {
                    default_settings.confirm_destructive_actions = 
                        json_object_get_boolean_member(obj, "confirm_destructive");
                }
                if (json_object_has_member(obj, "auto_refresh")) {
                    default_settings.auto_refresh = 
                        json_object_get_boolean_member(obj, "auto_refresh");
                }
                if (json_object_has_member(obj, "default_vcpus")) {
                    default_settings.default_vcpus = 
                        json_object_get_int_member(obj, "default_vcpus");
                }
                if (json_object_has_member(obj, "default_ram_mb")) {
                    default_settings.default_ram_mb = 
                        json_object_get_int_member(obj, "default_ram_mb");
                }
                if (json_object_has_member(obj, "default_disk_gb")) {
                    default_settings.default_disk_gb = 
                        json_object_get_int_member(obj, "default_disk_gb");
                }
                if (json_object_has_member(obj, "default_image")) {
                    const char *img = json_object_get_string_member(obj, "default_image");
                    if (img) strncpy(default_settings.default_image, img, 127);
                }
                
                g_object_unref(parser);
            }
            g_free(contents);
        }
        if (error) g_error_free(error);
    }
    g_object_unref(file);
    
    /* Store settings in app data - we'll add this to AppData later */
    (void)app; /* For now, just use defaults */
}

void settings_save(AppData *app) {
    char config_path[512];
    const char *home = getenv("HOME");
    if (!home) return;
    
    snprintf(config_path, sizeof(config_path), "%s/.config/vmmanager", home);
    g_mkdir_with_parents(config_path, 0755);
    
    snprintf(config_path, sizeof(config_path), "%s/.config/vmmanager/settings.json", home);
    
    /* Build JSON */
    JsonBuilder *builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "refresh_interval");
    json_builder_add_int_value(builder, REFRESH_INTERVAL / 1000);
    json_builder_set_member_name(builder, "confirm_destructive");
    json_builder_add_boolean_value(builder, true);
    json_builder_set_member_name(builder, "auto_refresh");
    json_builder_add_boolean_value(builder, true);
    json_builder_set_member_name(builder, "default_vcpus");
    json_builder_add_int_value(builder, 2);
    json_builder_set_member_name(builder, "default_ram_mb");
    json_builder_add_int_value(builder, 2048);
    json_builder_set_member_name(builder, "default_disk_gb");
    json_builder_add_int_value(builder, 20);
    json_builder_set_member_name(builder, "default_image");
    json_builder_add_string_value(builder, "ubuntu/24.04");
    json_builder_end_object(builder);
    
    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, json_builder_get_root(builder));
    json_generator_set_pretty(gen, TRUE);
    
    GError *error = NULL;
    json_generator_to_file(gen, config_path, &error);
    
    if (error) {
        app_log(app, "WARN", "Failed to save settings: %s", error->message);
        g_error_free(error);
    }
    
    g_object_unref(builder);
    g_object_unref(gen);
}

/* ──────────────────────────────────────────────────────────── */
/* ── Keyboard Shortcuts ────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

static void on_shortcut_refresh(GSimpleAction *action, GVariant *param, gpointer data) {
    (void)action; (void)param;
    AppData *app = data;
    ui_refresh_all(app);
    app_log(app, "INFO", "Refreshed all data");
}

static void on_shortcut_settings(GSimpleAction *action, GVariant *param, gpointer data) {
    (void)action; (void)param;
    AppData *app = data;
    ui_show_settings_dialog(app);
}

static void on_shortcut_new_vm(GSimpleAction *action, GVariant *param, gpointer data) {
    (void)action; (void)param;
    AppData *app = data;
    ui_show_vm_create_dialog(app);
}

static void on_shortcut_new_ct(GSimpleAction *action, GVariant *param, gpointer data) {
    (void)action; (void)param;
    AppData *app = data;
    ui_show_ct_create_dialog(app);
}

void ui_setup_shortcuts(AppData *app) {
    /* Create action group */
    GSimpleActionGroup *group = g_simple_action_group_new();
    
    GSimpleAction *refresh_action = g_simple_action_new("refresh", NULL);
    g_signal_connect(refresh_action, "activate", G_CALLBACK(on_shortcut_refresh), app);
    g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(refresh_action));
    
    GSimpleAction *settings_action = g_simple_action_new("settings", NULL);
    g_signal_connect(settings_action, "activate", G_CALLBACK(on_shortcut_settings), app);
    g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(settings_action));
    
    GSimpleAction *new_vm_action = g_simple_action_new("new-vm", NULL);
    g_signal_connect(new_vm_action, "activate", G_CALLBACK(on_shortcut_new_vm), app);
    g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(new_vm_action));
    
    GSimpleAction *new_ct_action = g_simple_action_new("new-ct", NULL);
    g_signal_connect(new_ct_action, "activate", G_CALLBACK(on_shortcut_new_ct), app);
    g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(new_ct_action));
    
    gtk_widget_insert_action_group(app->window, "app", G_ACTION_GROUP(group));
    
    /* Create shortcuts */
    GtkShortcutController *controller = gtk_shortcut_controller_new();
    
    gtk_shortcut_controller_add_shortcut(controller,
        gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_F5, 0),
                         gtk_named_action_new("app.refresh")));
    
    gtk_shortcut_controller_add_shortcut(controller,
        gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_comma, GDK_CONTROL_MASK),
                         gtk_named_action_new("app.settings")));
    
    gtk_shortcut_controller_add_shortcut(controller,
        gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_n, GDK_CONTROL_MASK | GDK_SHIFT_MASK),
                         gtk_named_action_new("app.new-vm")));
    
    gtk_shortcut_controller_add_shortcut(controller,
        gtk_shortcut_new(gtk_keyval_trigger_new(GDK_KEY_n, GDK_CONTROL_MASK),
                         gtk_named_action_new("app.new-ct")));
    
    gtk_widget_add_controller(app->window, GTK_EVENT_CONTROLLER(controller));
    
    g_object_unref(group);
}

/* ──────────────────────────────────────────────────────────── */
/* ── Console Opening ───────────────────────────────────────── */
/* ──────────────────────────────────────────────────────────── */

void ui_open_vm_console(AppData *app, const char *vm_name) {
    if (!app->virt_conn) {
        ui_show_error_dialog(app, "Error", 
                             "Cannot open console: libvirt not connected",
                             "Ensure libvirtd is running");
        return;
    }
    
    /* Try to launch virt-viewer */
    char command[512];
    snprintf(command, sizeof(command), "virt-viewer --connect qemu:///system %s &", vm_name);
    
    int result = system(command);
    if (result != 0) {
        ui_show_error_dialog(app, "Error",
                             "Failed to launch VM console",
                             "Ensure virt-viewer is installed: sudo apt install virt-viewer");
        app_log(app, "WARN", "Failed to launch console for VM '%s'", vm_name);
    } else {
        app_log(app, "INFO", "Opened console for VM '%s'", vm_name);
    }
}

void ui_open_ct_console(AppData *app, const char *ct_name) {
    if (!app->incus_available) {
        ui_show_error_dialog(app, "Error",
                             "Cannot open shell: Incus/LXD not connected",
                             "Ensure Incus is installed and socket is accessible");
        return;
    }
    
    /* Try to launch terminal with incus exec */
    char command[512];
    
    /* Try different terminals */
    const char *terminals[] = {
        "gnome-terminal -- incus exec %s -- /bin/bash",
        "konsole -e incus exec %s -- /bin/bash",
        "xterm -e incus exec %s -- /bin/bash",
        NULL
    };
    
    bool launched = false;
    for (int i = 0; terminals[i]; i++) {
        snprintf(command, sizeof(command), terminals[i], ct_name);
        if (system(command) == 0) {
            launched = true;
            break;
        }
    }
    
    if (!launched) {
        ui_show_error_dialog(app, "Error",
                             "Failed to launch container shell",
                             "Install a terminal emulator (gnome-terminal, konsole, or xterm)");
        app_log(app, "WARN", "Failed to launch shell for container '%s'", ct_name);
    } else {
        app_log(app, "INFO", "Opened shell for container '%s'", ct_name);
    }
}
