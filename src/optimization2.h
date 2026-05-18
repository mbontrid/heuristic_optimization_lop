
#pragma once

#include "utilities.h"
#include <sys/types.h>

/**
 * @brief Iterated local search for the lop.
 *
 * @param cost_mat Cost matrix of the lop.
 * @param sol_1d solution of the lop index
 * @param size shape of cost_mat (size, size) and size of sol_1d (size).
 * @param perturb_rate perturbation rate to apply at each local optimum.
 * Perturbation are applied with preturb_rate swaps on solution.
 * @param n_try Number of tries to escape from a local optimum before giving up.
 * @param worse solution delta due to perturbation acceptable. Higher value has
 * greater change to escape local minimum but can accept worse solution.
 * @param pivot_rule pivot rule fro vnd.
 * @param fptr_delta_neigh_exploration Neighborhood exploration function pointer
 * for vnd.
 * @param n_neighb_vn numvber of neighborhood to explore in vnd.
 * @return cost_delta
 */

t_cost_delta dpx_crossover(const t_cost *const cost_mat,
                           size_t *const p1_offspring, const size_t *const p2,
                           size_t size);

t_cost_delta ob_crossover(const t_cost *const cost_mat,
                          size_t *const p1_offspring, const size_t *const p2,
                          const size_t size, const float cross_rate);

bool accept_worse(const t_cost_delta delta, const t_cost worse_bracket);

t_cost_delta ils(const t_cost *const cost_mat, size_t *const sol_1d,
                 size_t size, const float perturb_rate, const size_t n_try,
                 const t_cost worse, enum pivot_enum pivot_rule,
                 t_fptr_delta_neigh_exploration *fptr_delta_neigh_exploration,
                 ushort n_neighb_vn);

t_cost_delta
memetic(const t_cost *const cost_mat, size_t *const sol_1d, size_t size,
        const size_t n_population, const size_t generation,
        const float offspring_cross_mut, const size_t n_offspring,
        const float mutation_rate, const float cross_rate,
        enum pivot_enum pivot_rule,
        t_fptr_delta_neigh_exploration *fptr_delta_neigh_explaration,
        ushort n_neighb_vn);
