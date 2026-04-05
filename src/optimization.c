/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Lcaol Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include "instance.h"
#include "optimization.h"
#include "utilities.h"

#ifdef __MINGW32__
#include <float.h>
#define MAX_FLOAT FLT_MAX
#else
#define MAX_FLOAT MAXFLOAT
#endif

long int **CostMat;

long long int computeCost(long int *s) {
  int h, k;
  long long int sum = 0;

  /* Diagonal value are not considered */
  for (sum = 0, h = 0; h < PSize; h++)
    for (k = h + 1; k < PSize; k++)
      sum += CostMat[s[h]][s[k]];
  return (sum);
}

void createRandomSolution(long int *s) {
  int j;
  long int *random;

  random = generate_random_vector(PSize);
  for (j = 0; j < PSize; j++) {
    s[j] = random[j];
  }
  free(random);
}

int pivot_first(long int **matrix) {
  DPRINTF("executing pivot_first\n");
  return 0;
}

int pivot_best(long int **matrix) {
  DPRINTF("executing pivot_best\n");
  return 0;
}

ulong get_n_transpose(uint size) {

  if (size < 2) {
    return 0;
  }
  DPRINTF("get_n_transpose: for a %u array, there is %u transpose\n", size,
          (size));
  return size;
}

void get_transpose(t_sizemat *transposes, t_sizemat n_collumns, ulong n_rows) {

  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_collumns; j++) {
      transposes[n_collumns * i + j] = j;
    }
    t_sizemat temp = transposes[n_collumns * i + i];
    transposes[n_collumns * i + i] = transposes[n_collumns * i + i + 1];
    transposes[n_collumns * i + (i + 1) % n_collumns] = temp;
  }

#ifndef NDEBUG
  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_collumns; j++) {
      printf("%ld ", transposes[n_collumns * i + j]);
    }
    printf("\n");
  }
#endif
}

int neighborhood_tranpose(int a, int b, long int **matrix) {
  DPRINTF("executing neighborhood_tranpose\n");
  DPRINTF("allocating memory for transposes possibilities\n");
  t_sizemat n_collumns = 10;
  ulong n_rows = get_n_transpose(n_collumns);
  t_sizemat *transposes = malloc(n_rows * n_collumns * sizeof(t_sizemat));
  DPRINTF("%lu transoposes possibilities allocated\n", n_rows);

  get_transpose(transposes, n_collumns, n_rows);

  return 0;
}

uint get_n_exchange(t_sizemat size) {
  if (size < 2) {
    return 0;
  }
  return size * (size - 1) / 2;
}

void get_exchange(t_sizemat *exchanges, t_sizemat n_collumns, uint n_rows) {

  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_collumns; j++) {
      exchanges[n_collumns * i + j] = j;
    }
  }

  uint row = 0;
  for (t_sizemat i = 0; i < n_collumns - 1; i++) {
    for (t_sizemat j = i + 1; j < n_collumns; j++) {
      t_sizemat temp = exchanges[n_collumns * row + i];
      exchanges[n_collumns * row + i] = exchanges[n_collumns * row + j];
      exchanges[n_collumns * row + j] = temp;
      row++;
    }
  }

#ifndef NDEBUG
  DPRINTF("executing for %ld collumns and %ud rows=\n", n_collumns, n_rows);
  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_collumns; j++) {
      printf("%ld ", exchanges[n_collumns * i + j]);
    }
    printf("\n");
  }
#endif
}

int neighborhood_exchange(int a, int b, long int **matrix) {
  DEBUG_PRINT("executing neighborhood_exchange");
  t_sizemat n_collumns = 5;
  ulong n_rows = get_n_exchange(n_collumns);
  t_sizemat *exchanges = malloc(n_rows * n_collumns * sizeof(t_sizemat));
  DPRINTF("%lu exchanges possibilities allocated\n", n_rows);

  get_exchange(exchanges, n_collumns, n_rows);

  return 0;
}

ulong get_n_inserts(uint size) {
  if (size < 2) {
    return 0;
  } else if (size == 2) {
    return 3;
  }
  DPRINTF("get_n_insert: for a %u array, there is %u insert\n", size,
          ((size - 1) * (size - 1)));

  return (size - 1) * (size - 1);
}

void get_inserts(t_sizemat *insertions, t_sizemat n_collumns, ulong n_rows) {

  DPRINTF("%s: executing for %ld collumns and %ld rows=\n", __func__,
          n_collumns, n_rows);

  ulong row = 0;

  // which number has to move
  for (t_sizemat i = 0; i < n_collumns; i++) {
    // to which number i has to move
    for (t_sizemat j = 0; j < n_collumns; j++) {
      // avoid duplicates
      if (i != j && i != j + 1) {
        // go through each number of a row
        for (t_sizemat x = 0; x < n_collumns; x++) {
          // "move"" number to the left
          if (i < j && x >= i && x < j) {
            insertions[n_collumns * row + x] = x + 1;
          } else if (i > j && x > j && x <= i) { // "move" number to the right
            insertions[n_collumns * row + x] = x - 1;
          } else if (x == j) { // put i to j
            insertions[n_collumns * row + x] = i;
          } else {
            insertions[n_collumns * row + x] = x;
          }
        }
#ifndef NDEBUG
        for (t_sizemat x = 0; x < n_collumns; x++) {
          printf("%ld ", insertions[n_collumns * i + x]);
        }
        printf(" for i=%ld, j=%ld\n", i, j);
#endif
        row++;
      }
    }
  }
}

int neighborhood_insert(int a, int b, long int **matrix) {
  DEBUG_PRINT("executing neighborhood_insert");

  // allocate malloc for the number of insertions possible.
  t_sizemat n_collumns = PSize;
  ulong n_rows = get_n_inserts(n_collumns);
  t_sizemat *insertions = malloc(n_rows * n_collumns * sizeof(t_sizemat));

  get_inserts(insertions, n_collumns, n_rows);
  printf("for %ld collumns; total number of insert: %lu\n", n_collumns, n_rows);

  return 0;
}

t_sizemat *sol_start_random(t_mat_cell **mat, t_sizemat n_collumns) {
  DEBUG_PRINT("executing sol_start_random");

  t_sizemat *new_random_vector = generate_random_vector(n_collumns);

#ifndef NDEBUG
  DPRINTF("random generated starting solution vector\n")
  for (t_sizemat i = 0; i < n_collumns; i++) {
    printf("%lu ", new_random_vector[i]);
  }
  printf("\n");
#endif

  return new_random_vector;
}

t_mat_cell *prefix_sum_per_row_2d(t_mat_cell **mat, t_sizemat n_rows,
                                  t_sizemat n_collumns) {

  /* Make a array of sum of another array like :
   * 0 1 2 3 4 5 6 7 8 9
   * after sum :
   * 0 1 3 6 10 17 25 34
   * */
  t_mat_cell *sum_row_2d = malloc(n_collumns * n_collumns * sizeof(t_mat_cell));
  for (t_sizemat i = 0; i < n_collumns; i++) {
    sum_row_2d[n_collumns * i] = mat[i][0];
    for (t_sizemat j = 1; j < n_collumns; j++) {
      sum_row_2d[n_collumns * i + j] =
          sum_row_2d[n_collumns * i + j - 1] + mat[i][j];
    }
  }

  return sum_row_2d;
}

/**
 * @brief Chenery and Watanabe (CW) heuristic.
 *
 * @param mat Cost matrix.
 * @param n_collumns Dimension of the square matrix
 * @return A array of the CW ordering.
 */
t_sizemat *sol_start_c_and_w(t_mat_cell **mat, t_sizemat n_collumns) {
  DEBUG_PRINT("executing sol_start_c_and_w\n");

  t_mat_cell *sum_row_2d = prefix_sum_per_row_2d(mat, n_collumns, n_collumns);

#ifndef NDEBUG
  DPRINTF("cost matrix:\n");
  print_array_2d2(mat, n_collumns, n_collumns);
  DPRINTF("Prefix sum of matrix:\n");
  print_array_2d(sum_row_2d, n_collumns, n_collumns);
#endif

  t_sizemat *new_best_start_1d = generate_inc_vector(n_collumns);

  for (t_sizemat i = 0; i < n_collumns; i++) {
    t_sizemat best_pos = i;
    t_mat_cell best = 0;
    DPRINTF("sum for i=%ld : ", i);
    for (t_sizemat j = i; j < n_collumns; j++) {
      t_sizemat sum_row_idx = new_best_start_1d[j];

      // current = mat[i][last] - mat[i][i]
      t_mat_cell last_sum =
          sum_row_2d[n_collumns * sum_row_idx + n_collumns - 1];
      t_mat_cell i_sum = sum_row_2d[n_collumns * sum_row_idx + i];

      t_mat_cell current = last_sum - i_sum;

#ifndef NDEBUG
      printf("%ld ", current);
#endif

      if (current >= best) {
        best_pos = j;
        best = current;
      }
    }
    DNPRINTF("\n");
    DPRINTF("best at %ld : %lu for value %lu\n", i, best_pos, best);

    t_sizemat tmp = new_best_start_1d[i];
    new_best_start_1d[i] = new_best_start_1d[best_pos];
    new_best_start_1d[best_pos] = tmp;

    DPRINTF("new_best_start_1d: ");
#ifndef NDEBUG
    print_array_1d(new_best_start_1d, n_collumns);
#endif
  }

#ifndef NDEBUG
  DPRINTF("C_and_W solution\n");
  print_array_1d(new_best_start_1d, n_collumns);
#endif

  free(sum_row_2d);
  return new_best_start_1d;
}

void lop(t_fptr_sol_start fptr_sol_start, t_fptr_pivot_rule fptr_pivot_rule,
         t_fptr_neighborhood fptr_neighborhood) {}
