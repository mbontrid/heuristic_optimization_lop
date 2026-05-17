#pragma once

#include "utilities.h"
#include <sys/types.h>

t_cost_delta
memetic(const t_cost *const cost_mat, size_t *const sol_1d, size_t size,
        const size_t n_population, const float mutation_rate,
        const float cross_rate, enum pivot_enum pivot_rule,
        t_fptr_delta_neigh_exploration *fptr_delta_neigh_explaration,
        ushort n_neighb_vn);

t_cost_delta dpx_crossover(const t_cost *const cost_mat, size_t *p1_offspring,
                           size_t *p2, size_t size);

t_cost_delta ob_crossover(const t_cost *const cost_mat,
                          size_t *const p1_offspring, const size_t *const p2,
                          const size_t size, const float cross_rate);
