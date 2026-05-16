
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "memetic.h"
#include "optimization.h"
#include "utilities.h"

size_t *memetic(const t_cost *const cost_mat, size_t **population,
                size_t size_pop, size_t size) {

  size_t n_offspring = size / 2;
  size_t *offspring_2d = malloc(size * sizeof(size_t) * n_offspring);

  for (size_t i = 0; i < n_offspring; i++) {
    size_t *p1 = population[randInt(0, size_pop - 1)];
    size_t *p2 = population[randInt(0, size_pop - 1)];
    size_t *p1_offspring = malloc(size * sizeof(size_t));
    memcpy(p1_offspring, p1, size * sizeof(size_t));
    t_cost_delta delta_spring = dpx_crossover(cost_mat, p1_offspring, p2, size);
  }
}

t_cost_delta dpx_crossover(const t_cost *const cost_mat, size_t *p1_offspring,
                           size_t *p2, size_t size) {
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
      indexes[to_move] = i;
      to_move++;
    }
  }

  t_cost_delta delta = 0;

  while (to_move > 2) {
    size_t mov1_index = randInt(0, to_move - 1);
    size_t mov1 = indexes[mov1_index];
    indexes[mov1_index] = indexes[--to_move];
    size_t mov2_index = randInt(0, to_move - 1);
    size_t mov2 = indexes[mov2_index];
    indexes[mov2_index] = indexes[--to_move];

    delta += cost_swap_delta(cost_mat, p1_offspring, size, mov1, mov2);
    swap(p1_offspring, mov1, mov2);
  }

  free(indexes);

  assert(assert_cost_before + delta ==
         computeCost(cost_mat, p1_offspring, size));
  return delta;
}

t_cost_delta mutate(const t_cost *const cost_mat, size_t *const array,
                    const size_t size, const float rate) {
  assert(rate <= 1 || rate >= 0);
  assert(array);

  t_cost_delta delta = 0;

  for (size_t i = 0; i < size; i++) {
    if ((float)ran01(&Seed) < rate) {
      size_t j = randInt(0, size - 1);
      delta += cost_swap_delta(cost_mat, array, size, i, j);
      swap(array, i, j);
    }
  }
  return delta;
}
