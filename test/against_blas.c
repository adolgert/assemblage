#include <stdlib.h>
#include <string.h>

#include <cblas.h>


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
    for (int bi = 0; bi < m * n; ++bi) b[bi] = randreal();
    memset(c, 0, sizeof(double) * m * p);

    /*
     * cblas_dgemm performs C = alpha * A * B + beta * C
     * 
     * Parameters:
     * - Order: CblasRowMajor or CblasColMajor (memory layout)
     * - TransA: CblasNoTrans, CblasTrans, CblasConjTrans (whether to transpose A)
     * - TransB: CblasNoTrans, CblasTrans, CblasConjTrans (whether to transpose B)
     * - M: number of rows in A and C (after any transpose)
     * - N: number of columns in B and C (after any transpose)
     * - K: number of columns in A and rows in B (after any transpose)
     * - alpha: scalar multiplier for A*B
     * - A: pointer to matrix A
     * - lda: leading dimension of A (stride between rows in row-major, or cols in col-major)
     * - B: pointer to matrix B
     * - ldb: leading dimension of B (stride between rows in row-major, or cols in col-major)
     * - beta: scalar multiplier for C (use 0.0 to overwrite C, 1.0 to add to existing C)
     * - C: pointer to matrix C (output)
     * - ldc: leading dimension of C (stride between rows in row-major, or cols in col-major)
     *
     * Leading dimension (ld*): The actual allocated width of the matrix in memory.
     * This allows for matrices that are sub-matrices of larger allocated arrays.
     * For contiguous matrices: lda >= K (cols of A), ldb >= N (cols of B), ldc >= N (cols of C)
     */
    cblas_dgemm(
        CblasRowMajor, // Order
        CblasNoTrans,  // TransA
        CblasNoTrans,  // TransB
        m,             // M
        p,             // N
        n,             // K
        1.0,           // alpha
        a,             // A
        n,             // lda
        b,             // B
        p,             // ldb
        0.0,           // beta
        c,             // C
        p              // ldc
    );
    free(c);
    free(b);
    free(a);
}
