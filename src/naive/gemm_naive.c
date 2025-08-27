#include <stdlib.h>

// Row-major with these assumed dimensions.
// A (2x3) * B (3x5) = C (2x5)
// int m = 2, n = 3, p = 5;
void naive(size_t m, size_t n, size_t p, const double* __restrict a, const double* __restrict b, double* __restrict c) {
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < p; ++j) {
            for (size_t k = 0; k < n; ++k) {
                c[i * p + j] += a[i * n + k] * b[k * p + j];
            }
        }
    }
}
