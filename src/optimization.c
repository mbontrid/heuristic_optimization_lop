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

t_cost computeCost(const t_mat_cell *const cost_mat_2d, const size_t *const lo,
                   size_t size) {
  // #ifndef NDEBUG
  //   DPRINTF("executing computeCost with vector: \n");
  //   print_array_1d(lo, size);
  // #endif

  t_cost sum = 0;
  /* Diagonal value are not considered */
  for (size_t h = 0; h < size; h++)
    for (size_t k = h + 1; k < size; k++) {
      assert(MAX_COST - cost_mat_2d[size * lo[h] + lo[k]] >= sum);
      sum += cost_mat_2d[size * lo[h] + lo[k]];
    }
  return (sum);
}

long int get_cost_diff_with_shuffle(const t_mat_cell *const cost_mat_2d,
                                    const size_t *const sol,
                                    const size_t *const neighb_delta,
                                    const size_t size) {
  DPRINTF("executing... \n");

  long int cost_diff = 0;

  for (size_t moved_idx = 0; moved_idx < size; moved_idx++) {
    const size_t moved_pos = neighb_delta[moved_idx];
    if (moved_pos == moved_idx) {
      continue;
    }

    const size_t moved_sol = sol[moved_idx];
    const size_t moved_row = size * moved_sol;

    /* Pairs (moved_idx, right_idx) with moved_idx < right_idx */
    for (size_t right_idx = moved_idx + 1; right_idx < size; right_idx++) {
      if (moved_pos <= neighb_delta[right_idx]) {
        continue;
      }
      const size_t right_sol = sol[right_idx];
      const size_t right_row_offset = (size_t)size * right_sol;
      cost_diff += (long int)cost_mat_2d[right_row_offset + moved_sol] -
                   (long int)cost_mat_2d[moved_row + right_sol];
    }

    /*
     * Pairs (left_idx, moved_idx) with left_idx < moved_idx.
     */
    for (size_t left_idx = 0; left_idx < moved_idx; left_idx++) {
      if (neighb_delta[left_idx] != left_idx || left_idx <= moved_pos) {
        continue;
      }
      const size_t left_value = sol[left_idx];
      const size_t left_row_offset = (size_t)size * left_value;
      cost_diff += (long int)cost_mat_2d[moved_row + left_value] -
                   (long int)cost_mat_2d[left_row_offset + moved_sol];
    }
  }

// this is a test of the good result runing only when NDEBUG is defined. It is
// slowing considerably the execution. as multiple ComputeCost are made.
#ifndef NDEBUG
  size_t *new_sol_ndebug = malloc(size * sizeof(size_t));
  assert(new_sol_ndebug);
  array_apply_shuffle(new_sol_ndebug, neighb_delta, sol, size);
  t_cost cost_assert = computeCost(cost_mat_2d, sol, size);
  t_cost new_cost_assert = computeCost(cost_mat_2d, new_sol_ndebug, size);
  assert(cost_assert + cost_diff == new_cost_assert);
  free(new_sol_ndebug);
  DPRINTF("cost_diff computed : %ld\n", cost_diff);
#endif

  return cost_diff;
}

t_cost_delta cost_swap_delta(const t_mat_cell *const cost_mat_2d,
                             const size_t *const sol_1d, const size_t size,
                             const size_t i, const size_t j) {

  assert(cost_mat_2d);
  assert(sol_1d);
  assert(i < size);
  assert(j < size);
  assert(get_max_array(sol_1d, size) < size);

  if (size < 2 || i == j) {
    return 0;
  }

  size_t left = min(i, j);
  size_t right = max(i, j);

  const size_t left_sol = sol_1d[left];
  const size_t right_sol = sol_1d[right];
  const size_t left_row_offset = size * left_sol;
  const size_t right_row_offset = size * right_sol;

  long int delta = (long int)cost_mat_2d[right_row_offset + left_sol] -
                   (long int)cost_mat_2d[left_row_offset + right_sol];

  for (size_t k = left + 1; k < right; k++) {
    const size_t middle_value = sol_1d[k];
    const size_t middle_row_offset = size * middle_value;
    delta += (long int)cost_mat_2d[right_row_offset + middle_value] -
             (long int)cost_mat_2d[left_row_offset + middle_value] +
             (long int)cost_mat_2d[middle_row_offset + left_sol] -
             (long int)cost_mat_2d[middle_row_offset + right_sol];
  }

  // #ifndef NDEBUG
  //   size_t *new_sol_assert = malloc(size * sizeof(size_t));
  //   assert(new_sol_assert);
  //   memcpy(new_sol_assert, sol_1d, size * sizeof(size_t));
  //   swap(new_sol_assert, left, right);
  //   t_cost sol_cost = computeCost(cost_mat_2d, sol_1d, size);
  //   t_cost new_sol_assert_cost = computeCost(cost_mat_2d, new_sol_assert,
  //   size);
  //   assert((long int)sol_cost + delta == (long int)new_sol_assert_cost);
  //   free(new_sol_assert);
  // #endif

  return delta;
}

t_cost_delta rand_swap(const t_cost *const cost_mat, size_t *const array,
                       const size_t size, const float rate) {
  assert(rate <= 1 || rate >= 0);
  assert(array);

  t_cost_delta delta = 0;
  const size_t n_mutate = (size_t)(rate * size);
  for (size_t i = 0; i < n_mutate; i++) {
    size_t j = randInt(0, size - 1);
    size_t k = randInt(0, size - 1);
    delta += cost_swap_delta(cost_mat, array, size, j, k);
    swap(array, j, k);
  }
  return delta;
}

t_mat_cell *prefix_sum_per_row_2d(t_mat_cell *mat, size_t n_rows,
                                  size_t n_columns) {

  t_mat_cell *sum_row_2d = malloc(n_columns * n_columns * sizeof(t_mat_cell));
  assert(sum_row_2d);
  for (size_t i = 0; i < n_rows; i++) {
    sum_row_2d[n_columns * i] = mat[n_columns * i];
    for (size_t j = 1; j < n_columns; j++) {
      assert(MAX_COST_CELL -
             sum_row_2d[n_columns * i + j - 1 >= mat[n_columns * i + j]]);
      sum_row_2d[n_columns * i + j] =
          sum_row_2d[n_columns * i + j - 1] + mat[n_columns * i + j];
    }
  }

  return sum_row_2d;
}

size_t get_n_transpose(size_t size) {

  if (size < 2) {
    return 0;
  } else if (size == 2) {
    return 1;
  }
  DPRINTF("get_n_transpose: for a %zu array, there is %zu transpose\n", size,
          (size));
  return size;
}

size_t get_n_inserts(size_t size) {
  assert(MAX_SIZEMAT - (size - 1) * (size - 1) >= 0);
  size_t n_inserts = (size - 1) * (size - 1);
  if (size < 2) {
    n_inserts = 0;
  } else if (size == 2) {
    n_inserts = 3;
  }

  DPRINTF("for a %zu array, there is %zu insert\n", size, n_inserts);

  return n_inserts;
}

size_t get_n_exchange(size_t size) {
  if (size < 2) {
    return 0;
  }
  return size * (size - 1) / 2;
}

t_cost_delta cost_delta_exchange(const t_mat_cell *const cost_mat_2d,
                                 size_t *const sol_1d, size_t size,
                                 bool is_first) {

  t_cost best_delta = 0;
  size_t best_i = 0;
  size_t best_j = 0;

  for (size_t i = 0; i < size - 1; i++) {
    for (size_t j = i + 1; j < size; j++) {
      long int cost_delta = cost_swap_delta(cost_mat_2d, sol_1d, size, i, j);

      if (cost_delta > best_delta) {
        best_delta = cost_delta;
        best_i = i;
        best_j = j;
        if (is_first) {
          swap(sol_1d, best_i, best_j);
          return best_delta;
        }
      }
    }
  }

  assert(best_delta ==
         cost_swap_delta(cost_mat_2d, sol_1d, size, best_i, best_j));
  assert(get_max_array(sol_1d, size) < size);
  swap(sol_1d, best_i, best_j);
  return best_delta;
}

t_cost_delta cost_delta_transpose(const t_mat_cell *const cost_mat_2d,
                                  size_t *const sol_1d, size_t size,
                                  bool is_first) {
  assert(get_max_array(sol_1d, size) < size);
  t_cost_delta best_delta = 0;
  size_t best_i = 0;
  // best_j necessary as to prevent to a bug with swap if there is no better sol
  // found before the end of the for.
  size_t best_j = 0;

  for (size_t i = 0; i < size; i++) {
    size_t j = (i + 1) % size;
    t_cost_delta cost_delta = cost_swap_delta(cost_mat_2d, sol_1d, size, i, j);

    if (cost_delta > best_delta) {
      best_delta = cost_delta;
      best_i = i;
      best_j = j;
      if (is_first) {
        break;
      }
    }
  }
  assert(best_delta ==
         cost_swap_delta(cost_mat_2d, sol_1d, size, best_i, best_j));
  assert(get_max_array(sol_1d, size) < size);

  swap(sol_1d, best_i, best_j);
  return best_delta;
}

t_cost_delta cost_delta_insert(const t_mat_cell *const cost_mat_2d,
                               size_t *const sol_1d, size_t size,
                               bool is_first) {
  t_cost_delta best_delta = 0;
  size_t *best_sol = malloc(size * sizeof(size_t));
  memcpy(best_sol, sol_1d, size * sizeof(size_t));

  t_cost_delta construction_cost_delta = 0;
  size_t *constructive_sol = malloc(size * sizeof(size_t));
  memcpy(constructive_sol, sol_1d, size * sizeof(size_t));

  /*
   * As insertions from i to j includes all insertions from i to j-n (n<i) and
   * ar simple swap until j, the number of insertions can be greatly reduced and
   * calculated by cost difference.
   */
  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      if (i == j) {
        // reset the constructive solution and the cost delta for the next i.
        memcpy(constructive_sol, sol_1d, size * sizeof(size_t));

        construction_cost_delta = 0;
      } else if (i != j + 1) {
        // i != j + 1 avoid to redundant swap of adjacent element, which is a
        // transpose and already tested in cost_delta_transpose.

        // calculating the cost difference of the swap.
        construction_cost_delta +=
            cost_swap_delta(cost_mat_2d, constructive_sol, size, i, j);
        swap(constructive_sol, i, j);

        assert(get_max_array(best_sol, size) < size);
        assert(get_max_array(sol_1d, size) < size);

        if (construction_cost_delta > best_delta) {
          memcpy(best_sol, constructive_sol, size * sizeof(size_t));
          best_delta = construction_cost_delta;
          if (is_first) {
            i = size;
            break;
          }
        }
      }
    }
    memcpy(constructive_sol, sol_1d, size * sizeof(size_t));
    construction_cost_delta = 0;
  }

  assert(best_delta == computeCost(cost_mat_2d, best_sol, size) -
                           computeCost(cost_mat_2d, sol_1d, size));
  assert(get_max_array(best_sol, size) < size);
  assert(get_max_array(sol_1d, size) < size);
  memcpy(sol_1d, best_sol, size * sizeof(size_t));
  free(best_sol);
  free(constructive_sol);

  return best_delta;
}

size_t *sol_start_random(t_mat_cell *mat, size_t n_columns) {
  DPRINTF("executing sol_start_random\n");

  size_t *new_random_vector = generate_random_vector(n_columns);

  return new_random_vector;
}

size_t *sol_start_cw(t_mat_cell *cost_mat_2d, size_t size) {
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

  size_t *new_best_start_1d = generate_random_vector(size);

  for (size_t i = 0; i < size; i++) {
    size_t best_pos = i;
    t_mat_cell best = 0;
    for (size_t j = i; j < size; j++) {
      size_t sum_row_idx = new_best_start_1d[j];

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

    swap(new_best_start_1d, i, best_pos);
  }

  DPRINTF("C_and_W solution\n");
  PARRAY((long int *)new_best_start_1d, size);

  free(sum_row_2d);
  return new_best_start_1d;
}

t_cost_delta
lop_iter_impr(const t_mat_cell *const cost_mat_2d, size_t mat_cost_dim,
              size_t *const sol_1d, enum pivot_enum pivot_rule,
              t_fptr_delta_neigh_exploration fptr_cost_delta_neig_exploration) {

  // new solution after each pivot.
  size_t *new_sol_1d = malloc(mat_cost_dim * sizeof(size_t));
  memcpy(new_sol_1d, sol_1d, mat_cost_dim * sizeof(size_t));

  t_cost_delta neighb_delta = 0;
  t_cost_delta delta_total = 0;

  do {

    delta_total += neighb_delta;
    memcpy(sol_1d, new_sol_1d, mat_cost_dim * sizeof(size_t));

    neighb_delta = fptr_cost_delta_neig_exploration(cost_mat_2d, new_sol_1d,
                                                    mat_cost_dim, pivot_rule);

    // DPRINTF("delta total=%ld | neighb delta=%ld\n", delta_total,
    // neighb_delta);
    assert(neighb_delta == computeCost(cost_mat_2d, new_sol_1d, mat_cost_dim) -
                               computeCost(cost_mat_2d, sol_1d, mat_cost_dim));
    assert(get_max_array(sol_1d, mat_cost_dim) < mat_cost_dim);
  } while (neighb_delta);

  free(new_sol_1d);
  return delta_total;
}

t_cost_delta
vnd_lop(const t_mat_cell *const cost_mat_2d, size_t mat_cost_dim,
        size_t *const sol_1d, enum pivot_enum pivot_rule,
        t_fptr_delta_neigh_exploration *fptr_delta_neigh_exploration,
        const ushort n_neighb_vn) {

  t_cost_delta cost_delta = 0;
  ushort k_neighb = 0;
  // try all neighborhood methods in order and start again if there is
  // improvement.
  while (k_neighb < n_neighb_vn) {

    t_cost_delta new_delta =
        lop_iter_impr(cost_mat_2d, mat_cost_dim, sol_1d, pivot_rule,
                      fptr_delta_neigh_exploration[k_neighb]);
    cost_delta += new_delta;

    if (new_delta && k_neighb > 0) {
      k_neighb = 0;
      PVERB("Found a new optimization of %d with neighborhood method %u\n",
            new_delta, k_neighb);
    } else {
      PVERB("No improvement with neighborhood method %u\n", k_neighb);
      k_neighb++;
    }
  }
  return cost_delta;
}
