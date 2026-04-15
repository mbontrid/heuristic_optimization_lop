#ifndef _OPTIM_OBSOLET_H_
#define _OPTIM_OBSOLET_H_

#include "optimization.h"

/**
 * @brief get_all_possible transposes in a vector of size n_columns
 *
 * @param transposes 2d Vector in which all possible transposes will be filled
 * at each row.
 * @param n_columns size of vector to transpose.
 * @param n_rows number of possible transposes.
 */
size_t *neighb_transpose_deltas(size_t *const n_rows, const size_t n_columns);
size_t *neighb_exchange_deltas(size_t *const n_rows, const size_t n_columns);
size_t *neighb_insert_deltas(size_t *const n_rows, const size_t n_columns);

/**
 * @brief false implementation of CW. Efficient, but false.
 *
 * @param cost_mat_2d cost matrix 2d flattened to 1d.
 * @param n_columns size of the solution.
 * @return a false CW ordering array
 */
size_t *sol_start_cw_tentative(const t_mat_cell *const cost_mat_2d,
                               size_t size);

#endif
