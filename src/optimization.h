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
  const t_mat_cell *mat_2d;
  size_t n_rows;
  size_t n_columns;
};

extern t_mat_cell **cost_mat_2d;

t_cost computeCost(const t_mat_cell *const cost_mat, const size_t *const lo,
                   size_t size);

/**
 * @brief get the cost difference if a neithberhood modification is applied to a
 * solution. Contrary to cost_delta functions, this function is not specific to
 * a swap but can take multiple modifcations.
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
t_cost_delta cost_swap_delta(const t_mat_cell *const cost_mat_2d,
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

t_cost_delta cost_delta_transpose(t_mat_cell *cost_mat_2d, size_t *const sol_1d,
                                  size_t size, bool is_first);
t_cost_delta cost_delta_exchange(t_mat_cell *cost_mat_2d, size_t *const sol_1d,
                                 size_t size, bool is_first);
t_cost_delta cost_delta_insert(t_mat_cell *cost_mat_2d, size_t *const sol_1d,
                               size_t size, bool is_first);

size_t *sol_start_random(t_mat_cell *mat, size_t n_columns);

/**
 * @brief Chenery and Watanabe (CW) greedy heuristic.
 *
 * @param mat Cost matrix.
 * @param size Dimension of the square matrix
 * @return A array of the CW ordering.
 */
size_t *sol_start_cw(t_mat_cell *cost_mat_2d, size_t size);

/**
 * @brief Apply the pivot_rule neighb by neighb until a local optimum is
 * reached.
 *
 * @param cost_mat_2d Instance of cost matrix for the lop.
 * @param cost_mat_dim dimension of the square cost matrix and the solution
 * array.
 * @param sol_1d solution array pointer. The result is stored at this pointer.
 * @param pivot_rule fist or best pivot rule enum.
 * @param fptr_delta_neigh_exploration pointer to function of neighborhood
 * modification.
 * @return difference in cost for sol_1d between the local optimum reached and
 * the initial solution.
 */
t_cost_delta
it_imp_lop(t_mat_cell *cost_mat_2d, size_t cost_mat_dim, size_t *const sol_1d,
           enum pivot_enum pivot_rule,
           t_fptr_delta_neigh_exploration fptr_delta_neigh_exploration);

/**
 * @brief Apply the variables neighborhood descent (VND) algorithm to a solution
 * of the lop, with the given neighborhood exploration function and pivot rule.
 *
 * @param cost_mat_2d [TODO:parameter]
 * @param cost_mat_dim [TODO:parameter]
 * @param sol_1d [TODO:parameter]
 * @param pivot_rule [TODO:parameter]
 * @param fptr_delta_neigh_exploration [TODO:parameter]
 * @param n_neighb_vnd [TODO:parameter]
 * @return [TODO:return]
 */
t_cost vnd_lop(t_mat_cell *cost_mat_2d, size_t cost_mat_dim,
               size_t *const sol_1d, enum pivot_enum pivot_rule,
               t_fptr_delta_neigh_exploration *fptr_delta_neigh_exploration,
               const ushort n_neighb_vnd);
#endif
