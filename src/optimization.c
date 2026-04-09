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

#include <assert.h>
#include <limits.h>
#include <stddef.h>
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

t_cost computeCost(const t_mat_cell *const cost_mat_2d, t_sizemat *lo,
                   t_sizemat size) {
  DPRINTF("executing computeCost\n");
  int h, k;
  t_cost sum = 0;

  /* Diagonal value are not considered */
  for (sum = 0, h = 0; h < size; h++)
    for (k = h + 1; k < size; k++) {
      assert(MAX_COST - cost_mat_2d[size * lo[h] + lo[k]] >= sum);
      sum += cost_mat_2d[size * lo[h] + lo[k]];
    }
  DPRINTF("cost calculated : %du\n", sum);
  return (sum);
}

t_mat_cell *prefix_sum_per_row_2d(t_mat_cell *mat, t_sizemat n_rows,
                                  t_sizemat n_columns) {

  t_mat_cell *sum_row_2d = malloc(n_columns * n_columns * sizeof(t_mat_cell));
  for (t_sizemat i = 0; i < n_rows; i++) {
    sum_row_2d[n_columns * i] = mat[n_columns * i];
    for (t_sizemat j = 1; j < n_columns; j++) {
      assert(MAX_COST_CELL -
             sum_row_2d[n_columns * i + j - 1 >= mat[n_columns * i + j]]);
      sum_row_2d[n_columns * i + j] =
          sum_row_2d[n_columns * i + j - 1] + mat[n_columns * i + j];
    }
  }

  return sum_row_2d;
}

t_sizemat get_n_transpose(t_sizemat size) {

  if (size < 2) {
    return 0;
  }
  DPRINTF("get_n_transpose: for a %u array, there is %u transpose\n", size,
          (size));
  return size;
}

t_sizemat *neighb_transpose_deltas(t_sizemat *const n_rows,
                                   const t_sizemat n_columns) {

  DPRINTF("executing neighborhood_tranpose\n");

  *n_rows = get_n_transpose(n_columns);
  t_sizemat *transposes_deltas_2d =
      malloc(*n_rows * n_columns * sizeof(t_sizemat));

  DPRINTF("%u transoposes possibilities allocated\n", *n_rows);

  for (ulong i = 0; i < *n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      transposes_deltas_2d[n_columns * i + j] = j;
    }
    t_sizemat temp = transposes_deltas_2d[n_columns * i + i];
    transposes_deltas_2d[n_columns * i + i] =
        transposes_deltas_2d[n_columns * i + i + 1];
    transposes_deltas_2d[n_columns * i + (i + 1) % n_columns] = temp;
  }

#ifndef NDEBUG
  for (ulong i = 0; i < *n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      printf("%u ", transposes_deltas_2d[n_columns * i + j]);
    }
    printf("\n");
  }
#endif
  return transposes_deltas_2d;
}

uint get_n_exchange(t_sizemat size) {
  if (size < 2) {
    return 0;
  }
  return size * (size - 1) / 2;
}

t_sizemat *neighb_exchange_deltas(t_sizemat *const n_rows,
                                  const t_sizemat n_columns) {

  *n_rows = get_n_exchange(n_columns);
  t_sizemat *exchanges_deltas_2d =
      malloc(*n_rows * n_columns * sizeof(t_sizemat));

  DPRINTF("%ud exchanges possibilities allocated\n", *n_rows);

  for (ulong i = 0; i < *n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      exchanges_deltas_2d[n_columns * i + j] = j;
    }
  }

  uint row = 0;
  for (t_sizemat i = 0; i < n_columns - 1; i++) {
    for (t_sizemat j = i + 1; j < n_columns; j++) {
      t_sizemat temp = exchanges_deltas_2d[n_columns * row + i];
      exchanges_deltas_2d[n_columns * row + i] =
          exchanges_deltas_2d[n_columns * row + j];
      exchanges_deltas_2d[n_columns * row + j] = temp;
      row++;
    }
  }

#ifndef NDEBUG
  DPRINTF("executing for %u collumns and %ud rows=\n", n_columns, *n_rows);
  for (ulong i = 0; i < *n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      printf("%u ", exchanges_deltas_2d[n_columns * i + j]);
    }
    printf("\n");
  }
#endif

  return exchanges_deltas_2d;
}

t_sizemat get_n_inserts(t_sizemat size) {
  if (size < 2) {
    return 0;
  } else if (size == 2) {
    return 3;
  }
  DPRINTF("get_n_insert: for a %u array, there is %u insert\n", size,
          ((size - 1) * (size - 1)));

  return (size - 1) * (size - 1);
}

t_sizemat *neighb_insert_deltas(t_sizemat *const n_rows,
                                const t_sizemat n_columns) {

  DPRINTF("%s: executing for %u collumns and %d rows=\n", __func__, n_columns,
          *n_rows);

  *n_rows = get_n_inserts(n_columns);

  t_sizemat *inserts_delta_2d = malloc(n_columns * *n_rows * sizeof(t_sizemat));

  // only increment a row if condition completed, so no for loop possible for
  // the row.
  ulong row = 0;

  // which number has to move
  for (t_sizemat i = 0; i < n_columns; i++) {
    // to which number i has to move
    for (t_sizemat j = 0; j < n_columns; j++) {
      // avoid duplicates
      if (i != j && i != j + 1) {
        // go through each number of a row
        for (t_sizemat x = 0; x < n_columns; x++) {
          // "move"" number to the left
          if (i < j && x >= i && x < j) {
            inserts_delta_2d[n_columns * row + x] = x + 1;
          } else if (i > j && x > j && x <= i) { // "move" number to the right
            inserts_delta_2d[n_columns * row + x] = x - 1;
          } else if (x == j) { // put i to j
            inserts_delta_2d[n_columns * row + x] = i;
          } else {
            inserts_delta_2d[n_columns * row + x] = x;
          }
        }
#ifndef NDEBUG
        if (n_columns < 100) {
          for (t_sizemat x = 0; x < n_columns; x++) {
            printf("%u ", inserts_delta_2d[n_columns * i + x]);
          }
          printf(" for i=%u, j=%u\n", i, j);
        }
#endif
        row++;
      }
    }
  }

  DPRINTF("for %u collumns; total number of insert: %u\n", n_columns, *n_rows);

  return inserts_delta_2d;
}

t_sizemat *sol_start_random(t_mat_cell *mat, t_sizemat n_columns) {
  DEBUG_PRINT("executing sol_start_random");

  t_sizemat *new_random_vector = generate_random_vector(n_columns);

#ifndef NDEBUG
  DPRINTF("random generated starting solution vector\n")
  for (t_sizemat i = 0; i < n_columns; i++) {
    printf("%u ", new_random_vector[i]);
  }
  printf("\n");
#endif

  return new_random_vector;
}

t_sizemat *sol_start_c_and_w(t_mat_cell *mat, t_sizemat n_columns) {
  DEBUG_PRINT("executing sol_start_c_and_w\n");

  t_mat_cell *sum_row_2d = prefix_sum_per_row_2d(mat, n_columns, n_columns);

#ifndef NDEBUG
  DPRINTF("cost matrix:\n");
  print_array_2d(mat, n_columns, n_columns);
  DPRINTF("Prefix sum of matrix:\n");
  print_array_2d(sum_row_2d, n_columns, n_columns);
#endif

  t_sizemat *new_best_start_1d = generate_inc_vector(n_columns);

  for (t_sizemat i = 0; i < n_columns; i++) {
    t_sizemat best_pos = i;
    t_mat_cell best = 0;
    DPRINTF("sum for i=%u : ", i);
    for (t_sizemat j = i; j < n_columns; j++) {
      t_sizemat sum_row_idx = new_best_start_1d[j];

      // current = mat[i][last] - mat[i][i]
      t_mat_cell last_sum = sum_row_2d[n_columns * sum_row_idx + n_columns - 1];
      t_mat_cell i_sum = sum_row_2d[n_columns * sum_row_idx + i];

      t_mat_cell current = last_sum - i_sum;

      if (current > best) {
        best_pos = j;
        best = current;
      }
    }
    DNPRINTF("\n");
    DPRINTF("best at %u : %u for value %u\n", i, best_pos, best);

    t_sizemat tmp = new_best_start_1d[i];
    new_best_start_1d[i] = new_best_start_1d[best_pos];
    new_best_start_1d[best_pos] = tmp;

    DPRINTF("new_best_start_1d: ");
#ifndef NDEBUG
    print_array_1d(new_best_start_1d, n_columns);
#endif
  }

#ifndef NDEBUG
  DPRINTF("C_and_W solution\n");
  print_array_1d(new_best_start_1d, n_columns);
#endif

  free(sum_row_2d);
  return new_best_start_1d;
}

void neighb_modif(t_sizemat *new_sol_1d, const t_sizemat *sol_1d,
                  const t_sizemat *neighb_delta_1d, t_sizemat n_columns) {
  DPRINTF("executing neighb_modif\n")

  // #pragma omp simd
  for (t_sizemat i = 0; i < n_columns; i++) {
    new_sol_1d[i] = sol_1d[neighb_delta_1d[i]];
  }
}

t_cost pivot_first(const t_sizemat *sol_1d, t_sizemat *new_sol_1d, t_cost cost,
                   struct matrix neighb_deltas, struct matrix cost_matrix) {
  DPRINTF("executing pivot_first\n");

  for (t_sizemat i = 0; i < neighb_deltas.n_rows; i++) {
    t_sizemat *neighb_delta =
        &neighb_deltas.mat_2d[neighb_deltas.n_columns * i];

    neighb_modif(new_sol_1d, sol_1d, neighb_delta, neighb_deltas.n_columns);

    t_cost new_cost =
        computeCost(cost_matrix.mat_2d, new_sol_1d, cost_matrix.n_columns);
    if (new_cost > cost) {
      cost = new_cost;
      break;
    }
  }
  return cost;
}

t_cost pivot_best(const t_sizemat *sol_1d, t_sizemat *new_sol_1d, t_cost cost,
                  struct matrix neighb_deltas, struct matrix cost_matrix) {
  DPRINTF("executing pivot_best\n");

  cost = 0;
  memcpy(new_sol_1d, sol_1d, cost_matrix.n_columns);
  t_sizemat *best_neighb_modif = &neighb_deltas.mat_2d[neighb_deltas.n_columns];

  for (t_sizemat i = 0; i < neighb_deltas.n_rows; i++) {

    t_sizemat *neighb_delta =
        &neighb_deltas.mat_2d[neighb_deltas.n_columns * i];

#ifndef NDEBUG
    DPRINTF("testing neibgh_deltas : ");
    print_array_1d(neighb_delta, neighb_deltas.n_columns);
#endif

    neighb_modif(new_sol_1d, sol_1d, neighb_delta, neighb_deltas.n_columns);

    t_cost new_cost =
        computeCost(cost_matrix.mat_2d, new_sol_1d, cost_matrix.n_columns);

    if (new_cost >= cost) {
      DPRINTF("Found better cost: old cost : %u | new cost : %u\n", *cost,
              new_cost);
      cost = new_cost;
      best_neighb_modif = neighb_delta;
    }
  }

  neighb_modif(new_sol_1d, sol_1d, best_neighb_modif, neighb_deltas.n_columns);
  return cost;
}

void lop(t_mat_cell *cost_mat_2d, t_sizemat cost_mat_dim,
         t_fptr_sol_start fptr_sol_start, t_fptr_pivot_rule fptr_pivot_rule,
         t_fptr_neighborhood fptr_neighborhood) {
  DPRINTF("executing lop\n");

  struct matrix cost_mat;
  cost_mat.mat_2d = cost_mat_2d;
  cost_mat.n_columns = cost_mat_dim;
  cost_mat.n_rows = cost_mat_dim;

  // initial solution
  t_sizemat *sol_1d = fptr_sol_start(cost_mat_2d, cost_mat_dim);

  // all possible modif to apply to a vector to neighborhood
  struct matrix neigh_deltas;
  neigh_deltas.n_columns = cost_mat_dim;
  neigh_deltas.mat_2d =
      fptr_neighborhood(&neigh_deltas.n_rows, neigh_deltas.n_columns);

  t_cost cost = computeCost(cost_mat_2d, sol_1d, cost_mat_dim);
  t_cost new_cost = cost;
  t_sizemat *new_sol_1d = malloc(cost_mat_dim * sizeof(t_sizemat));

  while (cost < new_cost ||
         (cost == new_cost && !array_equal(new_sol_1d, sol_1d, cost_mat_dim))) {

#ifndef NDEBUG
    DPRINTF("lop while : cost=%u and new_cost=%u\n", cost, new_cost);

    DPRINTF("best sol: ");
    print_array_1d(new_sol_1d, cost_mat_dim);
#endif

    // TODO: cost and new_cost can have the same value. alows it but verify that
    // there is no comming back.
    cost = new_cost;
    memcpy(sol_1d, new_sol_1d, cost_mat_dim * sizeof(t_sizemat));

    new_cost =
        fptr_pivot_rule(sol_1d, new_sol_1d, cost, neigh_deltas, cost_mat);
    DPRINTF("lop while after pivot: cost=%u and new_cost=%u\n", cost, new_cost);
  }
  free(new_sol_1d);
  free(neigh_deltas.mat_2d);
}
