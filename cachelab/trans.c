/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include <assert.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);



/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, k, s, a0, a1, a2, a3, a4, a5, a6, a7;
    const int len = 8;
    if (N == 32) {
         for (i = 0; i < N; i += len) {
            for (j = 0; j < M; j += len) {
                for (k = 0; k < len; k++) {
                    a0 = A[i + k][j];
                    a1 = A[i + k][j + 1];
                    a2 = A[i + k][j + 2];
                    a3 = A[i + k][j + 3];
                    a4 = A[i + k][j + 4];
                    a5 = A[i + k][j + 5];
                    a6 = A[i + k][j + 6];
                    a7 = A[i + k][j + 7];
                    B[j + k][i] = a0;
                    B[j + k][i + 1] = a1;
                    B[j + k][i + 2] = a2;
                    B[j + k][i + 3] = a3;
                    B[j + k][i + 4] = a4;
                    B[j + k][i + 5] = a5;
                    B[j + k][i + 6] = a6;
                    B[j + k][i + 7] = a7;
                }
                 // tranpose part of B at position (i, j) of B
                 for (k = 0; k < len; k++) {
                    for (s = k + 1; s < len; s++) {
                        a0 = B[j + k][i + s];
                        B[j + k][i + s] = B[j + s][i + k];
                        B[j + s][i + k] = a0;
                    }
                 }
            }
        }
    }
    //assert(is_transpose(M, N, A, B));
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    //registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

