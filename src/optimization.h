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

/**
 * @brief Compute the cost of a LOP. This method is not efficient as it is of
 * complexity O((n^2)/2). It is only meant to be used when the initial cost is
 * not yet known.
 *
 * @param cost_mat_2d Cost matrix of the LOP instance.
 * @param Ordering (solution) of the LOP instance.
 * @param size Lenght of sol_1d and shape of cost_mat_2d (size, size).
 * @return Sum of the uper left triangle with the sol_1d applied.
 */
t_cost computeCost(const t_mat_cell *restrict const cost_mat_2d,
                   const size_t *const lo, const size_t size);

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
 * @brief Compute efficiently the cost difference IF a swap of two elemen of a
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
t_delta_cost cost_if_swap_delta(const t_mat_cell *const cost_mat_2d,
                                const size_t *const sol_1d, const size_t size,
                                const size_t i, const size_t j);

t_delta_cost rand_swaps_with_delta(const t_cost *restrict const cost_mat,
                                   size_t *restrict const array,
                                   const size_t size, const float rate);

/**
 * @brief Compute the prefix sum of each row of a matrix independently.
 * 0 1 2 3 4 5 6 7 8 9 -> 0 1 3 6 10 17 25 34
 *
 * @param mat Matrix from which to compute the prefix sum on each row.
 * @param n_rows dimension of mat
 * @param n_columns dimension of mat
 * @return A new 2d matrix pointer.
 */
t_mat_cell *prefix_sum_per_row_2d(const t_mat_cell *const mat, size_t n_rows,
                                  const size_t n_columns);

/**
 * @brief Get the number of possible transpose operations on a array of size
 * size.
 *
 * @param size Size of the array.
 * @return Integral number of possible transpose operations a array of size
 * size.
 */
size_t get_n_transpose(const size_t size);
/**
 * @brief Get the number of possible insert operations on a array of size size.
 *
 * @param size Size of the array.
 * @return Integral number of possible insert operations a array of size size.
 */
size_t get_n_inserts(const size_t size);
/**
 * @brief Get the number of possible exchange operations on a array of size
 * size.
 *
 * @param size Size of the array.
 * @return Integral number of possible exchange operations a array of size size.
 */
size_t get_n_exchange(const size_t size);

/**
 * @brief Compute the best or first cost difference in all possible neighborhood
 * modifications by transpose. the initial solution is directly modified
 * accoding the the cost difference found.
 *
 * @param cost_mat_2d Cost matrix of the LOP
 * @param sol_1d Initial ordering (solution) of the LOP to be modified.
 * Neighorhood will be searched from this initial solution.
 * @param size lenght of sol_1d and shape (size, size) of cost_mat_2d.
 * @param is_first If false, the best cost difference will be returned.
 * @return The cost difference between the initial solution and the new
 * solution.
 */
t_delta_cost cost_delta_transpose(const t_mat_cell *restrict const cost_mat_2d,
                                  size_t *const sol_1d, const size_t size,
                                  const bool is_first);
/**
 * @brief Compute the best or first cost difference in all possible neighborhood
 * modifications by exchange. the initial solution is directly modified accoding
 * the the cost difference found.
 *
 * @param cost_mat_2d Cost matrix of the LOP
 * @param sol_1d Initial ordering (solution) of the LOP to be modified.
 * Neighorhood will be searched from this initial solution.
 * @param size lenght of sol_1d and shape (size, size) of cost_mat_2d.
 * @param is_first If false, the best cost difference will be returned.
 * @return The cost difference between the initial solution and the new
 * solution.
 */
t_delta_cost cost_delta_exchange(const t_mat_cell *restrict const cost_mat_2d,
                                 size_t *const sol_1d, const size_t size,
                                 const bool is_first);
/**
 * @brief Compute the best or first cost difference in all possible neighborhood
 * modifications by exchange. the initial solution is directly modified accoding
 * the the cost difference found.
 *
 * @param cost_mat_2d Cost matrix of the LOP
 * @param sol_1d Initial ordering (solution) of the LOP to be modified.
 * Neighorhood will be searched from this initial solution.
 * @param size lenght of sol_1d and shape (size, size) of cost_mat_2d.
 * @param is_first If false, the best cost difference will be returned.
 * @return The cost difference between the initial solution and the new
 * solution.
 */
t_delta_cost cost_delta_insert(const t_mat_cell *restrict const cost_mat_2d,
                               size_t *const sol_1d, const size_t size,
                               const bool is_first);

/**
 * @brief Generate a random ordering (solution) for the lop problem.
 *
 * @param mat Cost matrix. Isn't used in this fuction. The parameter is here for
 * the function signature.
 * @param n_columns Size of the random vector to generate.
 * @return A pointer of a new allocated memory array with random values ranging
 * from 0 to n_columns-1.
 */
size_t *sol_start_random(const t_mat_cell *const mat, size_t n_columns);

/**
 * @brief Chenery and Watanabe (CW) greedy heuristic.
 *
 * @param mat Cost matrix.
 * @param size Dimension of the square matrix
 * @return A array of the CW ordering.
 */
size_t *sol_start_cw(const t_mat_cell *const cost_mat_2d, size_t size);

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
t_delta_cost lop_iter_impr(
    const t_mat_cell *const cost_mat_2d, const size_t mat_cost_dim,
    size_t *const sol_1d, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration fptr_delta_neigh_exploration);

/**
 * @brief Apply the variables neighborhood descent (VND) algorithm to a solution
 * of the lop, with the given neighborhood exploration function and pivot rule.
 *
 * @param cost_mat_2d matrix of costs for the lop instance of shape
 * (cost_mat_dim, cost_mat_dim).
 * @param cost_mat_dim shape of cast_mat_2d and lenght of sol_1d.
 * @param sol_1d initial solution of the lop problem. After execution, point to
 * the best solution found.
 * @param pivot_rule enum of the pivot rule to use
 * @param fptr_delta_neigh_exploration function pointers methods of neighborhood
 * explaration in order to be used (insert/transpose/exchange).
 * @param n_neighb_vnd lenght of fptr_delat_neigh_explaration
 * @return resulting lop cost of the the solution sol_1d
 */
t_delta_cost vnd_lop(
    const t_mat_cell *const cost_mat_2d, const size_t cost_mat_dim,
    size_t *const sol_1d, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_exploration,
    const ushort n_neighb_vnd);

typedef t_delta_cost (*t_fptr_vnd_lop)(
    const t_mat_cell *const cost_mat_2d, size_t mat_cost_dim,
    size_t *const sol_1d, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_exploration,
    const ushort n_neighb_vn);

#endif
