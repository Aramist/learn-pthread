#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "randtools.h"

#define NUM_TRIALS 10

void matmul_naive(float *M, float *N, float *R, int m, int k, int n);
double timediff(struct timespec a, struct timespec b);

int main(void) {
  float *M, *N, *R;
  double runs[NUM_TRIALS];
  double avg = 0.0;
  size_t m = 4096, k = 64, n = 4096;

  srand((unsigned int)time(NULL));
  M = (float *)malloc(sizeof(float) * m * k);
  N = (float *)malloc(sizeof(float) * k * n);
  R = (float *)malloc(sizeof(float) * m * n);

  for (int i = 0; i < NUM_TRIALS; i++) {
    struct timespec start, end;
    random_matrix(M, m, k);
    random_matrix(N, k, n);

    clock_gettime(CLOCK_MONOTONIC, &start);
    matmul_naive(M, N, R, m, k, n);
    clock_gettime(CLOCK_MONOTONIC, &end);

    runs[i] = timediff(end, start);
    printf("Trial %d: %.2fsec\n", i + 1, runs[i]);
    fflush(stdout);
  }

  for (int i = 0; i < NUM_TRIALS; i++) {
    avg += runs[i];
  }
  avg /= NUM_TRIALS;
  avg *= 1000;

  printf("Ran %d times with an average runtime of %.1fms\n", NUM_TRIALS, avg);

  free(M);
  free(N);
  free(R);
  return EXIT_SUCCESS;
}

void matmul_naive(float *M, float *N, float *R, int m, int k, int n) {
  /**
   * Multiplies an mxk matrix `M` to a kxn matrix `N`, storing the result
   * in `R`
   */

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      // Compute R_i,j
      float rij = 0;
      for (int inner = 0; inner < k; inner++)
        rij += M[i * k + inner] * N[inner * n + j];
      R[i * n + j] = rij;
    }
  }
}

double timediff(struct timespec a, struct timespec b) {
  double secs = a.tv_sec - b.tv_sec;
  double nsecs = a.tv_nsec * 1e-9 - b.tv_nsec * 1e-9;
  return secs + nsecs;
}
