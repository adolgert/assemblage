#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

unsigned long long rdtsc()
{
    unsigned a, d;

    __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

    return ((unsigned long long) a | ((unsigned long long) d) << 32);
}

void get_frequencies(unsigned int *base, unsigned int *maximum, unsigned int *bus_reference)
{
    unsigned int max_leaf, ebx, ecx, edx;

    // First check if leaf 0x16 is supported
    __asm__ volatile("cpuid"
                     : "=a"(max_leaf), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(0x0), "c"(0));

    if (max_leaf < 0x16)
    {
        // Leaf 0x16 not supported
        *base = 0;
        *maximum = 0;
        *bus_reference = 0;
        return;
    }

    // Query frequency information using leaf 0x16
    __asm__ volatile("cpuid"
                     : "=a"(*base), "=b"(*maximum), "=c"(*bus_reference), "=d"(edx)
                     : "a"(0x16), "c"(0));

    // Extract frequency values from lower 16 bits (frequencies are in MHz)
    *base &= 0xFFFF;
    *maximum &= 0xFFFF;
    *bus_reference &= 0xFFFF;
}

int pin_thread(void)
{
    int current_cpu = sched_getcpu();
    if (current_cpu == -1)
    {
        perror("sched_getcpu failed");
        return -1;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(current_cpu, &cpuset);

    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1)
    {
        perror("sched_setaffinity failed");
        return -1;
    }
    
    return current_cpu;
}

unsigned int get_current_frequency(int cpu_id)
{
    char path[256];
    FILE *fp;
    unsigned int freq_khz = 0;
    
    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu_id);
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Failed to open scaling_cur_freq");
        return 0;
    }
    
    if (fscanf(fp, "%u", &freq_khz) != 1) {
        perror("Failed to read frequency");
        freq_khz = 0;
    }
    
    fclose(fp);
    return freq_khz;
}

/* Get frequency from a kernel module. Requires root.
*/
uint64_t read_msr(int cpu_id, uint32_t msr_register)
{
    char path[256];
    int fd;
    uint64_t value = 0;
    
    snprintf(path, sizeof(path), "/dev/cpu/%d/msr", cpu_id);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open MSR device (try as root or load msr module)");
        return 0;
    }
    
    if (pread(fd, &value, sizeof(value), msr_register) != sizeof(value)) {
        perror("Failed to read MSR");
        value = 0;
    }
    
    close(fd);
    return value;
}

void check_msr_frequency(int cpu_id)
{
    uint64_t perf_status, platform_info;
    
    // Read IA32_PERF_STATUS (0x198) - Current P-state
    perf_status = read_msr(cpu_id, 0x198);
    if (perf_status != 0) {
        uint32_t current_ratio = (perf_status >> 8) & 0xFF;
        printf("MSR IA32_PERF_STATUS: 0x%lx, Current ratio: %u\n", perf_status, current_ratio);
    }
    
    // Read MSR_PLATFORM_INFO (0xCE) - TSC/Bus frequency info
    platform_info = read_msr(cpu_id, 0xCE);
    if (platform_info != 0) {
        uint32_t max_non_turbo_ratio = (platform_info >> 8) & 0xFF;
        printf("MSR_PLATFORM_INFO: 0x%lx, Max non-turbo ratio: %u\n", platform_info, max_non_turbo_ratio);
    }
}

#define MULTIPLY(dest, src) \
    __asm__ volatile( \
        "imul %[rsrc], %[rdest]\n" \
        : [rdest] "+r"(dest) \
        : [rsrc] "r" (src) \
    );


#define MULTIPLY10(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y) \
    MULTIPLY(x, y)

#define MULTIPLY100(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y) \
    MULTIPLY10(x, y)

#define MULTIPLY1000(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y) \
    MULTIPLY100(x, y)

#define MULTIPLY10000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y) \
    MULTIPLY1000(x, y)

/*
   Keys to making this work: ensure x and y are both arguments, not constants
   in the function, because the compiler will always use a constant value in
   the assembly instead of a variable.
   Use register ints which say they are in eax and ecx.
   Experiment with -O0, -O1, -O2. Here O1 did better.
*/
void longmul(int x, int y, unsigned long long *duration, long long* ns)
{
    struct timespec start, end;
    unsigned long long timing = 0;

    // Comparing clock_gettime with rdtsc will always tell you the base
    // frequency, which is 3.68 for this CPU.
    clock_gettime(CLOCK_MONOTONIC, &start);
    // Putting variables into registers helps compiler stop copying from parameters.
    register int dest asm("eax") = x;
    register int src asm("ecx") = y;
    MULTIPLY1000(dest, src); // warm-up
    timing = rdtsc();
    MULTIPLY10000(dest, src);
    timing = rdtsc() - timing;
    clock_gettime(CLOCK_MONOTONIC, &end);
    // long microseconds = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    *ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    *duration = timing;
}

// # cpu family 25, model 33.
// # AMD Ryzen 9 5900X 12-Core Processor
// # 550 MHz when idle. Base freq 3.68 GHz,
// $ ./dependent
// pinned to 23
// Frequencies base=0 max=0, ref=0
// Time: 24420
// 7271 ns
// Cycles per ns 3.358548 (this number will be wrong by a bit).
// Frequences start 4331434 midpoint 4550001 end 4549957
// pipeline depth 3.003001

int main()
{
    unsigned long long timing;
    long long nanoseconds;
    double cycles_per_ns;
    unsigned int base, max, ref;
    unsigned int start_freq, mid_freq, end_freq;
    int cpu_id, asroot;
    double cycles_per_mul;

    asroot = (geteuid() == 0);
    cpu_id = pin_thread();
    printf("pinned to %d\n", cpu_id);
    start_freq = get_current_frequency(cpu_id);
    get_frequencies(&base, &max, &ref);
    printf("Frequencies base=%u max=%u, ref=%u\n", base, max, ref);
    // Warm up the grill.
    for (int burn_idx=0; burn_idx < 1000000; burn_idx++) {
        longmul(42, 3, &timing, &nanoseconds);
    }
    mid_freq = get_current_frequency(cpu_id);
    for (int burn_idx=0; burn_idx < 1000000; burn_idx++) {
        longmul(42, 3, &timing, &nanoseconds);
    }
    // Now measure.
    longmul(42, 3, &timing, &nanoseconds);
    if (asroot) {
        check_msr_frequency(cpu_id);
    }
    end_freq = get_current_frequency(cpu_id);
    cycles_per_ns = timing;
    cycles_per_ns /= nanoseconds;
    printf("Time: %llu\n", timing);
    printf("%llu ns\n", nanoseconds);
    printf("Cycles per ns %f\n", cycles_per_ns);
    printf("Frequences start %u midpoint %u end %u\n", start_freq, mid_freq, end_freq);

    cycles_per_mul = timing;
    cycles_per_mul *= mid_freq; // Core frequency in a hot loop.
    cycles_per_mul /= 3700000.0; // Base frequency should be 3.7 GHz
    cycles_per_mul /= 10000.0; // Doing 10,000 imuls in the loop.
    printf("pipeline depth %f\n", cycles_per_mul);
    return 0;
}
