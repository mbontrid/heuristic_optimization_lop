#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"
#include "optimization.h"
#include "optimization2.h"
#include "utilities.h"

bool is_accetp_worse(const t_delta_cost delta, const t_cost worse) {
  assert(worse >= 0);
  return delta > -(const int)worse;
}

t_delta_cost
ils(const t_cost *const cost_mat, size_t *const sol_1d, const t_cost start_cost,
    size_t size, const float perturb_rate, const size_t n_try,
    const t_cost worse, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_exploration,
    const ushort n_neighb_vn) {
  if (perturb_rate <= 0 || perturb_rate > 1) {
    fprintf(stderr, "Error: perturb_rate must be in ]0 ,1]\n");
    exit(EXIT_FAILURE);
  }

  assert(start_cost == computeCost(cost_mat, sol_1d, size));

  t_delta_cost delta = vnd_lop(cost_mat, size, sol_1d, pivot_rule,
                               fptr_delta_neigh_exploration, n_neighb_vn);
#ifndef NDEBUG
  size_t *const assert_sol_1d_old = malloc(size * sizeof(size_t));
  memcpy(assert_sol_1d_old, sol_1d, size * sizeof(size_t));
  const t_cost assert_cost_old = computeCost(cost_mat, sol_1d, size);
  assert((t_delta_cost)computeCost(cost_mat, sol_1d, size) ==
         (t_delta_cost)assert_cost_old);
  assert(n_try > 0);
  assert(worse >= 0);
#endif

  size_t try = 0;
  size_t *const new_sol_1d = malloc(size * sizeof(size_t));
  memcpy(new_sol_1d, sol_1d, size * sizeof(size_t));

  set_result_clock();
  while (try++ < n_try && !is_interrupt_requested()) {

    // swap some value of the solution.
    t_delta_cost delta_delta =
        rand_swaps_with_delta(cost_mat, new_sol_1d, size, perturb_rate);
    // local search on the modified solution.
    delta_delta += vnd_lop(cost_mat, size, new_sol_1d, pivot_rule,
                           fptr_delta_neigh_exploration, n_neighb_vn);

    PVERB("delta=%ld | delta_delta=%ld | try=%zu/%zu\n", delta, delta_delta,
          try, n_try);
    if (is_accetp_worse(delta_delta, worse)) {
      // copy the new sol to sol
      memcpy(sol_1d, new_sol_1d, size * sizeof(size_t));
      delta += delta_delta;
      result_printer(delta + start_cost, sol_1d, size, false);
      try = 0;
    } else {
      // revert new_sol to old sol
      memcpy(new_sol_1d, sol_1d, size * sizeof(size_t));
    }
  }

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

  // generating incremental solution and its cost as base for all individuals.
  // All individuals will be based on this with random swapes while keeping the
  // cost delta history.
  const size_t *const tmp_sol_1d = generate_incr_vector(size);

  // generating population
  for (size_t i = from; i < n_population; i++) {
    size_t *restrict const current = &pop_2d[i * size];
    t_cost *restrict const current_cost = &pop_cost_1d[i];

    memcpy(current, tmp_sol_1d, size * sizeof(size_t));
    // try to generate a new local optimum solution that is not already in the
    // population.
    do {
      DPRINTF("Generating solution for individual %zu\n", i);

      randomize_vector(current, size);

      vnd_lop(cost_mat, size, current, pivot_rule, fptr_delta_neigh_explaration,
              n_neighb_vn);
      *current_cost = computeCost(cost_mat, current, size);

    } while (is_array_in_arrays(current, pop_2d, size, i));
  }
  free((size_t *)tmp_sol_1d);

#ifndef NDEBUG
  for (size_t i = from; i < n_population - from; i++) {
    assert(pop_cost_1d[i] == computeCost(cost_mat, &pop_2d[i * size], size));
  }
#endif
}

t_delta_cost dpx_crossover(const t_cost *const cost_mat,
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

  // make a indexes array with the first to_move elements that are not at the
  // same position in the two parents.
  for (size_t i = 0; i < size; i++) {
    if (p1_offspring[i] != p2[i]) {
      indexes[to_move++] = i;
    }
  }

  t_delta_cost delta = 0;

  // im proud of this but don't understand it anymore.
  //  select two random indexes to move and swap them.
  while (to_move > 2) {
    size_t mov1_index = randInt(0, to_move - 1);
    size_t mov1 = indexes[mov1_index];
    indexes[mov1_index] = indexes[--to_move];
    size_t mov2_index = randInt(0, to_move - 1);
    size_t mov2 = indexes[mov2_index];
    indexes[mov2_index] = indexes[--to_move];

    delta += cost_if_swap_delta(cost_mat, p1_offspring, size, mov1, mov2);
    swap(p1_offspring, mov1, mov2);
    assert(delta == (t_delta_cost)computeCost(cost_mat, p1_offspring, size) -
                        (t_delta_cost)computeCost(cost_mat, p2, size));
  }

  free(indexes);
  assert((t_delta_cost)assert_cost_before + delta ==
         (t_delta_cost)computeCost(cost_mat, p1_offspring, size));
  return delta;
}

t_delta_cost ob_crossover(const t_cost *restrict const cost_mat,
                          size_t *const p1_offspring, const size_t *const p2,
                          const size_t size, const float cross_rate) {
  assert(p1_offspring);
  assert(p2);
  assert(cross_rate >= 0 && cross_rate <= 1);
  assert(size > 1);
  assert(get_max_array(p1_offspring, size) < size);
  assert(get_max_array(p2, size) < size);
  assert(!is_array_overlap(p1_offspring, size * sizeof(size_t), p2,
                           size * sizeof(size_t)));

#ifndef NDEBUG
  size_t *const assert_p1_offspring_before = malloc(size * sizeof(size_t));
  memcpy(assert_p1_offspring_before, p1_offspring, size * sizeof(size_t));
#endif

  t_delta_cost delta = 0;

  size_t n_cross = (size_t)(cross_rate * size);
  assert(n_cross >= 0 && n_cross < size);

  size_t *const selected_positions = generate_rand_no_rep_array(size);
  bool *const is_selected_value = calloc(size, sizeof(bool));
  assert(selected_positions);
  assert(is_selected_value);

  // mark the n_cross first indexe of selected_positions in is_selected
  for (size_t i = 0; i < n_cross; i++) {
    const size_t position = selected_positions[i];
    is_selected_value[p1_offspring[position]] = true;
  }

  // sort the selected positions.
  ascending_sort(selected_positions, n_cross);

  size_t ordered_count = 0;
  for (size_t i = 0; i < size; i++) {
    if (is_selected_value[p2[i]]) {
      size_t j = selected_positions[ordered_count++];
      delta += cost_if_swap_delta(cost_mat, p1_offspring, size, i, j);
      swap(p1_offspring, i, j);
      if (ordered_count == n_cross) {
        break;
      }
    }
  }

  ////////////////////////
  // end ob_crossover
  /////////////////////////
#ifndef NDEBUG
  assert(delta == (t_delta_cost)computeCost(cost_mat, p1_offspring, size) -
                      (t_delta_cost)computeCost(
                          cost_mat, assert_p1_offspring_before, size));

  free(assert_p1_offspring_before);
#endif

  free(is_selected_value);
  free(selected_positions);
  return delta;
}

void crossover(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const crossover_2d,
    t_cost *const crossover_cost_2d, const size_t n_crossover,
    const float cross_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn) {

  assert(n_population > 0);
  assert(n_crossover == 0 || n_population > 1);
  assert(!is_array_overlap(pop_2d, ARRAY_BYTES(pop_2d, size * n_population),
                           crossover_2d,
                           ARRAY_BYTES(crossover_2d, size * n_crossover)));

  for (size_t cross_id = 0; cross_id < n_crossover; cross_id++) {
    t_delta_cost p1_cost = 0;
    size_t replicate_count = 0; //
    ///////////////////////////////////////////////////////////
    // search for a non existing crossover solution until found
    ///////////////////////////////////////////////////////////
    do {
      // select two different random parents
      size_t p1_index;
      size_t p2_index;
      do {
        p1_index = randInt(0, n_population - 1);
        p2_index = randInt(0, n_population - 1);
      } while (p1_index == p2_index);

      // retrieve parents and their cost
      const size_t *const p1_1d = &pop_2d[p1_index * size];
      const size_t *const p2_1d = &pop_2d[p2_index * size];
      p1_cost = pop_cost_1d[p1_index];
      assert(p1_cost == computeCost(cost_mat, p1_1d, size));
      assert(!is_array_overlap(crossover_2d,
                               ARRAY_BYTES(crossover_2d, size * n_crossover),
                               p1_1d, ARRAY_BYTES(p1_1d, size)));
      assert(!is_array_overlap(crossover_2d,
                               ARRAY_BYTES(crossover_2d, size * n_crossover),
                               p2_1d, ARRAY_BYTES(p2_1d, size)));

      ///////////////////////////////////////////////
      /// Do crossover and local search.
      /// The increasing rand_swap allow to avoid the generation of the same
      /// solution over and over again.
      /////////////////////////////////////////////
      memcpy(&crossover_2d[cross_id * size], p1_1d, size * sizeof(*p1_1d));

      // p1_cost +=
      //     dpx_crossover(cost_mat, &crossover_2d[cross_id * size], p2_1d,
      //     size);
      p1_cost += ob_crossover(cost_mat, &crossover_2d[cross_id * size], p2_1d,
                              size, cross_rate);
      p1_cost += rand_swaps_with_delta(cost_mat, &crossover_2d[cross_id * size],
                                       size, (float)replicate_count++ / size);
      p1_cost += vnd_lop(cost_mat, size, &crossover_2d[cross_id * size],
                         pivot_rule, fptr_delta_neigh_explaration, n_neighb_vn);

      DPRINTF("Crossover %zu with parents %zu and %zu has cost %ld\n", cross_id,
              p1_index, p2_index, p1_cost);
    } while (is_array_in_arrays(&crossover_2d[cross_id * size], pop_2d, size,
                                n_population) ||
             is_array_in_arrays(&crossover_2d[cross_id * size], crossover_2d,
                                size, cross_id));
    crossover_cost_2d[cross_id] = p1_cost;

    assert(crossover_cost_2d[cross_id] ==
           computeCost(cost_mat, &crossover_2d[cross_id * size], size));
  }
}

void mutation(
    const t_cost *restrict const cost_mat, const size_t size,
    const size_t *const pop_2d, const t_cost *const pop_cost_1d,
    const size_t n_population, size_t *const mutation_2d,
    t_cost *const mutation_cost_2d, const size_t n_mutation,
    const float mutation_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn) {

  for (size_t mut_id = 0; mut_id < n_mutation; mut_id++) {
    t_delta_cost p1_cost = 0;
    // make a random mutation on a random parent until a non existing solution
    // is found. replicate_count allow to accelerate the generation of different
    // solution by starting with a hiher mutation rate if the same solution is
    // generated multiple times.
    size_t replicate_count = 0;
    do {
      const size_t rand_index = randInt(0, n_population - 1);
      // copy parent
      const size_t *const p1 = &pop_2d[rand_index * size];
      memcpy(&mutation_2d[mut_id * size], p1, size * sizeof(size_t));
      p1_cost = pop_cost_1d[rand_index];
      // apply mutation and local search
      p1_cost +=
          rand_swaps_with_delta(cost_mat, &mutation_2d[mut_id * size], size,
                                mutation_rate + 0.01 * replicate_count++);
      p1_cost += vnd_lop(cost_mat, size, &mutation_2d[mut_id * size],
                         pivot_rule, fptr_delta_neigh_explaration, n_neighb_vn);
      DPRINTF("Mutation %zu with parent %zu has cost %ld\n", mut_id, rand_index,
              p1_cost);
    } while (is_array_in_arrays(&mutation_2d[mut_id * size], pop_2d, size,
                                n_population) ||
             is_array_in_arrays(&mutation_2d[mut_id * size], mutation_2d, size,
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
    const size_t n_offspring, const float cross_rate_mut,
    const float cross_rate, const enum pivot_enum pivot_rule,
    const t_fptr_delta_neigh_exploration *const fptr_delta_neigh_explaration,
    const ushort n_neighb_vn, float mutation_rate) {

  assert(cross_rate_mut >= 0 && cross_rate_mut <= 1);
  assert(!is_array_overlap(pop_2d, ARRAY_BYTES(pop_2d, size * n_population),
                           offspring_2d,
                           ARRAY_BYTES(offspring_2d, size * n_offspring)));
#ifndef NDEBUG
  for (size_t i = 0; i < n_population; i++) {
    assert(!is_array_in_arrays(&pop_2d[i * size], pop_2d, size, i));
  }
#endif

  // calculating number of crossover and muatation offsprnig.
  const size_t n_crossover = (size_t)(cross_rate_mut * n_offspring);
  const size_t n_mutation = n_offspring - n_crossover;
  assert(n_crossover + n_mutation == n_offspring);

  // attribute pointers to the offspring array between crossover and mutaation.
  size_t *restrict const crossover_2d = &offspring_2d[0];
  size_t *restrict const mutation_2d = &offspring_2d[n_crossover * size];
  assert(!is_array_overlap(
      crossover_2d, ARRAY_BYTES(crossover_2d, size * n_crossover), mutation_2d,
      ARRAY_BYTES(mutation_2d, size * n_mutation)));

  // attribute pointers to the offspring cost array between crossover cost and
  // mutation cost.
  t_cost *restrict const crossover_cost_2d = &offspring_cost_2d[0];
  t_cost *restrict const mutation_cost_2d = &offspring_cost_2d[n_crossover];
  assert(!is_array_overlap(
      crossover_cost_2d, ARRAY_BYTES(crossover_cost_2d, n_crossover),
      mutation_cost_2d, ARRAY_BYTES(mutation_cost_2d, n_mutation)));

  // populate the first n_crossover elements of offspring with crossover.
  crossover(cost_mat, size, pop_2d, pop_cost_1d, n_population, crossover_2d,
            crossover_cost_2d, n_crossover, cross_rate, pivot_rule,
            fptr_delta_neigh_explaration, n_neighb_vn);

  // populate the rest of offspring with mutation.
  mutation(cost_mat, size, pop_2d, pop_cost_1d, n_population, mutation_2d,
           mutation_cost_2d, n_mutation, mutation_rate, pivot_rule,
           fptr_delta_neigh_explaration, n_neighb_vn);

#ifndef NDEBUG
  for (size_t i = 0; i < n_offspring; i++) {
    assert(offspring_cost_2d[i] ==
           computeCost(cost_mat, &offspring_2d[i * size], size));
    assert(!is_array_in_arrays(&offspring_2d[i * size], pop_2d, size,
                               n_population) &&
           !is_array_in_arrays(&offspring_2d[i * size], offspring_2d, size, i));
  }
#endif
}

void select_n_best(size_t *restrict const best_2d,
                   t_cost *restrict const best_cost_1d,
                   const size_t *restrict const all_2d,
                   const t_cost *restrict const all_cost_1d,
                   const size_t n_best, const size_t n_all,
                   const size_t size_elem) {
  assert(n_best < n_all);

  // get the n best cost indices.
  const size_t *const best_pop_id =
      get_n_best_sorted_cost(all_cost_1d, n_best, n_all);

  size_t *restrict const new_pop_2d =
      malloc(size_elem * n_best * sizeof(size_t));
  t_cost *restrict const new_pop_cost_1d = malloc(n_best * sizeof(t_cost));
// populate pop_2d with the sortede best.
#pragma omp simd
  for (size_t i = 0; i < n_best; i++) {
    memcpy(&new_pop_2d[i * size_elem], &all_2d[best_pop_id[i] * size_elem],
           size_elem * sizeof(*new_pop_2d));
    new_pop_cost_1d[i] = all_cost_1d[best_pop_id[i]];
  }
  memcpy(best_2d, new_pop_2d, size_elem * n_best * sizeof(size_t));
  memcpy(best_cost_1d, new_pop_cost_1d, n_best * sizeof(t_cost));

  assert(best_cost_1d[0] >= get_max_array_cost(all_cost_1d, n_all));

  free((size_t *const)best_pop_id);
  free(new_pop_2d);
  free(new_pop_cost_1d);
  DPRINTF("Selected best %zu from %zu population with best cost %u\n", n_best,
          n_all, best_cost_1d[0]);
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

t_delta_cost
memetic(const t_cost *const cost_mat, size_t *const sol_1d, size_t size,
        const size_t n_population, const size_t n_diversi_try,
        const size_t n_mean_try, const float cross_rate_mut,
        const size_t n_offspring, const float mutation_rate,
        const float cross_rate, const enum pivot_enum pivot_rule,
        const t_fptr_delta_neigh_exploration *fptr_delta_neigh_explaration,
        const ushort n_neighb_vn) {

  assert(cost_mat);
  assert(sol_1d);
  assert(size > 1);
  assert(n_population > 0);
  assert(n_offspring > 0);
  assert(cross_rate_mut >= 0 && cross_rate_mut <= 1);
  assert(mutation_rate >= 0 && mutation_rate <= 1);
  assert(cross_rate >= 0 && cross_rate <= 1);

  //////////////////////////////////
  /// assing overlaping memory
  //////////////////////////////////
  size_t *const pop_off_2d =
      calloc(size * (n_population + n_offspring), sizeof(*pop_off_2d));
  t_cost *const pop_off_cost_2d =
      calloc((n_population + n_offspring), sizeof(*pop_off_cost_2d));
  assert(pop_off_2d);
  assert(pop_off_cost_2d);

  size_t *const pop_2d = &pop_off_2d[0];
  t_cost *const pop_cost_1d = &pop_off_cost_2d[0];

  size_t *const offspring_2d = &pop_off_2d[size * n_population];
  t_cost *const offspring_cost_2d = &pop_off_cost_2d[n_population];

  assert(is_array_overlap(
      pop_off_2d, ARRAY_BYTES(pop_off_2d, size * (n_population + n_offspring)),
      pop_2d, ARRAY_BYTES(pop_2d, size * n_population)));
  assert(is_array_overlap(
      pop_off_cost_2d, ARRAY_BYTES(pop_off_cost_2d, n_population + n_offspring),
      pop_cost_1d, ARRAY_BYTES(pop_cost_1d, n_population)));
  assert(is_array_overlap(
      pop_off_2d, ARRAY_BYTES(pop_off_2d, size * (n_population + n_offspring)),
      offspring_2d, ARRAY_BYTES(offspring_2d, size * n_offspring)));
  assert(is_array_overlap(
      pop_off_cost_2d, ARRAY_BYTES(pop_off_cost_2d, n_population + n_offspring),
      offspring_cost_2d, ARRAY_BYTES(offspring_cost_2d, n_offspring)));

  ///////////////////////////////
  /// populating initial population
  /// keep sol_1d at 1st place of population
  ///////////////////////////////

  PVERB("Populating initial population\n");
  memcpy(&pop_2d[0], sol_1d, size);
  pop_cost_1d[0] = computeCost(cost_mat, sol_1d, size);
  populate(cost_mat, pop_2d, pop_cost_1d, size, n_population, 1, pivot_rule,
           n_neighb_vn, fptr_delta_neigh_explaration);

  /////////////////////////////////////
  /// generations of individuals
  ////////////////////////////////////
  t_cost best_cost = pop_cost_1d[0];
  float mean_cost_pop = get_mean(pop_cost_1d, n_population);
  float best_mean_pop_cost = mean_cost_pop;
  size_t mean_try = 0;
  size_t diversi_try = 0;
  size_t gen = 0;
  set_result_clock();
  while (diversi_try < n_diversi_try && !is_interrupt_requested()) {

    PVERB("Generating offspring\n");
    offspring(cost_mat, size, pop_2d, pop_cost_1d, n_population, offspring_2d,
              offspring_cost_2d, n_offspring, cross_rate_mut, cross_rate,
              pivot_rule, fptr_delta_neigh_explaration, n_neighb_vn,
              mutation_rate);

    PVERB("Selecting %zu best\n", n_population);
    // the best sols will be in descending order.
    select_n_best(pop_2d, pop_cost_1d, pop_off_2d, pop_off_cost_2d,
                  n_population, n_offspring + n_population, size);

    if (best_cost < pop_cost_1d[0]) { // will never be inferior.
      best_cost = pop_cost_1d[0];
      result_printer(best_cost, &pop_2d[0], size, false);
      diversi_try = 0;
      mean_try = 0;
    }

    ///////////////////////////
    /// is diversification needed?
    /////////////////////////////
    const float new_mean = get_mean(pop_cost_1d, n_population);
    const bool is_better = (mean_cost_pop < new_mean);
    mean_cost_pop = new_mean;

    if (is_better) {
      mean_try = 0;
      if (best_mean_pop_cost < mean_cost_pop) { // the mean improved
        best_mean_pop_cost = mean_cost_pop;
        diversi_try = 0;
      }
    } else if (mean_try++ >= n_mean_try) {
      /////////////////////////////////////////
      /// diversify
      ////////////////////////////////////////
      mean_try = 0;
      diversi_try++;
      PVERB("Diversifying population\n");
      diversification(cost_mat, size, pop_2d, pop_cost_1d, n_population, 1,
                      pivot_rule, fptr_delta_neigh_explaration, n_neighb_vn);
    }

    PVERB("gen=%zu | mean_pop_cost=%f | best_mean_pop_cost=%f | "
          "best_cost=%u | mean_try=%zu/%zu | "
          "diversity_try=%zu/%zu\n",
          gen, mean_cost_pop, best_mean_pop_cost, best_cost, mean_try,
          n_mean_try, diversi_try, n_diversi_try);
    gen++;
  }
  ////////////////////////////////////////////////////////////////////
  /// end of generations, return the first element of the population
  //////////////////////////////////////////////////////////////////////
  memcpy(sol_1d, &pop_2d[0], size * sizeof(size_t));
  assert(best_cost == get_max_array_cost(pop_cost_1d, n_population));
  assert(best_cost == computeCost(cost_mat, sol_1d, size));

  free(pop_off_2d);
  free(pop_off_cost_2d);
  return best_cost;
}
