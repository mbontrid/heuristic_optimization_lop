
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "memetic.h"
#include "optimization.h"
#include "utilities.h"

size_t *memetic(const t_cost *const cost_mat, size_t **population,
                size_t size_pop, size_t size,
                const t_fptr_memetic_local_search fptr_local_search) {

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

t_cost_delta ob_crossover(const t_cost *const cost_mat,
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

  size_t *const to_cross = generate_random_vector(size);
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
  free(to_cross);

  assert(delta == 0 ||
         !array_equal(p1_offspring, assert_p1_offspring_before, size));
  assert((t_cost_delta)computeCost(cost_mat, p1_offspring, size) -
             (t_cost_delta)computeCost(assert_p1_offspring_before) ==
         delta);
#ifndef NDEBUG
  free(assert_p1_offspring_before);
#endif
  return delta;
}
