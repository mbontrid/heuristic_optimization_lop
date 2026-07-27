
#pragma once

#include "utilities.h"
#include <sys/types.h>

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
 * @return cost_delat of sol_1d after new solution.
 */
t_delta_cost
ils(const t_cost *const cost_mat, size_t *const sol_1d, const t_cost start_cost,
    size_t size, const float perturb_rate, const size_t n_try,
    const t_cost worse, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_exploration,
    const ushort n_neighb_vn);

/**
 * @brief Generate a population of solution for the LOP. The new individuals are
 * uniques and are generated randomly with a then applied vnd optimization.
 *
 * @param cost_mat Cost matrix of the LOP.
 * @param pop_2d Population of solution for the LOP. The individuals from from
 * to the end will be rempliced by new individuals.
 * @param pop_cost_1d Cost of each individual. The costs from form to the end
 * will be replaced.
 * @param size size of the LOP and its solution.
 * @param n_population Number of individuals in the population.
 * @param from From which individual to the end of the population has to be
 * regenerated.
 * @param pivot_rule Pivoting rule for the VND on LOP. First or Best.
 * @param n_neighb_vn lenght of fptr_delta_neigh_explaration, i.e. number of
 * neighborhood to explore in VND.
 * @param fptr_delta_neigh_explaration Array of ointer to neighborhood
 * exploration function for iterated local search.
 */
void populate(
    const t_cost *const cost_mat, size_t *const pop_2d,
    t_cost *const pop_cost_1d, const size_t size, const size_t n_population,
    const size_t from, const enum pivot_enum pivot_rule,
    const ushort n_neighb_vn,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration);

/**
 * @brief The offspring inherits the elements that have the same position in
 * both parents; these are put into the same position as in the parents. The
 * other elements are assigned randomly between those positions that have not
 * yet been chosen. This results in an offspring that, on average, has the same
 * distance from both parents.
 *
 * @param cost_mat Cost matrix of the LOP.
 * @param p1_offspring Offspring solution derived from parent 1.
 * @param p2 Parent 2 solution.
 * @param size Size of the LOP and its solution.
 * @return Cost delta of the offspring solution. (parent 1).
 */
t_delta_cost dpx_crossover(const t_cost *const cost_mat,
                           size_t *const p1_offspring, const size_t *const p2,
                           size_t size);

/**
 * @brief In the first phase of the order-based crossover, the solution of the
 * first parent is copied to the offspring. In the second phase it selects k
 * positions, 0 < k < n, and orders the elements in these k positions according
 * to their order in the second parent.
 *
 * @param cost_mat Cost matrix of the LOP.
 * @param p1_offspring Offspring solution derived from parent 1.
 * @param p2 Parent 2 solution.
 * @param size Size of the LOP and its solution.
 * @param cross_rate Define the value of k propartionally to the size of the
 * solution. 0 < k < n.
 * @return Cost delat of the offspring solution (parent 1).
 */
t_delta_cost ob_crossover(const t_cost *restrict const cost_mat,
                          size_t *const p1_offspring, const size_t *const p2,
                          const size_t size, const float cross_rate);

/**
 * @brief Generate new LOP solutions
 *
 * @param cost_mat Cost matrix of the LOP.
 * @param size size of solutions and shape of cost_mat.
 * @param pop_2d array of array of solutions.
 * @param pop_cost_1d array of cost of pop_2d.
 * @param n_population Number of individuals (soultions).
 * @param crossover_2d array of array to be populatied with crossover solutions.
 * @param crossover_cost_2d array to be populated with cost of crossover_2d
 * soultions.
 * @param n_crossover number of crossover solutions.
 * @param cross_rate Cross rate to apply by the crosover algorithm. Only
 * ob_crossover use this parameter.
 * @param pivot_rule Pivoting rule for the VND on LOP. First or Best.
 * @param fptr_delta_neigh_explaration Pointer to neighborhood exploration
 * function for iterated local search.
 * @param n_neighb_vn Number of pointer in fptr_delta_neigh_explaration, i.e.
 * number of neighborhood to explore in VND.
 */
void crossover(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const crossover_2d,
    t_cost *const crossover_cost_2d, const size_t n_crossover,
    const float cross_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn);

/**
 * @brief Apply random swaps on n_mutation randomly selected  solutions with
 * mutation_rate rate.
 *
 * @param cost_mat Cost matrix of the LOP.
 * @param size Size of solutions and shape of cost_mat.
 * @param pop_2d Array of array of soultions (individuals).
 * @param pop_cost_1d LOP cost of each individual in pop_2d.
 * @param n_population nubmer of individuals.
 * @param mutation_2d Array of array on which the mutated solutions will be
 * stored.
 * @param mutation_cost_2d Array on which the cost of each new mutated solution
 * will be stored.
 * @param n_mutation number of mutated solutions to generete.
 * @param mutation_rate Rate of swaps to apply on a solution to be mutated.
 * @param pivot_rule Pivoting ruele for the VND on LOP. First or Best.
 * @param fptr_delta_neigh_explaration Pointer to neighborhood exploration
 * function for iterated local search.
 * @param n_neighb_vn Number of pointer in fptr_delta_neigh_explaration, i.e.
 * number of neighborhood to explore in VND.
 */
void mutation(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const mutation_2d,
    t_cost *const mutation_cost_2d, const size_t n_mutation,
    const float mutation_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn);

/**
 * @brief
 *
 * @param cost_mat Cost matrix of the LOP.
 * @param size Size of LOP solutions and shape of cost_mat.
 * @param pop_2d Array of array of LOP solutions.
 * @param pop_cost_1d Array of LOP cost of pop_2d solutions.
 * @param n_population Number of individuals in the population.
 * @param offspring_2d Array of Array of offspring. This is a result array.
 * @param offspring_cost_2d LOP cost of ech offspring solution. This is a result
 * array.
 * @param n_offspring Number of offspring to be generated.
 * @param cross_rate_mut Rate of crossover over mutation. 1 Means the
 * offspring_2d will be populated only with crossover solutions. 0 Means the
 * offspring_2d will be populated only with mutation solutions.
 * @param cross_rate Cross rate of the crossover algorithm. Only ob_crossover
 * use this parameter.
 * @param pivot_rule Pivoting rule for the VND on LOP. First or Best.
 * @param fptr_delta_neigh_explaration Pointer to neighborhood exploration
 * function for iterated local search.
 * @param n_neighb_vn Nubmer of pointer in fptr_delta_neigh_explaration, i.e.
 * number of neighborhood to explore in VND.
 * @param mutation_rate Rate of mutation on a sulution. A longer solution will
 * have more mutation.
 */
void offspring(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *restrict const pop_2d,
    const t_cost *restrict const pop_cost_1d, const size_t n_population,
    size_t *const offspring_2d, t_cost *const offspring_cost_2d,
    const size_t n_offspring, const float cross_rate_mut,
    const float cross_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn, float mutation_rate);

/**
 * @brief Retrieve in pop_2d the n_pop best solutions based on their cost in
 * pop_cost_1d.
 *
 * @param pop_2d Best solutions in offspring_2d. This is a result array.
 * @param pop_cost_1d Population cost of pop_2d. This is a result array.
 * @param pop_off_2d Offspring solutions to select the best from.
 * @param pop_off_cost_2d Offspring cost of pop_off_2d.
 * @param n_pop Number of individuals to be selected from pop_off_2d and
 * pop_off_cost_2d.
 * @param n_pop_off Number of offspring.
 * @param size Size of individuals in pop_2d and pop_off_2d.
 */
void select_n_best(size_t *restrict const pop_2d,
                   t_cost *restrict const pop_cost_1d,
                   const size_t *restrict const pop_off_2d,
                   const t_cost *restrict const pop_off_cost_2d,
                   const size_t n_pop, const size_t n_pop_off,
                   const size_t size);

/**
 * @brief Repopulatee all solution but the n_keep_front.
 *
 * @param cost_mat LOP cost matrix.
 * @param size Size of solutions and shape of cost_mat.
 * @param pop_2d Array of array of solutions from which only the n_keep_front
 * will be kept. The rest will be regenerated.
 * @param pop_cost_1d LOP cost of pop_2d. The cost of the n_keep_front will be
 * kept. The rest will be regenerated.
 * @param n_population Number of individuals in the population.
 * @param n_first Number of first solution to keep.
 * @param pivot_rule Pivoting rule for the VND on LOP. First or Best.
 * @param fptr_delta_neigh_explaration Array of pointer to neighborhood
 * exploration function for iterated local search.
 * @param n_neighb_vn Number of pointer in fptr_delta_neigh_explaration, i.e.
 * number of neighborhood to explore in VND.
 */
void diversification(
    const t_cost *restrict const cost_mat, size_t size, size_t *const pop_2d,
    t_cost *const pop_cost_1d, const size_t n_population,
    const size_t n_keep_front, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn);

/**
 * @brief Stochastic local search by population for the LOP. Use mutation and
 * crossover to find new solutions.
 *
 * @param cost_mat LOP matrix.
 * @param sol_1d Current solution of the lop. The result will be stored at this
 * location.
 * @param size size of solution and shape of cost_mat.
 * @param n_population Number of individuals to keep after each generation.
 * @param n_diversi_try Number of diversification to try before ending the
 * algorithm. (Keep the best individual and regenerate the rest of the
 * population)
 * @param n_mean_try Number of try with meant delta cost in the population
 * before diversifying.
 * @param cross_rate_mut rate of crossover to in population. The rest will be
 * mutated.
 * @param n_offspring number of offspring at each generation.
 * @param mutation_rate Rate of mutation on a sulution. A longer solution will
 * have more mutation.
 * @param cross_rate Rate of crossover between two solutions.
 * @param pivot_rule first or best pivot for the iterated local search.
 * @param fptr_delta_neigh_explaration pointer to neighborhood exploration
 * function for iterated local search.
 * @param n_neighb_vn Number of pointer in fptr_delta_neigh_explaration, i.e.
 * number of neighborhood to explore in VND.
 * @return Delta cost between the initial solution and the best solution found.
 */
t_delta_cost
memetic(const t_cost *const cost_mat, size_t *const sol_1d, size_t size,
        const size_t n_population, const size_t n_diversi_try,
        const size_t n_mean_try, const float cross_rate_mut,
        const size_t n_offspring, const float mutation_rate,
        const float cross_rate, const enum pivot_enum pivot_rule,
        const t_fptr_delta_neigh_exploration *fptr_delta_neigh_explaration,
        const ushort n_neighb_vn);
