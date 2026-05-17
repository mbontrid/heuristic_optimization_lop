#pragma once

#include "utilities.h"
#include <sys/types.h>

t_cost_delta ils(const t_cost *const cost_mat, size_t *const sol_1d,
                 size_t size, const float perturb_rate, const size_t n_try,
                 const t_cost worse, enum pivot_enum pivot_rule,
                 t_fptr_delta_neigh_exploration *fptr_delta_neigh_exploration,
                 ushort n_neighb_vn);

bool accept_worse(const t_cost_delta delta, const t_cost worse_bracket);
