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
#include <stdlib.h>

struct neighb {
  t_sizemat *neighborhood_modif_2d;
  t_sizemat n_rows;
  t_sizemat n_columns;
};

typedef void (*t_fptr_pivot_rule)(unsigned long *cost, t_sizemat *new_sol_1d,
                                  t_sizemat *sol_1d, struct neighb *neigh_pot,
                                  t_mat_cell **matrix);
typedef struct neighb (*t_fptr_neighborhood)(t_sizemat n_columns);
typedef t_sizemat *(*t_fptr_sol_start)(t_mat_cell **CostMat, t_sizemat size);

extern t_mat_cell **CostMat;

t_cost computeCost(t_sizemat *lo);
void createRandomSolution(t_sizemat *s);

/**
 * @brief Compute the prefix sum of each row of a matrix independently.
 * 0 1 2 3 4 5 6 7 8 9 -> 0 1 3 6 10 17 25 34
 *
 * @param mat Matrix from which to compute the prefix sum on each row.
 * @param n_rows dimension of mat
 * @param n_columns dimension of mat
 * @return A new 2d matrix pointer.
 */
t_mat_cell *prefix_sum_per_row_2d(t_mat_cell **mat, t_sizemat n_rows,
                                  t_sizemat n_columns);

ulong get_n_transpose(uint size);
/**
 * @brief get_all_possible transposes in a vector of size n_columns
 *
 * @param transposes 2d Vector in which all possible transposes will be filled
 * at each row.
 * @param n_columns size of vector to transpose.
 * @param n_rows number of possible transposes.
 */
void get_transpose(t_sizemat *transposes, t_sizemat n_columns, ulong n_rows);
struct neighb neighborhood_tranpose(t_sizemat n_columns);

uint get_n_exchange(t_sizemat size);
void get_exchange(t_sizemat *exchanges, t_sizemat n_columns, uint n_rows);
struct neighb neighborhood_exchange(t_sizemat n_columns);

ulong get_n_inserts(uint size);
void get_inserts(t_sizemat *insertions, t_sizemat n_columns, ulong n_rows);
struct neighb neighborhood_insert(t_sizemat n_columns);

t_sizemat *sol_start_random(t_mat_cell **mat, t_sizemat n_columns);

/**
 * @brief Chenery and Watanabe (CW) greedy heuristic.
 *
 * @param mat Cost matrix.
 * @param n_columns Dimension of the square matrix
 * @return A array of the CW ordering.
 */
t_sizemat *sol_start_c_and_w(t_mat_cell **mat, t_sizemat n_columns);

void neighb_modif(t_sizemat *new_sol_1d, const t_sizemat *sol_1d,
                  const t_sizemat *neighb_pot_1d, t_sizemat n_columns);

void pivot_first(unsigned long *cost, t_sizemat *new_sol_1d, t_sizemat *sol_1d,
                 struct neighb *neigh_pot, t_mat_cell **matrix);
void pivot_best(unsigned long *cost, t_sizemat *new_sol_1d, t_sizemat *sol_1d,
                struct neighb *neigh_pot, t_mat_cell **matrix);

void lop(t_fptr_sol_start fptr_sol_start, t_fptr_pivot_rule fptr_pivot_rule,
         t_fptr_neighborhood fptr_neighborhood);

#endif
