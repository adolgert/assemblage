#include <stdlib.h>
#include <string.h>

void naive(size_t m, size_t n, size_t k, double* __restrict a, double* __restrict b, double* __restrict c);

double randreal() {
    int half = RAND_MAX >> 1;
    return (rand() - half) / half;
}


int main() {
    srand(0x2644782a);


    // A (2x3) * B (3x5) = C (2x5)
    int m = 2, n = 3, p = 5;

    double* a = malloc(sizeof(double) * m * n);
    double* b = malloc(sizeof(double) * n * p);
    double* c = malloc(sizeof(double) * m * p);

    for (int ai = 0; ai < m * n; ++ai) a[ai] = randreal();
    for (int bi = 0; bi < n * p; ++bi) b[bi] = randreal();
    memset(c, 0, sizeof(double) * m * p);

    naive(m, p, n, a, b, c);

    free(a);
    free(b);
    free(c);
    
    return 0;
}
