#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

unsigned long long rdtsc()
{
    unsigned a, d;

    __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

    return ((unsigned long long) a | ((unsigned long long) d) << 32);
}

#define N 10
#define k 4   // # of 64-bit words

uint64_t snp_array[N*k];
uint64_t ld_array[N*N];

// initialize the array randomly
void init_arrays()
{
    srandom(42u);  // reproducibility

    for (size_t ix = 0; ix < N*k; ++ix)
    {
        snp_array[ix] =
            ((uint64_t)random() + (((uint64_t)random()) << 32)) &
            ((uint64_t)random() + (((uint64_t)random()) << 32));
    }

    bzero(&ld_array[0], N*N*8);
}

uint64_t popcount(uint64_t val)
{
    uint64_t cnt = 0;
    for (int i = 0; i < 64; ++i)
        if (val & (1ul << i)) ++cnt;
    return cnt;
}

void kernel(uint64_t *out, uint64_t A, uint64_t B)
{
    uint64_t t = popcount(A&B);
    fprintf(stdout, "popcount(%08lx & %08lx) = popcount(%08lx) = %ld\n",
            A,B, A&B, t);
    *out += t;
}

void driver(uint64_t *snp_array, uint64_t *ld_array,
            unsigned num_snps, unsigned num_words)
{
    for (unsigned ia = 0; ia < num_snps; ++ia)
    {
        for (unsigned ib = 0; ib < num_snps; ++ib)
        {
            for (unsigned ik = 0; ik < num_words; ++ik)
            {
                kernel(&ld_array[ia*num_snps + ib],
                       snp_array[ia*num_words + ik],
                       snp_array[ib*num_words + ik]);
            }
        }
    }
}

int main()
{
    init_arrays();

    for (unsigned int ix = 0; ix < N; ++ix)
    {
        fprintf(stdout, "%4d:", ix);
        for (unsigned int iy = 0; iy < k; ++iy)
        {
            fprintf(stdout, " %016lx", snp_array[ix*k + iy]);
        }
        fprintf(stdout, "\n");
    }

    driver(snp_array, ld_array, N, k);

    fprintf(stdout, "\n");
    for (unsigned int ix = 0; ix < N; ++ix)
    {
        fprintf(stdout, "%4d:", ix);
        for (unsigned int iy = 0; iy < N; ++iy)
        {
            fprintf(stdout, " %4ld", ld_array[ix*N + iy]);
        }
        fprintf(stdout, "\n");
    }
}

// SNP kernel
