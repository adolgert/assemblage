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

unsigned long long longadd(int x)
{
    struct timespec start, end;
    unsigned long long timing = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    timing = rdtsc();
    register int dest asm("eax") = x;
    register int src asm("ecx") = 3;
    MULTIPLY10000(dest, src);
    timing = rdtsc() - timing;
    clock_gettime(CLOCK_MONOTONIC, &end);
    long microseconds = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    long long nanoseconds = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    printf("%llu ns\n", nanoseconds);
    printf("%lu ms\n", microseconds);
    return timing;
}

# cpu family 25, model 33.
# AMD Ryzen 9 5900X 12-Core Processor
# 550 MHz
int main(int argc, char** argv)
{
    unsigned long long timing;
    unsigned int base, max, ref;
    get_frequencies(&base, &max, &ref);
    printf("Frequencies base=%u max=%u, ref=%u\n", base, max, ref);
    timing = longadd(42);
    printf("Time: %llu\n", timing);
    return 0;
}
