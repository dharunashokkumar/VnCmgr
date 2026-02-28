/*
 * system_info.c - System resource monitoring (CPU, RAM, Disk)
 */

#include "../include/vmmanager.h"
#include <inttypes.h>

/* ── CPU Usage (reads /proc/stat) ──────────────────────────── */
double sys_get_cpu_usage(void) {
    static long prev_idle = 0, prev_total = 0;

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return 0.0;

    char line[256];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return 0.0;
    }
    fclose(fp);

    long user, nice, sys, idle, iowait, irq, softirq, steal;
    sscanf(line, "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
           &user, &nice, &sys, &idle, &iowait, &irq, &softirq, &steal);

    long total_idle = idle + iowait;
    long total = user + nice + sys + idle + iowait + irq + softirq + steal;

    long diff_idle  = total_idle - prev_idle;
    long diff_total = total - prev_total;

    prev_idle  = total_idle;
    prev_total = total;

    if (diff_total == 0) return 0.0;

    return (1.0 - (double)diff_idle / (double)diff_total) * 100.0;
}

/* ── Memory from /proc/meminfo ─────────────────────────────── */
static void get_memory_info(int64_t *total_mb, int64_t *used_mb) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) { *total_mb = 0; *used_mb = 0; return; }

    int64_t mem_total = 0, mem_free = 0, buffers = 0, cached = 0, sreclaimable = 0;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %" SCNd64 " kB", &mem_total) == 1) continue;
        if (sscanf(line, "MemFree: %" SCNd64 " kB", &mem_free) == 1) continue;
        if (sscanf(line, "Buffers: %" SCNd64 " kB", &buffers) == 1) continue;
        if (sscanf(line, "Cached: %" SCNd64 " kB", &cached) == 1) continue;
        if (sscanf(line, "SReclaimable: %" SCNd64 " kB", &sreclaimable) == 1) continue;
    }
    fclose(fp);

    *total_mb = mem_total / 1024;
    int64_t avail = mem_free + buffers + cached + sreclaimable;
    *used_mb = (mem_total - avail) / 1024;
    if (*used_mb < 0) *used_mb = 0;
}

/* ── Disk usage from statvfs ───────────────────────────────── */
static void get_disk_info(int64_t *total_mb, int64_t *used_mb) {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        *total_mb = 0; *used_mb = 0; return;
    }

    uint64_t block_size = stat.f_frsize;
    *total_mb = (int64_t)((uint64_t)stat.f_blocks * block_size / (1024 * 1024));
    int64_t free_mb = (int64_t)((uint64_t)stat.f_bavail * block_size / (1024 * 1024));
    *used_mb = *total_mb - free_mb;
}

/* ── Gather all system stats ───────────────────────────────── */
void sys_get_stats(AppData *app) {
    app->stats.cpu_usage_percent = sys_get_cpu_usage();
    get_memory_info(&app->stats.total_ram_mb, &app->stats.used_ram_mb);
    get_disk_info(&app->stats.total_disk_mb, &app->stats.used_disk_mb);
}
