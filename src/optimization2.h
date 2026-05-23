
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

/**
 * @brief Acceptance heuristic. Can accept worse solution than previous
 * solution.
 *
 * @param delta Delta between the new solution and the previous solution.
 * @param worse_bracket negative dalta accetpable.
 * @return Acceted or not.
 */
bool is_accetp_worse(const t_delta_cost delta, const t_cost worse_bracket);

/**
 * @brief Iteratid local search for the LOP.
 *
 * @param cost_mat Cost matrix of the LOP.
 * @param sol_1d Sol of the LOP on which the ILS will start. After execution,
 * the result will stored at this location.
 * @param size Size of the array sol_1d and shape of cost_mat (size, size).
 * @param perturb_rate Perturbation rate to apply at solution when needed. Rate
 * of swap applied to solution.
 * @param n_try Number of try to escape the local optimum before giving up.
 * @param worse Worse solution acceptance.
 * @param pivot_rule Pivoting rule for the VND on LOP. First or Best
 * @param fptr_delta_neigh_exploration List of neighborhood exploration function
 * pointer for VND to apply iteratively.
 * @param n_neighb_vn size of fptr_delta_neigh_exploration, i.e. number of
 * neighborhood to explore in VND.
 * @return cost_delat of sol_1d before and after ILS.
 */
t_delta_cost
ils(const t_cost *const cost_mat, size_t *const sol_1d, const t_cost start_cost,
    size_t size, const float perturb_rate, const size_t n_try,
    const t_cost worse, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_exploration,
    const ushort n_neighb_vn);

void populate(
    const t_cost *const cost_mat, size_t *const pop_2d,
    t_cost *const pop_cost_1d, const size_t size, const size_t n_population,
    const size_t from, const enum pivot_enum pivot_rule,
    const ushort n_neighb_vn,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration);

t_delta_cost dpx_crossover(const t_cost *const cost_mat,
                           size_t *const p1_offspring, const size_t *const p2,
                           size_t size);

t_delta_cost ob_crossover(const t_cost *restrict const cost_mat,
                          size_t *const p1_offspring, const size_t *const p2,
                          const size_t size, const float cross_rate);

void crossover(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const crossover_2d,
    t_cost *const crossover_cost_2d, const size_t n_crossover,
    const float cross_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn);

void mutation(
    const t_cost *restrict const cost_mat, size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const mutation_2d,
    t_cost *const mutation_cost_2d, const size_t n_mutation,
    float mutation_rate, enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    ushort n_neighb_vn);

void offspring(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *restrict const pop_2d,
    const t_cost *restrict const pop_cost_1d, const size_t n_population,
    size_t *const offspring_2d, t_cost *const offspring_cost_2d,
    const size_t n_offspring, const float cross_rate_mut,
    const float cross_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn, float mutation_rate);

void select_n_best(size_t *restrict const pop_2d,
                   t_cost *restrict const pop_cost_1d,
                   const size_t *restrict const pop_off_2d,
                   const t_cost *restrict const pop_off_cost_2d,
                   const size_t n_pop, const size_t n_pop_off,
                   const size_t size);

void diversification(
    const t_cost *restrict const cost_mat, size_t size, size_t *const pop_2d,
    t_cost *const pop_cost_1d, const size_t n_population, const size_t n_best,
    const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn);

t_delta_cost
memetic(const t_cost *const cost_mat, size_t *const sol_1d, size_t size,
        const size_t n_population, const size_t n_diversi_try,
        const size_t n_mean_try, const float cross_rate_mut,
        const size_t n_offspring, const float mutation_rate,
        const float cross_rate, const enum pivot_enum pivot_rule,
        const t_fptr_delta_neigh_exploration *fptr_delta_neigh_explaration,
        const ushort n_neighb_vn);
