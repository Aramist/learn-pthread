#include "randtools.h"

float rand_float() { return (float)rand() / (float)RAND_MAX; }

void random_matrix(float *M, int m, int n) {
  /** Generates a matrix with random elements and stores into `M`
   * The matrix has dimensions mxn
   */
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      M[i * n + j] = rand_float();
    }
  }
}
