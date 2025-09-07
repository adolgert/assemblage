#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <time.h>

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

void pin_thread(void)
{
    int current_cpu = sched_getcpu();
    if (current_cpu == -1)
    {
        perror("sched_getcpu failed");
        return;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(current_cpu, &cpuset);

    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1)
    {
        perror("sched_setaffinity failed");
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
void longadd(int x, int y, unsigned long long *duration, long long* ns)
{
    struct timespec start, end;
    unsigned long long timing = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    timing = rdtsc();
    register int dest asm("eax") = x;
    register int src asm("ecx") = y;
    MULTIPLY10000(dest, src);
    timing = rdtsc() - timing;
    clock_gettime(CLOCK_MONOTONIC, &end);
    // long microseconds = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    *ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    *duration = timing;
}

// # cpu family 25, model 33.
// # AMD Ryzen 9 5900X 12-Core Processor
// # 550 MHz
int main(int argc, char** argv)
{
    unsigned long long timing;
    long long nanoseconds;
    double cycles_per_ns;
    unsigned int base, max, ref;
    get_frequencies(&base, &max, &ref);
    printf("Frequencies base=%u max=%u, ref=%u\n", base, max, ref);
    for (int burn_idx=0; burn_idx < 10000000; burn_idx++) {
        longadd(42, 3, &timing, &nanoseconds);
    }
    longadd(42, 3, &timing, &nanoseconds);
    cycles_per_ns = timing;
    cycles_per_ns /= nanoseconds;
    printf("Time: %llu\n", timing);
    printf("%llu ns\n", nanoseconds);
    printf("Cycles per ns %f\n", cycles_per_ns);
    return 0;
}
