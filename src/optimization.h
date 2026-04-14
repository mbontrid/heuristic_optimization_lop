/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
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
#ifndef _LO_H_
#define _LO_H_

#include "utilities.h"
#include <sys/types.h>

struct matrix {
  const size_t *mat_2d;
  size_t n_rows;
  size_t n_columns;
};

typedef size_t *(*t_fptr_neighborhood)(size_t *const n_rows,
                                          const size_t n_columns);
typedef t_cost (*t_fptr_pivot_rule)(const size_t *sol_1d,
                                    size_t *new_sol_1d, t_cost cost,
                                    struct matrix nighb_deltas,
                                    struct matrix cost_matrix);
typedef size_t *(*t_fptr_sol_start)(t_mat_cell *CostMat, size_t size);

extern t_mat_cell **cost_mat_2d;

t_cost computeCost(const t_mat_cell *const cost_mat, const size_t *const lo,
                   size_t size);

/**
 * @brief get the cost difference if a neithberhood modification is applied to a
 * solution.
 *
 * @param cost_mat_2d 2d matrix of costs.
 * @param sol source solution (neighb_delta is not yet applied on it).
 * @param neighb_delta modification on sol.
 * @param size size of neighb_delta and sol.
 * @return the diference of the cost of sol and cost of sol with neighb_delta
 * applied.
 */
long int get_cost_diff_with_shuffle(const t_mat_cell *const cost_mat_2d,
                                    const size_t *const sol,
                                    const size_t *const neighb_delta,
                                    const size_t size);

/**
 * @brief Compute efficiently the cost difference if a swap of two elemen of a
 * solution of a lop is apllied.
 *
 * @param cost_mat_2d Cost matrix 2d  of size [size, size] flattened to 1d.
 * @param sol_1d 1d array of size [size] on which the swap is applied.
 * @param size size of {cost_mat_2d} and {sol_1d}.
 * @param i index of the first element to swap in sol_1d.
 * @param j index of the second element to swap in sol_1d.
 * @return delta cost of the swap, i.e. cost of sol_1d with i and j swapped -
 * cost of sol_1d.
 */
long int cost_swap_delta(const t_mat_cell *const cost_mat_2d,
                         const size_t *const sol_1d, const size_t size,
                         const size_t i, const size_t j);

/**
 * @brief Compute the prefix sum of each row of a matrix independently.
 * 0 1 2 3 4 5 6 7 8 9 -> 0 1 3 6 10 17 25 34
 *
 * @param mat Matrix from which to compute the prefix sum on each row.
 * @param n_rows dimension of mat
 * @param n_columns dimension of mat
 * @return A new 2d matrix pointer.
 */
t_mat_cell *prefix_sum_per_row_2d(t_mat_cell *mat, size_t n_rows,
                                  size_t n_columns);

size_t get_n_transpose(size_t size);
size_t get_n_inserts(size_t size);
size_t get_n_exchange(size_t size);

/**
 * @brief get_all_possible transposes in a vector of size n_columns
 *
 * @param transposes 2d Vector in which all possible transposes will be filled
 * at each row.
 * @param n_columns size of vector to transpose.
 * @param n_rows number of possible transposes.
 */
size_t *neighb_transpose_deltas(size_t *const n_rows,
                                   const size_t n_columns);
size_t *neighb_exchange_deltas(size_t *const n_rows,
                                  const size_t n_columns);
size_t *neighb_insert_deltas(size_t *const n_rows,
                                const size_t n_columns);

size_t *sol_start_random(t_mat_cell *mat, size_t n_columns);

/**
 * @brief Chenery and Watanabe (CW) greedy heuristic.
 *
 * @param mat Cost matrix.
 * @param n_columns Dimension of the square matrix
 * @return A array of the CW ordering.
 */
size_t *sol_start_c_and_w(t_mat_cell *cost_mat_1d, size_t n_columns);

size_t *sol_start_cw(t_mat_cell *cost_mat_2d, size_t size);
/**
 * @brief false implementation of CW. Efficient, but false.
 *
 * @param cost_mat_2d cost matrix 2d flattened to 1d.
 * @param n_columns size of the solution.
 * @return a false CW ordering array
 */
size_t *sol_start_cw_tentative(const t_mat_cell *restrict const cost_mat_2d,
                                  size_t size);

t_cost pivot_first(const size_t *sol_1d, size_t *new_sol_1d, t_cost cost,
                   struct matrix nighb_deltas, struct matrix cost_matrix);

t_cost pivot_best(const size_t *const sol_1d, size_t *new_sol_1d,
                  const t_cost cost, struct matrix nighb_deltas,
                  struct matrix cost_matrix);

t_cost it_imp_lop(t_mat_cell *cost_mat_2d, size_t cost_mat_dim,
                  size_t *const sol_1d, t_cost cost,
                  t_fptr_pivot_rule fptr_pivot_rule,
                  t_fptr_neighborhood fptr_neighborhood);

t_cost vnd_lop(t_mat_cell *cost_mat_2d, size_t cost_mat_dim,
               size_t *const sol_1d, t_fptr_pivot_rule fptr_pivot_rule,
               t_fptr_neighborhood *fptr_neighborhood_1,
               const ushort n_neighb_vnd);
#endif
