#pragma once

#include "utilities.h"

typedef t_cost_delta (*t_fptr_memetic_local_search)(t_mat_cell *cost_mat_2d,
                                                    size_t *const sol_1d,
                                                    size_t size);

size_t *memetic(const t_cost *const cost_mat, size_t **population,
                size_t size_pop, size_t size,
                const t_fptr_memetic_local_search fptr_local_search);

t_cost_delta dpx_crossover(const t_cost *const cost_mat, size_t *p1_offspring,
                           size_t *p2, size_t size);

t_cost_delta ob_crossover(const t_cost *const cost_mat,
                          size_t *const p1_offspring, const size_t *const p2,
                          const size_t size, const float cross_rate);
