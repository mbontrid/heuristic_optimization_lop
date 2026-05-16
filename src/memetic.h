#pragma once

#include "utilities.h"

size_t *memetic(const t_cost *const cost_mat, size_t **population,
                size_t size_pop, size_t size);

t_cost_delta dpx_crossover(const t_cost *const cost_mat, size_t *p1_offspring,
                           size_t *p2, size_t size);
