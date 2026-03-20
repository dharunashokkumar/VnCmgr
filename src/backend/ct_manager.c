/*
 * ct_manager.c - Incus/LXD container backend via REST API over Unix socket
 */

#include "../include/vmmanager.h"

const char *ct_state_str(CtState s) {
    switch (s) {
        case CT_STATE_RUNNING: return "Running";
        case CT_STATE_STOPPED: return "Stopped";
        case CT_STATE_FROZEN:  return "Frozen";
        default:               return "Unknown";
    }
}

const char *ct_state_css(CtState s) {
    switch (s) {
        case CT_STATE_RUNNING: return "state-running";
        case CT_STATE_STOPPED: return "state-stopped";
        case CT_STATE_FROZEN:  return "state-paused";
        default:               return "state-unknown";
    }
}

/* ── CURL write callback ───────────────────────────────────── */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    HttpBuffer *buf = (HttpBuffer *)userp;

    char *ptr = realloc(buf->data, buf->size + realsize + 1);
    if (!ptr) return 0;

    buf->data = ptr;
    memcpy(&(buf->data[buf->size]), contents, realsize);
    buf->size += realsize;
    buf->data[buf->size] = '\0';
    return realsize;
}

/* ── Check if Incus/LXD socket exists ──────────────────────── */
bool ct_backend_connect(AppData *app) {
    /* Try Incus first - need R_OK | W_OK for Unix sockets */
    if (access(INCUS_SOCKET, R_OK | W_OK) == 0) {
        strncpy(app->incus_socket_path, INCUS_SOCKET, sizeof(app->incus_socket_path) - 1);
        app->incus_available = true;
        app_log(app, "INFO", "Connected to Incus socket");
        return true;
    }

    /* Try LXD */
    if (access(LXD_SOCKET, R_OK | W_OK) == 0) {
        strncpy(app->incus_socket_path, LXD_SOCKET, sizeof(app->incus_socket_path) - 1);
        app->incus_available = true;
        app_log(app, "INFO", "Connected to LXD socket");
        return true;
    }

    /* Try common alternative paths */
    const char *alt_paths[] = {
        "/var/lib/lxd/unix.socket",
        "/run/incus/unix.socket",
        NULL
    };

    for (int i = 0; alt_paths[i]; i++) {
        if (access(alt_paths[i], R_OK | W_OK) == 0) {
            strncpy(app->incus_socket_path, alt_paths[i],
                    sizeof(app->incus_socket_path) - 1);
            app->incus_available = true;
            app_log(app, "INFO", "Connected to container socket: %s", alt_paths[i]);
            return true;
        }
    }

    app->incus_available = false;
    app_log(app, "WARN", "No Incus/LXD socket found — container management disabled");
    app_log(app, "INFO", "To enable container support, add user to 'incus-admin' group:");
    app_log(app, "INFO", "  sudo usermod -aG incus-admin $USER");
    app_log(app, "INFO", "  Then log out and back in, or run: newgrp incus-admin");
    return false;
}

/* ── Generic REST API request to Incus/LXD with configurable timeout ── */
char *ct_api_request_timeout(AppData *app, const char *method,
                             const char *endpoint, const char *body,
                             long timeout_sec) {
    if (!app->incus_available) return NULL;

    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    HttpBuffer buf = { .data = malloc(1), .size = 0 };

    char url[1024];
    snprintf(url, sizeof(url), "http://localhost%s", endpoint);

    curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, app->incus_socket_path);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (body) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        else curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (body) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (strcmp(method, "PATCH") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        if (body) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    }

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        app_log(app, "ERROR", "API request failed: %s", curl_easy_strerror(res));
        free(buf.data);
        return NULL;
    }

    return buf.data;
}

/* ── Convenience wrapper with default 10s timeout ──────────── */
char *ct_api_request(AppData *app, const char *method,
                     const char *endpoint, const char *body) {
    return ct_api_request_timeout(app, method, endpoint, body, 10L);
}

/* ── Wait for API Operation ────────────────────────────────── */
bool ct_wait_operation(AppData *app, const char *json_resp, const char *action, const char *name) {
    if (!json_resp) return false;

    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_data(parser, json_resp, -1, NULL)) {
        g_object_unref(parser);
        return false;
    }

    JsonObject *root_obj = json_node_get_object(json_parser_get_root(parser));
    const char *op_url = NULL;

    if (json_object_has_member(root_obj, "operation")) {
        op_url = json_object_get_string_member(root_obj, "operation");
    }

    if (!op_url) {
        /* Not async, might be OK */
        g_object_unref(parser);
        return true;
    }

    /* Wait for it using the operation URL (up to 300s timeout on API request) */
    char wait_endpoint[512];
    snprintf(wait_endpoint, sizeof(wait_endpoint), "%s/wait", op_url);

    app_log(app, "INFO", "Waiting for container '%s' %s to complete...", name, action);

    char *wait_resp = ct_api_request_timeout(app, "GET", wait_endpoint, NULL, 60L);
    g_object_unref(parser);

    if (!wait_resp) {
        app_log(app, "ERROR", "Timed out or failed waiting for container '%s' %s", name, action);
        return false;
    }

    /* Check result if available */
    bool success = true;
    parser = json_parser_new();
    if (json_parser_load_from_data(parser, wait_resp, -1, NULL)) {
        JsonObject *w_root = json_node_get_object(json_parser_get_root(parser));
        if (json_object_has_member(w_root, "metadata")) {
            JsonObject *meta = json_object_get_object_member(w_root, "metadata");
            if (json_object_has_member(meta, "status")) {
                const char *status = json_object_get_string_member(meta, "status");
                if (status && strcasecmp(status, "Failure") == 0) {
                    success = false;
                    const char *err = json_object_has_member(meta, "err") ?
                        json_object_get_string_member(meta, "err") : "Unknown error";
                    app_log(app, "ERROR", "Container '%s' %s failed: %s", name, action, err);
                } else if (status && strcasecmp(status, "Success") == 0) {
                    success = true;
                }
            }
        }
    }

    g_object_unref(parser);
    free(wait_resp);
    return success;
}

/* ── Parse container state string ──────────────────────────── */
static CtState parse_ct_state(const char *status) {
    if (!status) return CT_STATE_UNKNOWN;
    if (strcasecmp(status, "Running") == 0) return CT_STATE_RUNNING;
    if (strcasecmp(status, "Stopped") == 0) return CT_STATE_STOPPED;
    if (strcasecmp(status, "Frozen") == 0)  return CT_STATE_FROZEN;
    return CT_STATE_UNKNOWN;
}

/* ── Refresh container list ────────────────────────────────── */
bool ct_backend_refresh(AppData *app) {
    if (!app->incus_available) return false;

    /* Free old list */
    if (app->containers) {
        free(app->containers);
        app->containers = NULL;
    }
    app->ct_count = 0;
    app->stats.total_containers = 0;
    app->stats.running_containers = 0;

    /* Get instances with full detail */
    char *response = ct_api_request(app, "GET", "/1.0/instances?recursion=1", NULL);
    if (!response) {
        app_log(app, "ERROR", "Failed to query containers");
        return false;
    }

    /* Parse JSON response */
    JsonParser *parser = json_parser_new();
    GError *error = NULL;

    if (!json_parser_load_from_data(parser, response, -1, &error)) {
        app_log(app, "ERROR", "JSON parse error: %s", error->message);
        g_error_free(error);
        g_object_unref(parser);
        free(response);
        return false;
    }

    JsonNode *root = json_parser_get_root(parser);
    JsonObject *root_obj = json_node_get_object(root);

    if (!json_object_has_member(root_obj, "metadata")) {
        g_object_unref(parser);
        free(response);
        return true; /* No containers */
    }

    JsonArray *metadata = json_object_get_array_member(root_obj, "metadata");
    int count = json_array_get_length(metadata);

    if (count == 0) {
        g_object_unref(parser);
        free(response);
        return true;
    }

    app->containers = calloc(count, sizeof(CtInfo));
    if (!app->containers) {
        g_object_unref(parser);
        free(response);
        return false;
    }

    int valid = 0;
    for (int i = 0; i < count; i++) {
        JsonObject *inst = json_array_get_object_element(metadata, i);
        if (!inst) continue;

        CtInfo *ct = &app->containers[valid];

        /* Name */
        if (json_object_has_member(inst, "name")) {
            const char *n = json_object_get_string_member(inst, "name");
            if (n) strncpy(ct->name, n, 255);
        }

        /* Type */
        if (json_object_has_member(inst, "type")) {
            const char *t = json_object_get_string_member(inst, "type");
            if (t) strncpy(ct->type, t, 31);
        }

        /* Status */
        if (json_object_has_member(inst, "status")) {
            const char *s = json_object_get_string_member(inst, "status");
            ct->state = parse_ct_state(s);
            if (ct->state == CT_STATE_RUNNING)
                app->stats.running_containers++;
        }

        /* Created */
        if (json_object_has_member(inst, "created_at")) {
            const char *c = json_object_get_string_member(inst, "created_at");
            if (c) strncpy(ct->created, c, 63);
        }

        /* Try to get IP from state.network */
        ct->ipv4[0] = '\0';
        if (json_object_has_member(inst, "state")) {
            JsonNode *state_node = json_object_get_member(inst, "state");
            if (state_node && JSON_NODE_HOLDS_OBJECT(state_node)) {
                JsonObject *state_obj = json_node_get_object(state_node);
                if (json_object_has_member(state_obj, "network")) {
                    JsonObject *net = json_object_get_object_member(state_obj, "network");
                    if (net && json_object_has_member(net, "eth0")) {
                        JsonObject *eth0 = json_object_get_object_member(net, "eth0");
                        if (eth0 && json_object_has_member(eth0, "addresses")) {
                            JsonArray *addrs = json_object_get_array_member(eth0, "addresses");
                            for (guint j = 0; j < json_array_get_length(addrs); j++) {
                                JsonObject *addr = json_array_get_object_element(addrs, j);
                                if (json_object_has_member(addr, "family")) {
                                    const char *fam = json_object_get_string_member(addr, "family");
                                    if (fam && strcmp(fam, "inet") == 0) {
                                        const char *ip = json_object_get_string_member(addr, "address");
                                        if (ip) strncpy(ct->ipv4, ip, 63);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        valid++;
    }

    app->ct_count = valid;
    app->stats.total_containers = valid;

    g_object_unref(parser);
    free(response);
    return true;
}

/* ── Container Operations ──────────────────────────────────── */
bool ct_start(AppData *app, const char *name) {
    char endpoint[512];
    snprintf(endpoint, sizeof(endpoint), "/1.0/instances/%s/state", name);
    char *body = "{\"action\": \"start\"}";

    char *resp = ct_api_request(app, "PUT", endpoint, body);
    if (!resp) {
        app_log(app, "ERROR", "Failed to start container '%s'", name);
        return false;
    }

    bool ok = ct_wait_operation(app, resp, "start", name);
    free(resp);

    if (ok) app_log(app, "INFO", "Container '%s' started", name);
    return ok;
}

bool ct_stop(AppData *app, const char *name) {
    char endpoint[512];
    snprintf(endpoint, sizeof(endpoint), "/1.0/instances/%s/state", name);
    char *body = "{\"action\": \"stop\", \"timeout\": 30}";

    char *resp = ct_api_request(app, "PUT", endpoint, body);
    if (!resp) {
        app_log(app, "ERROR", "Failed to stop container '%s'", name);
        return false;
    }

    bool ok = ct_wait_operation(app, resp, "stop", name);
    free(resp);

    if (ok) app_log(app, "INFO", "Container '%s' stopped", name);
    return ok;
}

bool ct_force_stop(AppData *app, const char *name) {
    char endpoint[512];
    snprintf(endpoint, sizeof(endpoint), "/1.0/instances/%s/state", name);
    char *body = "{\"action\": \"stop\", \"force\": true}";

    char *resp = ct_api_request(app, "PUT", endpoint, body);
    if (!resp) {
        app_log(app, "ERROR", "Failed to force stop container '%s'", name);
        return false;
    }

    bool ok = ct_wait_operation(app, resp, "force stop", name);
    free(resp);

    if (ok) app_log(app, "INFO", "Container '%s' force stopped", name);
    return ok;
}

bool ct_restart(AppData *app, const char *name) {
    char endpoint[512];
    snprintf(endpoint, sizeof(endpoint), "/1.0/instances/%s/state", name);
    char *body = "{\"action\": \"restart\"}";

    char *resp = ct_api_request(app, "PUT", endpoint, body);
    if (!resp) {
        app_log(app, "ERROR", "Failed to restart container '%s'", name);
        return false;
    }

    bool ok = ct_wait_operation(app, resp, "restart", name);
    free(resp);

    if (ok) app_log(app, "INFO", "Container '%s' restarted", name);
    return ok;
}

bool ct_delete(AppData *app, const char *name) {
    /* Stop first if running */
    ct_force_stop(app, name);
    usleep(500000); /* brief pause */

    char endpoint[512];
    snprintf(endpoint, sizeof(endpoint), "/1.0/instances/%s", name);

    char *resp = ct_api_request(app, "DELETE", endpoint, NULL);
    if (!resp) {
        app_log(app, "ERROR", "Failed to delete container '%s'", name);
        return false;
    }

    bool ok = ct_wait_operation(app, resp, "delete", name);
    free(resp);

    if (ok) app_log(app, "INFO", "Container '%s' deleted", name);
    return ok;
}

bool ct_create(AppData *app, const char *name, const char *image) {
    char body[2048];
    snprintf(body, sizeof(body),
        "{"
        "  \"name\": \"%s\","
        "  \"source\": {"
        "    \"type\": \"image\","
        "    \"mode\": \"pull\","
        "    \"server\": \"https://images.linuxcontainers.org\","
        "    \"protocol\": \"simplestreams\","
        "    \"alias\": \"%s\""
        "  }"
        "}", name, image);

    char *resp = ct_api_request(app, "POST", "/1.0/instances", body);
    if (!resp) {
        app_log(app, "ERROR", "Failed to create container '%s'", name);
        return false;
    }

    /* Check for error in response */
    JsonParser *parser = json_parser_new();
    if (json_parser_load_from_data(parser, resp, -1, NULL)) {
        JsonObject *obj = json_node_get_object(json_parser_get_root(parser));
        if (json_object_has_member(obj, "error") &&
            strlen(json_object_get_string_member(obj, "error")) > 0) {
            app_log(app, "ERROR", "Create container error: %s",
                    json_object_get_string_member(obj, "error"));
            g_object_unref(parser);
            free(resp);
            return false;
        }
    }
    g_object_unref(parser);

    bool ok = ct_wait_operation(app, resp, "create", name);
    free(resp);

    if (ok) app_log(app, "INFO", "Container '%s' created successfully (image: %s)", name, image);
    return ok;
}
