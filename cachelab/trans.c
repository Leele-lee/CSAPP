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
    if (N == 64) {
        for (j = 0; j < N; j += len) {
            if (j == 0) i = 8; else i = 0;
            //deal with diagonal block at [j, j]
            //copy the down part of A into the up part of borrow block 
            for (k = 0; k < 4; k++) {
                a0 = A[j + k + 4][j];
                a1 = A[j + k + 4][j + 1];
                a2 = A[j + k + 4][j + 2];
                a3 = A[j + k + 4][j + 3];
                a4 = A[j + k + 4][j + 4];
                a5 = A[j + k + 4][j + 5];
                a6 = A[j + k + 4][j + 6];
                a7 = A[j + k + 4][j + 7];

                B[j + k][i] = a0;
                B[j + k][i + 1] = a1;
                B[j + k][i + 2] = a2;
                B[j + k][i + 3] = a3;
                B[j + k][i + 4] = a4;
                B[j + k][i + 5] = a5;
                B[j + k][i + 6] = a6;
                B[j + k][i + 7] = a7;
            }

            // transpose upper left and upper right of borrowing block itself respectivly [j, i] 
            for (k = 0; k < 4; k++) {
                for (s = k + 1; s < 4; s++) {
                    a0 = B[j + k][i + s];
                    B[j + k][i + s] = B[j + s][i + k];
                    B[j + s][i + k] = a0;

                    a0 = B[j + k][i + s + 4];
                    B[j + k][i + s + 4] = B[j + s][i + k + 4];
                    B[j + s][i + k + 4] = a0;
                }
            }

            //copy the up part 4x8 of A into the up part of B 
            for (k = 0; k < 4; k++) {
                a0 = A[j + k][j];
                a1 = A[j + k][j + 1];
                a2 = A[j + k][j + 2];
                a3 = A[j + k][j + 3];
                a4 = A[j + k][j + 4];
                a5 = A[j + k][j + 5];
                a6 = A[j + k][j + 6];
                a7 = A[j + k][j + 7];

                B[j + k][j] = a0;
                B[j + k][j + 1] = a1;
                B[j + k][j + 2] = a2;
                B[j + k][j + 3] = a3;
                B[j + k][j + 4] = a4;
                B[j + k][j + 5] = a5;
                B[j + k][j + 6] = a6;
                B[j + k][j + 7] = a7;
            }

            // transpose upper left and right 4x4 of B itself respectivly [j, j] 
            for (k = 0; k < 4; k++) {
                for (s = k + 1; s < 4; s++) {
                    a0 = B[j + k][j + s];
                    B[j + k][j + s] = B[j + s][j + k];
                    B[j + s][j + k] = a0;

                    a0 = B[j + k][j + s + 4];
                    B[j + k][j + s + 4] = B[j + s][j + k + 4];
                    B[j + s][j + k + 4] = a0;
                }
            }

            // swaping the lower-left and upper-right
            for (k = 0; k < 4; k++) {
                a0 = B[j + k][j + 4];
                a1 = B[j + k][j + 5];
                a2 = B[j + k][j + 6];
                a3 = B[j + k][j + 7];

                B[j + k][j + 4] = B[j + k][i];
                B[j + k][j + 5] = B[j + k][i + 1];
                B[j + k][j + 6] = B[j + k][i + 2];
                B[j + k][j + 7] = B[j + k][i + 3];

                B[j + k][i] = a0;
                B[j + k][i + 1] = a1;
                B[j + k][i + 2] = a2;
                B[j + k][i + 3] = a3;
            }

            // copy the entire upper of blocking shifting parts into the lower part of B  
            for (k = 0; k < 4; k++) {
                B[j + k + 4][j] = B[j + k][i];
                B[j + k + 4][j + 1] = B[j + k][i + 1];
                B[j + k + 4][j + 2] = B[j + k][i + 2];
                B[j + k + 4][j + 3] = B[j + k][i + 3];
                B[j + k + 4][j + 4] = B[j + k][i + 4];
                B[j + k + 4][j + 5] = B[j + k][i + 5];
                B[j + k + 4][j + 6] = B[j + k][i + 6];
                B[j + k + 4][j + 7] = B[j + k][i + 7];
            }

            //for non-diagnoal block A and B is not correspond, A is reading cloumn-first
            // B is row-first, A start with [j,i] is corresponding B with [i, j]
            for (i = 0; i < M; i += len) {
                if (i == j) {
                    continue;
                } else {
                    // copy transposing the upper left and right 4x4 part from A to B
                    for (k = 0; k < 4; k++) {
                        a0 = A[i + k][j];
                        a1 = A[i + k][j + 1];
                        a2 = A[i + k][j + 2];
                        a3 = A[i + k][j + 3];
                        a4 = A[i + k][j + 4];
                        a5 = A[i + k][j + 5];
                        a6 = A[i + k][j + 6];
                        a7 = A[i + k][j + 7];

                        B[j][i + k] = a0;
                        B[j + 1][i + k] = a1;
                        B[j + 2][i + k] = a2;
                        B[j + 3][i + k] = a3;

                        B[j][i + k + 4] = a4;
                        B[j + 1][i + k + 4] = a5;
                        B[j + 2][i + k + 4] = a6;
                        B[j + 3][i + k + 4] = a7;
                    }
                    // taking transpose of lower-left 4x4 of A and store to upper-right 4x4 of B, 
                    // and move upper-right 4x4 of B to lower-left 4x4 of B
                    for (k = 0; k < 4; k++) {
                        a0 = A[i + 4][j + k];
                        a1 = A[i + 5][j + k];
                        a2 = A[i + 6][j + k];
                        a3 = A[i + 7][j + k];

                        a4 = B[j + k][i + 4];
                        a5 = B[j + k][i + 5];
                        a6 = B[j + k][i + 6];
                        a7 = B[j + k][i + 7];

                        B[j + k][i + 4] = a0;
                        B[j + k][i + 5] = a1;
                        B[j + k][i + 6] = a2;
                        B[j + k][i + 7] = a3;

                        B[j + k + 4][i] = a4;
                        B[j + k + 4][i + 1] = a5;
                        B[j + k + 4][i + 2] = a6;
                        B[j + k + 4][i + 3] = a7;
                    }

                    // taking transpose of lower-right 4x4 of A to B
                    for (k = 0; k < 4; k++) {
                        a0 = A[i + 4 + k][j + 4];
                        a1 = A[i + 4 + k][j + 5];
                        a2 = A[i + 4 + k][j + 6];
                        a3 = A[i + 4 + k][j + 7];

                        B[j + 4][i + k + 4] = a0;
                        B[j + 5][i + k + 4] = a1;
                        B[j + 6][i + k + 4] = a2;
                        B[j + 7][i + k + 4] = a3;

                    }

                }

            }
        }
    }
    if (M == 61) {
        for (i = 0; i < N; i += 16) {
            for (j = 0; j < M; j += 16) {
                for (k = j; (k < j + 16) && (k < M); k++) {
                    for (s = i; (s < i + 16) && (s < N); s++) {
                        B[k][s] = A[s][k];
                    }
                }
            }
        }
    }
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

