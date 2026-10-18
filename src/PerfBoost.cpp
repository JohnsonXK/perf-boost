// PerfBoost - pins Minecraft's main thread to the fastest CPU cores.
// Runs automatically when the .so is loaded (no LeviLaunchroid API needed).
#include <android/log.h>
#include <sched.h>
#include <sys/resource.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#define TAG "PerfBoost"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

// Max frequency (kHz) of a CPU core, or 0 if it can't be read.
static long readMaxFreq(int cpu) {
    std::ifstream f("/sys/devices/system/cpu/cpu" + std::to_string(cpu) +
                    "/cpufreq/cpuinfo_max_freq");
    long v = 0;
    if (f >> v) return v;
    return 0;
}

static void applyBoost() {
    int n = static_cast<int>(sysconf(_SC_NPROCESSORS_CONF));
    if (n <= 1) return;

    std::vector<long> freq(n, 0);
    long best = 0;
    for (int i = 0; i < n; ++i) {
        freq[i] = readMaxFreq(i);
        best = std::max(best, freq[i]);
    }
    if (best <= 0) {
        LOGI("Could not read CPU frequencies, skipping");
        return;
    }

    // Use every core within 70% of the fastest core's max frequency.
    cpu_set_t set;
    CPU_ZERO(&set);
    int used = 0;
    for (int i = 0; i < n; ++i) {
        if (freq[i] >= best * 0.7) {
            CPU_SET(i, &set);
            ++used;
        }
    }

    // On Android, the main thread's tid equals the process id.
    pid_t mainTid = getpid();
    int r = sched_setaffinity(mainTid, sizeof(set), &set);
    LOGI("Pinned main thread to %d fast cores (result %d)", used, r);

    // Likely to be denied for apps; harmless if it fails.
    int p = setpriority(PRIO_PROCESS, mainTid, -4);
    LOGI("setpriority result %d", p);
}

__attribute__((constructor)) static void onLoad() {
    applyBoost();
}
