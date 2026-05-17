
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"
#include "iterated_local_search.h"
#include "optimization.h"
#include "utilities.h"

t_cost_delta ils(const t_cost *const cost_mat, size_t *const sol_1d,
                 size_t size, const float perturb_rate, const size_t n_try,
                 const t_cost worse, enum pivot_enum pivot_rule,
                 t_fptr_delta_neigh_exploration *fptr_delta_neigh_exploration,
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
    new_delta += rand_swap(cost_mat, new_sol_1d, size, perturb_rate);
    new_delta += vnd_lop(cost_mat, size, new_sol_1d, pivot_rule,
                         fptr_delta_neigh_exploration, n_neighb_vn);

    if (accept_worse(new_delta - delta, worse)) {
      PVERB("Accepting new solution with delta %ld\n", new_delta);
      memcpy(sol_1d, new_sol_1d, size * sizeof(size_t));
      delta += new_delta;
      try = n_try;
    } else {
      PVERB("Rejecting new solution with delta %ld and %zu tries left\n",
            new_delta, try);
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

bool accept_worse(const t_cost_delta delta, const t_cost worse_bracket) {
  assert(worse_bracket >= 0);
  return delta > worse_bracket;
}
