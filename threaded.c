#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "randtools.h"

#define NUM_TRIALS 10
#define NUM_THREADS 8

typedef struct {
  int m;
  int n;
  float *data;
} matrix;

typedef struct {
  matrix *L;
  matrix *R;
  matrix *result;
} matmul_args;

void matmul(float *L, float *R, float *out, int m, int k, int n);
void *matmul_threaded(void *data);
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
    matmul(M, N, R, m, k, n);
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

void matmul(float *L, float *R, float *out, int m, int k, int n) {
  /** Parallelize the matrix multiplication by chunking the lhs rows
   * and using all cols in the rhs per thread
   *
   * Assumes the matrix size is divisible by the number of threads used
   */

  int blockSize;
  matrix *right, *result;
  matrix *lMatrices[NUM_THREADS];
  matrix *resultMatrices[NUM_THREADS];
  matmul_args *argsArray[NUM_THREADS];
  pthread_t threadPool[NUM_THREADS];

  blockSize = m / NUM_THREADS;
  right = malloc(sizeof(matrix));
  right->m = k;
  right->n = n;
  right->data = R;

  for (int block_idx = 0; block_idx < NUM_THREADS; block_idx++) {
    matrix *left, *result;
    matmul_args *args;
    int i_start = block_idx * blockSize;

    left = malloc(sizeof(matrix));
    left->m = blockSize;
    left->n = k;
    left->data = L + i_start * k;

    result = malloc(sizeof(matrix));
    result->m = m;
    result->n = n;
    result->data = out + i_start * k;

    args = (matmul_args *)malloc(sizeof(matmul_args));
    args->L = left;
    args->R = right;
    args->result = result;
    lMatrices[block_idx] = left;
    resultMatrices[block_idx] = result;
    argsArray[block_idx] = args;

    pthread_create(&threadPool[block_idx], NULL, &matmul_threaded,
                   (void *)args);
  }

  // Join all threads
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(threadPool[i], NULL);

  // Release memmory
  for (int i = 0; i < NUM_THREADS; i++) {
    free(lMatrices[i]);
    free(resultMatrices[i]);
    free(argsArray[i]);
  }

  free(right);
}

void *matmul_threaded(void *data) {
  matmul_args *args;
  matrix *L, *R, *result;
  int m, k, n;

  args = (matmul_args *)data;
  L = args->L;
  R = args->R;
  result = args->result;

  m = L->m;
  k = L->n;
  n = R->n;

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      float sum = 0.0;
      for (int inner = 0; inner < k; inner++)
        sum += L->data[i * k + inner] * R->data[inner * n + j];
      result->data[i * n + j] = sum;
    }
  }

  return NULL;
}

double timediff(struct timespec a, struct timespec b) {
  double secs = a.tv_sec - b.tv_sec;
  double nsecs = a.tv_nsec * 1e-9 - b.tv_nsec * 1e-9;
  return secs + nsecs;
}
