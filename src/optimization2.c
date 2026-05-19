#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"
#include "optimization.h"
#include "optimization2.h"
#include "utilities.h"

bool accept_worse(const t_cost_delta delta, const t_cost worse_bracket) {
  assert(worse_bracket >= 0);
  return delta > worse_bracket;
}

t_cost_delta
ils(const t_cost *const cost_mat, size_t *const sol_1d, size_t size,
    const float perturb_rate, const size_t n_try, const t_cost worse,
    enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *fptr_delta_neigh_exploration,
    ushort n_neighb_vn) {

  t_cost_delta delta = 0;
#ifndef NDEBUG
  size_t *const assert_sol_1d_old = malloc(size * sizeof(size_t));
  memcpy(assert_sol_1d_old, sol_1d, size * sizeof(size_t));
#endif

  size_t try = n_try;
  size_t *const new_sol_1d = malloc(size * sizeof(size_t));
  memcpy(new_sol_1d, sol_1d, size * sizeof(size_t));

  while (try--) {
    t_cost_delta new_delta = 0;
    new_delta += rand_swaps(cost_mat, new_sol_1d, size, perturb_rate);
    new_delta += vnd_lop(cost_mat, size, new_sol_1d, pivot_rule,
                         fptr_delta_neigh_exploration, n_neighb_vn);

    if (accept_worse(new_delta - delta, worse)) {
      PVERB("Accepting new solution with delta %ld\n", new_delta);
      // copy the new sol to sol
      memcpy(sol_1d, new_sol_1d, size * sizeof(size_t));
      delta += new_delta;
      try = n_try;
    } else {
      PVERB("Rejecting new solution with delta %ld and %zu tries left\n",
            new_delta, try);
      // revert new_sol to old sol
      memcpy(new_sol_1d, sol_1d, size * sizeof(size_t));
    }
  }

  assert(delta ==
         (t_cost_delta)computeCost(cost_mat, sol_1d, size) -
             (t_cost_delta)computeCost(cost_mat, assert_sol_1d_old, size));
#ifndef NDEBUG
  free(assert_sol_1d_old);
#endif

  return delta;
}

void populate(
    const t_cost *const cost_mat, size_t *const pop_2d,
    t_cost *const pop_cost_1d, const size_t size, const size_t n_population,
    const size_t from, const enum pivot_enum pivot_rule,
    const ushort n_neighb_vn,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration) {

  assert(from >= 0 && from <= n_population);

  // generating incremental solution and its cost as base for all individuals
  const size_t *const tmp_sol_1d = generate_incr_vector(size);
  t_cost incr_sol_cost = computeCost(cost_mat, tmp_sol_1d, size);

  // generating population
  for (size_t i = from; i < n_population; i++) {
    size_t *current = &pop_2d[i * size];
    t_cost *const current_cost = &pop_cost_1d[i];

    do {
      DPRINTF("Generating solution for individual %zu\n", i);
      memcpy(current, tmp_sol_1d, size * sizeof(size_t));
      *current_cost = incr_sol_cost;
      // random swap with rate 1 make a random solution
      *current_cost += rand_swaps(cost_mat, current, size, 1);
      *current_cost += vnd_lop(cost_mat, size, current, pivot_rule,
                               fptr_delta_neigh_explaration, n_neighb_vn);

    } while (is_array_in_arrays(current, pop_2d, size, i));
  }
  free((size_t *)tmp_sol_1d);

#ifndef NDEBUG
  for (size_t i = from; i < n_population - from; i++) {
    assert(pop_cost_1d[i] == computeCost(cost_mat, &pop_2d[i * size], size));
  }
#endif
}

t_cost_delta dpx_crossover(const t_cost *const cost_mat,
                           size_t *const p1_offspring, const size_t *const p2,
                           size_t size) {
  assert(p1_offspring);
  assert(p2);
#ifndef NDEBUG
  t_cost assert_cost_before = computeCost(cost_mat, p1_offspring, size);
#endif

  size_t *indexes = malloc(size * sizeof(size_t));
  assert(indexes);
  size_t to_move = 0;

  for (size_t i = 0; i < size; i++) {
    if (p1_offspring[i] != p2[i]) {
      indexes[to_move++] = i;
    }
  }

  t_cost_delta delta = 0;

  while (to_move > 2) {
    size_t mov1_index = randInt(0, to_move - 1);
    size_t mov1 = indexes[mov1_index];
    assert(to_move > 0);
    indexes[mov1_index] = indexes[--to_move];
    size_t mov2_index = randInt(0, to_move - 1);
    size_t mov2 = indexes[mov2_index];
    assert(to_move > 0);
    indexes[mov2_index] = indexes[--to_move];

    delta += cost_swap_delta(cost_mat, p1_offspring, size, mov1, mov2);
    swap(p1_offspring, mov1, mov2);
  }

  free(indexes);
  assert(assert_cost_before + delta ==
         computeCost(cost_mat, p1_offspring, size));
  return delta;
}

t_cost_delta ob_crossover(const t_cost *restrict const cost_mat,
                          size_t *const p1_offspring, const size_t *const p2,
                          const size_t size, const float cross_rate) {
  assert(p1_offspring);
  assert(p2);
  assert(cross_rate >= 0 && cross_rate <= 1);
  assert(size > 1);
  assert(get_max_array(p1_offspring, size) < size);
  assert(get_max_array(p2, size) < size);

  t_cost_delta delta = 0;

  size_t n_cross = (size_t)(cross_rate * size);
  if (n_cross == 0) {
    n_cross = 1;
  } else if (n_cross >= size) {
    n_cross = size - 1;
  }

#ifndef NDEBUG
  size_t *const assert_p1_offspring_before = malloc(size * sizeof(size_t));
  memcpy(assert_p1_offspring_before, p1_offspring, size * sizeof(size_t));
#endif

  const size_t *const to_cross = generate_random_no_rep(size);
  size_t *const selected_positions = malloc(n_cross * sizeof(size_t));
  size_t *const selected_values = malloc(n_cross * sizeof(size_t));
  bool *const is_selected_value = calloc(size, sizeof(bool));
  size_t *const ordered_values = malloc(n_cross * sizeof(size_t));
  assert(to_cross);
  assert(selected_positions);
  assert(selected_values);
  assert(is_selected_value);
  assert(ordered_values);

  for (size_t i = 0; i < n_cross; i++) {
    selected_positions[i] = to_cross[i];
    selected_values[i] = p1_offspring[selected_positions[i]];
    is_selected_value[selected_values[i]] = true;
  }

  ascending_sort(selected_positions, n_cross);

  size_t ordered_count = 0;
  for (size_t i = 0; i < size; i++) {
    const size_t value = p2[i];
    if (is_selected_value[value]) {
      ordered_values[ordered_count++] = value;
    }
  }
  assert(ordered_count == n_cross);

  for (size_t i = 0; i < n_cross; i++) {
    p1_offspring[selected_positions[i]] = ordered_values[i];
  }

  free(ordered_values);
  free(is_selected_value);
  free(selected_values);
  free(selected_positions);
  free((size_t *const)to_cross);

  assert(delta == 0 ||
         !array_equal(p1_offspring, assert_p1_offspring_before, size));
  assert(computeCost(cost_mat, p1_offspring, size) -
             computeCost(cost_mat, assert_p1_offspring_before, size) ==
         delta);
#ifndef NDEBUG
  free(assert_p1_offspring_before);
#endif
  return delta;
}

void crossover(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const crossover_2d,
    t_cost *const crossover_cost_2d, const size_t n_crossover,
    const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn) {

#ifndef NDEBUG
  for (size_t i = 0; i < n_population; i++) {
    assert(pop_cost_1d[i] == computeCost(cost_mat, &pop_2d[i * size], size));
  }
#endif

  for (size_t cross_id = 0; cross_id < n_crossover; cross_id++) {
    t_cost_delta p1_cost = 0;
    // search for a non existing crossover solution until found
    do {
      // select two random parents
      const size_t rand_index_1 = randInt(0, n_population - 1);
      const size_t rand_index_2 = randInt(0, n_population - 1);

      const size_t *const p1 = &pop_2d[rand_index_1 * size];
      const size_t *const p2 = &pop_2d[rand_index_2 * size];
      assert(p1);
      assert(p2);
      p1_cost = pop_cost_1d[rand_index_1];
      assert(p1_cost == computeCost(cost_mat, p1, size));

      // crossover and local search
      memcpy(&crossover_2d[cross_id * size], p1, size * sizeof(size_t));
      p1_cost +=
          dpx_crossover(cost_mat, &crossover_2d[cross_id * size], p2, size);
      assert(p1_cost ==
             computeCost(cost_mat, &crossover_2d[cross_id * size], size));
      p1_cost += vnd_lop(cost_mat, size, &crossover_2d[cross_id * size],
                         pivot_rule, fptr_delta_neigh_explaration, n_neighb_vn);
      assert(p1_cost ==
             computeCost(cost_mat, &crossover_2d[cross_id * size], size));
    } while (is_array_in_arrays(&crossover_2d[cross_id * size], crossover_2d,
                                size, cross_id));
    crossover_cost_2d[cross_id] = p1_cost;

    assert(crossover_cost_2d[cross_id] ==
           computeCost(cost_mat, &crossover_2d[cross_id * size], size));
  }
}

void mutation(
    const t_cost *restrict const cost_mat, size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const mutation_2d,
    t_cost *const mutation_cost_2d, const size_t n_mutation,
    float mutation_rate, enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    ushort n_neighb_vn) {

  for (size_t mut_id = 0; mut_id < n_mutation; mut_id++) {
    t_cost_delta p1_cost = 0;
    // make a random mutation on a random parent until a non existing solution
    // is found
    do {
      const size_t rand_index = randInt(0, n_population - 1);
      // copy parent
      const size_t *const p1 = &pop_2d[rand_index * size];
      memcpy(&mutation_2d[mut_id * size], p1, size * sizeof(size_t));
      p1_cost = pop_cost_1d[rand_index];
      // apply mutation and local search
      p1_cost += rand_swaps(cost_mat, &mutation_2d[mut_id * size], size,
                            mutation_rate);
      p1_cost += vnd_lop(cost_mat, size, &mutation_2d[mut_id * size],
                         pivot_rule, fptr_delta_neigh_explaration, n_neighb_vn);
    } while (is_array_in_arrays(&mutation_2d[mut_id * size], mutation_2d, size,
                                mut_id));
    mutation_cost_2d[mut_id] = p1_cost;

    assert(mutation_cost_2d[mut_id] ==
           computeCost(cost_mat, &mutation_2d[mut_id * size], size));
  }
}

void offspring(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *restrict const pop_2d,
    const t_cost *restrict const pop_cost_1d, const size_t n_population,
    size_t *const offspring_2d, t_cost *const offspring_cost_2d,
    const size_t n_offspring, const float offspring_cross_mut,
    const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn, float mutation_rate) {

  assert(offspring_cross_mut >= 0 && offspring_cross_mut <= 1);

  const size_t n_crossover = (size_t)(offspring_cross_mut * n_offspring);
  const size_t n_mutation = n_offspring - n_crossover;
  assert(n_crossover + n_mutation == n_offspring);

  size_t *restrict const crossover_2d = &offspring_2d[0];
  size_t *restrict const mutation_2d = &offspring_2d[n_crossover * size];

  t_cost *restrict const crossover_cost_2d = &offspring_cost_2d[0];
  t_cost *restrict const mutation_cost_2d = &offspring_cost_2d[n_crossover];

#ifndef NDEBUG
  for (size_t i = 0; i < n_population; i++) {
    assert(pop_cost_1d[i] == computeCost(cost_mat, &pop_2d[i * size], size));
  }
#endif

  crossover(cost_mat, size, pop_2d, pop_cost_1d, n_population, crossover_2d,
            crossover_cost_2d, n_crossover, pivot_rule,
            fptr_delta_neigh_explaration, n_neighb_vn);
  mutation(cost_mat, size, pop_2d, pop_cost_1d, n_population, mutation_2d,
           mutation_cost_2d, n_mutation, mutation_rate, pivot_rule,
           fptr_delta_neigh_explaration, n_neighb_vn);

#ifndef NDEBUG
  for (size_t i = 0; i < n_offspring; i++) {
    assert(offspring_cost_2d[i] ==
           computeCost(cost_mat, &offspring_2d[i * size], size));
  }
#endif
}

void select_best_pop(size_t *restrict const pop_2d,
                     const size_t *restrict const pop_off_2d,
                     const t_cost *restrict const pop_off_cost_2d,
                     const size_t n_pop, const size_t n_pop_off,
                     const size_t size) {
  assert(n_pop < n_pop_off);

  // get the n best cost indices.
  const size_t *const best_pop_id = get_n_best_sorted(
      (size_t *restrict const)pop_off_cost_2d, n_pop, n_pop_off);

  // populate pop_2d with the sortede best.
  for (size_t i = 0; i < n_pop; i++) {
    pop_2d[i] = pop_off_2d[best_pop_id[i] * size];
  }

#ifndef NDEBUG
  for (size_t i = 0; i < n_pop - 1; i++) {
    assert(pop_off_cost_2d[best_pop_id[i]] >=
           pop_off_cost_2d[best_pop_id[i + 1]]); // check if sorted
  }
  for (size_t i = n_pop; i < n_pop_off - 1; i++) {
    assert(pop_off_cost_2d[best_pop_id[n_pop - 1]] >=
           pop_off_cost_2d[best_pop_id[i]]); // check if the rest is worse than
                                             // the worst of the best
  }
#endif

  free((size_t *const)best_pop_id);
}

void diversification(
    const t_cost *restrict const cost_mat, size_t size, size_t *const pop_2d,
    t_cost *const pop_cost_1d, const size_t n_population,
    const size_t n_keep_front, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn) {

  populate(cost_mat, pop_2d, pop_cost_1d, size, n_population, n_keep_front,
           pivot_rule, n_neighb_vn, fptr_delta_neigh_explaration);
}

t_cost_delta
memetic(const t_cost *const cost_mat, size_t *const sol_1d, size_t size,
        const size_t n_population, const size_t n_diversi_try,
        const size_t n_mean_try, const float offspring_cross_mut,
        const size_t n_offspring, const float mutation_rate,
        const float cross_rate, const enum pivot_enum pivot_rule,
        const t_fptr_delta_neigh_exploration *fptr_delta_neigh_explaration,
        const ushort n_neighb_vn) {

  size_t *const pop_off_2d =
      malloc(size * (n_population + n_offspring) * sizeof(*pop_off_2d));
  t_cost *const pop_off_cost_2d =
      malloc((n_population + n_offspring) * sizeof(*pop_off_cost_2d));

  size_t *const pop_2d = &pop_off_2d[0];
  t_cost *const pop_cost_1d = &pop_off_cost_2d[0];

  size_t *const offspring_2d = &pop_off_2d[n_population];
  t_cost *const offspring_cost_2d = &pop_off_cost_2d[n_population];

  PVERB("Populating initial population\n");
  populate(cost_mat, pop_2d, pop_cost_1d, size, n_population, 0, pivot_rule,
           n_neighb_vn, fptr_delta_neigh_explaration);

  t_cost mean_pop_cost = 0;
  size_t mean_try = 0;
  size_t diversi_try = 0;
  do {

    PVERB("Generating offspring\n");
    offspring(cost_mat, size, pop_2d, pop_cost_1d, n_population, offspring_2d,
              offspring_cost_2d, n_offspring, offspring_cross_mut, pivot_rule,
              fptr_delta_neigh_explaration, n_neighb_vn, mutation_rate);

    PVERB("Selecting best population\n");
    // the best sols will be in descending order.
    select_best_pop(pop_2d, pop_off_2d, pop_off_cost_2d, n_population,
                    n_offspring + n_population, size);

    const t_cost new_mean_pop_cost = get_mean(pop_cost_1d, n_population);
    if (new_mean_pop_cost <= mean_pop_cost) {
      mean_try++;
    } else {
      mean_try = 0;
      mean_pop_cost = new_mean_pop_cost;
    }

    if (mean_try >= n_mean_try) {
      PVERB("Diversification try %zu/%zu with mean cost %ld\n", diversi_try + 1,
            n_diversi_try, mean_pop_cost);
      mean_try = n_mean_try;
      diversi_try++;
      diversification(cost_mat, size, pop_2d, pop_cost_1d, n_population, 1,
                      pivot_rule, fptr_delta_neigh_explaration, n_neighb_vn);
    } else {
      diversi_try = 0;
    }
    PVERB(
        "Mean cost of population is %ld with best cost %ld at dversi_try %ld\n",
        mean_pop_cost, pop_cost_1d[0], diversi_try);
  } while (diversi_try < n_diversi_try);
  memcpy(sol_1d, &pop_2d[0], size * sizeof(size_t));
  const t_cost best_cost = pop_cost_1d[0];
  assert(best_cost == computeCost(cost_mat, sol_1d, size));

  free(pop_off_2d);
  free(pop_off_cost_2d);
  return best_cost;
}
