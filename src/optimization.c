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

#include "arg_parser.h"
#include "optimization.h"
#include "utilities.h"

#ifdef __MINGW32__
#include <float.h>
#define MAX_FLOAT FLT_MAX
#else
#define MAX_FLOAT MAXFLOAT
#endif

extern struct arguments arguments;

t_cost computeCost(const t_mat_cell *const cost_mat_2d,
                   const t_sizemat *const lo, t_sizemat size) {
  // #ifndef NDEBUG
  //   DPRINTF("executing computeCost with vector: \n");
  //   print_array_1d(lo, size);
  // #endif

  t_cost sum = 0;
  /* Diagonal value are not considered */
  for (t_sizemat h = 0; h < size; h++)
    for (t_sizemat k = h + 1; k < size; k++) {
      assert(MAX_COST - cost_mat_2d[size * lo[h] + lo[k]] >= sum);
      sum += cost_mat_2d[size * lo[h] + lo[k]];
    }
  return (sum);
}

long int get_cost_diff_with_delta(const t_mat_cell *const cost_mat_2d,
                                  const t_sizemat *const sol,
                                  const t_sizemat *const neighb_delta,
                                  const t_sizemat size) {
  long int cost_diff = 0;

  for (t_sizemat moved_idx = 0; moved_idx < size; moved_idx++) {
    const t_sizemat moved_pos = neighb_delta[moved_idx];
    if (moved_pos == moved_idx) {
      continue;
    }

    const t_sizemat moved_sol = sol[moved_idx];
    const t_sizemat moved_row = size * moved_sol;

    /* Pairs (moved_idx, right_idx) with moved_idx < right_idx */
    for (t_sizemat right_idx = moved_idx + 1; right_idx < size; right_idx++) {
      if (moved_pos <= neighb_delta[right_idx]) {
        continue;
      }
      const t_sizemat right_value = sol[right_idx];
      const size_t right_row_offset = (size_t)size * right_value;
      cost_diff += (long int)cost_mat_2d[right_row_offset + moved_sol] -
                   (long int)cost_mat_2d[moved_row + right_value];
    }

    /*
     * Pairs (left_idx, moved_idx) with left_idx < moved_idx.
     */
    for (t_sizemat left_idx = 0; left_idx < moved_idx; left_idx++) {
      if (neighb_delta[left_idx] != left_idx || left_idx <= moved_pos) {
        continue;
      }
      const t_sizemat left_value = sol[left_idx];
      const size_t left_row_offset = (size_t)size * left_value;
      cost_diff += (long int)cost_mat_2d[moved_row + left_value] -
                   (long int)cost_mat_2d[left_row_offset + moved_sol];
    }
  }

// this is a test of the good result runing only when NDEBUG is defined. It is
// slowing considerably the execution. as multiple ComputeCost are made.
#ifndef NDEBUG
  t_sizemat *new_sol_ndebug = malloc(size * sizeof(t_sizemat));
  assert(new_sol_ndebug);
  array_apply_shuffle(new_sol_ndebug, neighb_delta, sol, size);
  t_cost cost_assert = computeCost(cost_mat_2d, sol, size);
  t_cost new_cost_assert = computeCost(cost_mat_2d, new_sol_ndebug, size);
  assert(cost_assert + cost_diff == new_cost_assert);
  free(new_sol_ndebug);
#endif

  return cost_diff;
}

t_cost get_cost(const t_mat_cell *const cost_mat_2d, const t_sizemat *const sol,
                const t_sizemat *const neighb_delta, const t_sizemat size,
                t_cost old_cost) {

  assert(old_cost == computeCost(cost_mat_2d, sol, size));

  long int cost_diff =
      get_cost_diff_with_delta(cost_mat_2d, sol, neighb_delta, size);
  t_cost new_cost = old_cost + cost_diff;

#ifndef NDEBUG
  // DPRINTF("old_cost=%u | cost_diff=%ld | new_cost=%u\n", old_cost, cost_diff,
  //         new_cost);
  t_sizemat *assert_new_sol_1d = malloc(size * sizeof(t_sizemat));
  assert(assert_new_sol_1d);
  array_apply_shuffle(assert_new_sol_1d, neighb_delta, sol, size);
  t_cost assert_new_cost = computeCost(cost_mat_2d, assert_new_sol_1d, size);
  assert(assert_new_cost == new_cost);
  free(assert_new_sol_1d);
#endif

  return new_cost;
}

t_mat_cell *prefix_sum_per_row_2d(t_mat_cell *mat, t_sizemat n_rows,
                                  t_sizemat n_columns) {

  t_mat_cell *sum_row_2d = malloc(n_columns * n_columns * sizeof(t_mat_cell));
  assert(sum_row_2d);
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

  *n_rows = get_n_transpose(n_columns);
  t_sizemat *transposes_deltas_2d =
      malloc(*n_rows * n_columns * sizeof(t_sizemat));
  assert(transposes_deltas_2d);

  DPRINTF("%u transoposes possibilities allocated\n", *n_rows);

  for (ulong i = 0; i < *n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      transposes_deltas_2d[n_columns * i + j] = j;
    }

    t_sizemat temp = transposes_deltas_2d[n_columns * i + i];
    transposes_deltas_2d[n_columns * i + i] =
        transposes_deltas_2d[n_columns * i + i + 1];
    transposes_deltas_2d[n_columns * i + (i + 1) % n_columns] =
        temp; // the % alows the transpose to go around the array.
  }

  //   DPRINTF("all transposes : \n");
  // #ifndef NDEBUG
  //   for (t_sizemat i = 0; i < *n_rows; i++) {
  //     DPRINTF("transpose %u : ", i)
  //     PARRAY(&transposes_deltas_2d[n_columns * i], n_columns);
  //   }
  // #endif
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
  t_sizemat *restrict exchanges_deltas_2d =
      malloc(*n_rows * n_columns * sizeof(t_sizemat));
  assert(exchanges_deltas_2d);

  DPRINTF("%d exchanges possibilities allocated\n", *n_rows);

#pragma omp simd
  for (ulong i = 0; i < *n_rows * n_columns; i++) {
    exchanges_deltas_2d[i] = i % n_columns;
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

  DPRINTF("executing for %d collumns and %d rows\n", n_columns, *n_rows);
#ifndef NDEBUG
  if (*n_rows < 100) {
    for (ulong i = 0; i < *n_rows; i++) {
      for (t_sizemat j = 0; j < n_columns; j++) {
        printf("%u ", exchanges_deltas_2d[n_columns * i + j]);
      }
      printf("\n");
    }
  }
#endif

  return exchanges_deltas_2d;
}

t_sizemat get_n_inserts(t_sizemat size) {
  assert(MAX_SIZEMAT - (size - 1) * (size - 1) >= 0);
  t_sizemat n_inserts = (size - 1) * (size - 1);
  if (size < 2) {
    n_inserts = 0;
  } else if (size == 2) {
    n_inserts = 3;
  }

  DPRINTF("for a %u array, there is %u insert\n", size, n_inserts);

  return n_inserts;
}

t_sizemat *neighb_insert_deltas(t_sizemat *const n_rows,
                                const t_sizemat n_columns) {

  *n_rows = get_n_inserts(n_columns);
  DPRINTF("executing for %u collumns and %d rows\n", n_columns, *n_rows);

  t_sizemat *inserts_delta_2d = malloc(n_columns * *n_rows * sizeof(t_sizemat));
  assert(inserts_delta_2d);
  DPRINTF("%u inserts possibilities allocated\n", *n_rows);

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
  DPRINTF("executing sol_start_random\n");

  t_sizemat *new_random_vector = generate_random_vector(n_columns);

#ifndef NDEBUG
  DPRINTF("random generated starting solution vector\n")
  for (t_sizemat i = 0; i < n_columns; i++) {
    printf("%d ", new_random_vector[i]);
  }
  printf("\n");
#endif

  return new_random_vector;
}

t_sizemat *sol_start_cw(t_mat_cell *cost_mat_2d, t_sizemat size) {
  DPRINTF("executing sol_start_c_and_w\n");

  t_mat_cell *sum_row_2d = prefix_sum_per_row_2d(cost_mat_2d, size, size);

#ifndef NDEBUG
  if (size <= 50) {
    DPRINTF("cost matrix:\n");
    print_array_2d(cost_mat_2d, size, size);
    DPRINTF("Prefix sum of matrix:\n");
    print_array_2d(sum_row_2d, size, size);
  }
#endif

  t_sizemat *new_best_start_1d = generate_inc_vector(size);

  for (t_sizemat i = 0; i < size; i++) {
    t_sizemat best_pos = i;
    t_mat_cell best = 0;
    for (t_sizemat j = i; j < size; j++) {
      t_sizemat sum_row_idx = new_best_start_1d[j];

      // current = cost_mat_2d[i][last] - cost_mat_2d[i][i]
      t_mat_cell last_sum = sum_row_2d[size * sum_row_idx + size - 1];
      t_mat_cell i_sum = sum_row_2d[size * sum_row_idx + i];

      t_mat_cell current = last_sum - i_sum;

      if (current > best) {
        best_pos = j;
        best = current;
      }
    }
    // DPRINTF("best at %u : %u for value %u\n", i, best_pos, best);

    t_sizemat tmp = new_best_start_1d[i];
    new_best_start_1d[i] = new_best_start_1d[best_pos];
    new_best_start_1d[best_pos] = tmp;

    // DPRINTF("new_best_start_1d: ");
#ifndef NDEBUG
    // print_array_1d(new_best_start_1d, size);
#endif
  }

#ifndef NDEBUG
  DPRINTF("C_and_W solution\n");
  print_array_1d(new_best_start_1d, size);
#endif

  free(sum_row_2d);
  return new_best_start_1d;
}

t_sizemat *sol_start_cw_tentative(const t_mat_cell *const restrict cost_mat_2d,
                                  t_sizemat size) {
  DPRINTF("executting cw\n");

  t_cost *const restrict r_1d = calloc(size, sizeof(t_cost));
  t_cost *const restrict c_1d = calloc(size, sizeof(t_cost));
  Item *const restrict s_1d = malloc(size * sizeof(Item));
  assert(s_1d);

  t_sizemat *restrict sol_1d = malloc(size * sizeof(t_sizemat));
  assert(sol_1d);

/*
 * r[i] = sum j w_ij (i=/=j)
 * c[i] = sum j w_ji (i=/=j)
 * s[i] = r[i] - c[i]
 */
#pragma omp simd
  for (t_sizemat x = 0; x < size * size; x++) {
    t_sizemat i = x / size;
    t_sizemat j = x % size;
    assert(MAX_COST_CELL - cost_mat_2d[size * i + j] >= r_1d[i]);
    r_1d[i] += cost_mat_2d[size * i + j];
    assert(MAX_COST_CELL - cost_mat_2d[size * j + i] >= c_1d[i]);
    c_1d[i] += cost_mat_2d[size * j + i];
  }

#pragma omp simd
  for (t_sizemat i = 0; i < size; i++) {
    s_1d[i].value = r_1d[i] - c_1d[i] - 2 * cost_mat_2d[size * i + i];
    assert(s_1d[i].index <= size);
    s_1d[i].index = i;
  }

  qsort(s_1d, size, sizeof(Item), cmp_desc);

#pragma omp simd
  for (t_sizemat i = 0; i < size; i++) {
    sol_1d[i] = s_1d[i].index;
  }

  DPRINTF("cw done, sol= ");
#ifndef NDEBUG
  print_array_1d(sol_1d, size);
#endif

  free(r_1d);
  free(c_1d);
  free(s_1d);
  return sol_1d;
}

t_sizemat *sol_start_c_and_w(t_mat_cell *cost_mat_2d, t_sizemat size) {
  DPRINTF("executing sol_start_c_and_w\n");
  t_sizemat *sol_1d = sol_start_cw(cost_mat_2d, size);
  // t_sizemat *sol_1d = sol_start_cw_tentative(cost_mat_2d, size); \\don't
  // work.

  return sol_1d;
}

t_cost pivot_first(const t_sizemat *sol_1d, t_sizemat *new_sol_1d,
                   const t_cost cost, struct matrix neighb_deltas,
                   struct matrix cost_matrix) {

  DPRINTF("executing...");

  assert(array_equal(new_sol_1d, sol_1d, neighb_deltas.n_columns));

  t_cost best_cost = cost;

  assert(cost ==
         computeCost(cost_matrix.mat_2d, sol_1d, neighb_deltas.n_columns));

  for (t_sizemat i = 0; i < neighb_deltas.n_rows; i++) {

    const t_sizemat *neighb_delta =
        &neighb_deltas.mat_2d[neighb_deltas.n_columns * i];

    assert(best_cost == computeCost(cost_matrix.mat_2d, new_sol_1d,
                                    neighb_deltas.n_columns));

    t_cost new_cost = get_cost(cost_matrix.mat_2d, sol_1d, neighb_delta,
                               neighb_deltas.n_columns, cost);

#ifndef NDEBUG
    t_sizemat *assert_new_sol =
        malloc(neighb_deltas.n_columns * sizeof(t_sizemat));
    assert(assert_new_sol);
    array_apply_shuffle(assert_new_sol, neighb_delta, sol_1d,
                        neighb_deltas.n_columns);
    t_cost assert_cost = computeCost(cost_matrix.mat_2d, assert_new_sol,
                                     neighb_deltas.n_columns);
    assert(new_cost == assert_cost);
    free(assert_new_sol);
#endif

    if (new_cost > cost) {
      DPRINTF("Found better cost: old cost : %u | new cost : %u\n", cost,
              new_cost);
      best_cost = new_cost;
      array_apply_shuffle(new_sol_1d, neighb_delta, sol_1d,
                          neighb_deltas.n_columns);
      break;
    }
  }
  return best_cost;
}

t_cost pivot_best(const t_sizemat *const sol_1d, t_sizemat *new_sol_1d,
                  const t_cost cost, struct matrix neighb_deltas,
                  struct matrix cost_matrix) {
  DPRINTF("executing...\n");

  assert(array_equal(new_sol_1d, sol_1d, neighb_deltas.n_columns));

  t_cost best_cost = cost;

  assert(cost ==
         computeCost(cost_matrix.mat_2d, sol_1d, neighb_deltas.n_columns));

  for (t_sizemat i = 0; i < neighb_deltas.n_rows; i++) {

    const t_sizemat *neighb_delta =
        &neighb_deltas.mat_2d[neighb_deltas.n_columns * i];

    assert(best_cost == computeCost(cost_matrix.mat_2d, new_sol_1d,
                                    neighb_deltas.n_columns));

    t_cost new_cost = get_cost(cost_matrix.mat_2d, sol_1d, neighb_delta,
                               neighb_deltas.n_columns, cost);

#ifndef NDEBUG
    t_sizemat *assert_new_sol =
        malloc(neighb_deltas.n_columns * sizeof(t_sizemat));
    assert(assert_new_sol);
    array_apply_shuffle(assert_new_sol, neighb_delta, sol_1d,
                        neighb_deltas.n_columns);
    t_cost assert_cost = computeCost(cost_matrix.mat_2d, assert_new_sol,
                                     neighb_deltas.n_columns);
    assert(new_cost == assert_cost);
    free(assert_new_sol);
#endif

    if (new_cost > best_cost) {
      DPRINTF("Found better cost: old cost : %u | new cost : %u\n", cost,
              new_cost);
      best_cost = new_cost;
      array_apply_shuffle(new_sol_1d, neighb_delta, sol_1d,
                          neighb_deltas.n_columns);
    }
  }

  return best_cost;
}

t_cost it_imp_lop(t_mat_cell *cost_mat_2d, t_sizemat mat_cost_dim,
                  t_sizemat *const sol_1d, t_fptr_pivot_rule fptr_pivot_rule,
                  t_fptr_neighborhood fptr_neighborhood) {
  DPRINTF("executing lop\n");

  // put matrixes in a struct
  struct matrix cost_mat;
  cost_mat.mat_2d = cost_mat_2d;
  cost_mat.n_columns = mat_cost_dim;
  cost_mat.n_rows = mat_cost_dim;

  // all possible modif to apply to a vector to neighborhood
  struct matrix neigh_deltas;
  neigh_deltas.n_columns = mat_cost_dim;
  neigh_deltas.mat_2d =
      fptr_neighborhood(&neigh_deltas.n_rows, neigh_deltas.n_columns);

  // new solution after each pivot.
  t_sizemat *new_sol_1d = malloc(cost_mat.n_columns * sizeof(t_sizemat));
  assert(new_sol_1d);
  memcpy(new_sol_1d, sol_1d, mat_cost_dim * sizeof(t_sizemat));

  t_cost cost = computeCost(cost_mat_2d, sol_1d, mat_cost_dim);
  t_cost new_cost = cost;
  DPRINTF("starting cost : %u cost\n", cost);

  bool is_improve = true;

  while (is_improve) {

    cost = new_cost;
    memcpy(sol_1d, new_sol_1d, mat_cost_dim * sizeof(t_sizemat));

    assert(array_equal(new_sol_1d, sol_1d, mat_cost_dim));
    assert(cost == new_cost);
    assert(cost_mat_2d[mat_cost_dim * 0 + 0] >= 0);

    new_cost =
        fptr_pivot_rule(sol_1d, new_sol_1d, cost, neigh_deltas, cost_mat);

    assert(new_cost == computeCost(cost_mat_2d, new_sol_1d, mat_cost_dim));

    // is_improve = cost < new_cost;
    is_improve =
        cost < new_cost ||
        (cost == new_cost && !array_equal(new_sol_1d, sol_1d, mat_cost_dim));
    DPRINTF("cost=%d | new_cost=%d\n", cost, new_cost);
  }
  DPRINTF("lop best cost found: %u\n", cost);
  free(new_sol_1d);
  free((t_mat_cell *const)neigh_deltas.mat_2d);
  return cost;
}

t_cost vnd_lop(t_mat_cell *cost_mat_2d, t_sizemat mat_cost_dim,
               t_sizemat *const sol_1d, t_fptr_pivot_rule fptr_pivot_rule,
               t_fptr_neighborhood *fptr_neighborhood,
               const ushort n_neighb_vn) {

  t_cost cost = 0;
  ushort k_neighb = 0;
  while (k_neighb < n_neighb_vn) {

    DPRINTF("vnd %d of %d\n", k_neighb, n_neighb_vn);

    t_cost new_cost = it_imp_lop(cost_mat_2d, mat_cost_dim, sol_1d,
                                 fptr_pivot_rule, fptr_neighborhood[k_neighb]);

    if (new_cost > cost) {
      k_neighb = 0;
      cost = new_cost;
      DPRINTF("vnd_lop found better cost: old cost : %u | new cost : %u\n",
              cost, new_cost);
    } else {
      DPRINTF("vnd_lop no improvement with neighborhood %u\n", k_neighb);
      k_neighb++;
    }
  }
  return cost;
}
