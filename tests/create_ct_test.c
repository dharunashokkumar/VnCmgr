#include "include/vmmanager.h"
#include <stdio.h>

void app_log(AppData *app, const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("[%s] ", level);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

int main() {
    AppData app = {0};
    ct_backend_connect(&app);
    printf("incus_available: %d\n", app.incus_available);
    bool ok = ct_create(&app, "test-ct-app", "alpine/3.19");
    printf("Create OK: %d\n", ok);
    return 0;
}
