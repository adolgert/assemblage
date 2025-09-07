#include <stdio.h>
#include <time.h>

unsigned long long rdtsc()
{
    unsigned a, d;

    __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

    return ((unsigned long long) a | ((unsigned long long) d) << 32);
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
    int dest = x;
    int src = 2;
    MULTIPLY10000(dest, src);
    timing = rdtsc() - timing;
    clock_gettime(CLOCK_MONOTONIC, &end);
    long microseconds = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("%lu ms\n", microseconds);
    return timing;
}


int main(int argc, char** argv)
{
    unsigned long long timing;
    timing = longadd(42);
    printf("Time: %llu\n", timing);
    return 0;
}
