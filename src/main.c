/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arg_parser.h"
#include "instance.h"
#include "optimization.h"
#include "optimization2.h"
#include "utilities.h"

int main(int argc, char **argv) {

  DPRINTF("------WARNING-------\n If you see this message, it means that you "
          "are running the debug version of this program. Performances will be "
          "greatly impacted.\n");

  // ---argument parsing----

  init_arguments(&arguments);
  parse_arguments(argc, argv, &arguments);

  PVERB("Verbose activated\n");

  //--- end args passing ----

  /* Read instance file */
  size_t mat_cost_dim = 0;
  const t_mat_cell *const cost_mat_2d = (const t_mat_cell *const)readInstance(
      arguments.instance_file, &mat_cost_dim);
  PVERB("Data have been read from instance file. Size of instance = %u.\n\n",
        mat_cost_dim);

  /* initialize random number generator, deterministically based on instance.
   * To do this we simply set the seed to the sum of elements in the matrix,
   so it is constant per-instance, but (most likely) varies between instances
 */
  Seed = (long int)0;
  for (size_t i = 0; i < mat_cost_dim; ++i)
    for (size_t j = 0; j < mat_cost_dim; ++j)
      Seed += (long int)cost_mat_2d[mat_cost_dim * i + j];
  PVERB("Seed used to initialize RNG: %ld.\n", Seed);

  // ----------------------------------------------------------
  // lop measurement
  // ------------------------------------------------------------

  // generating first solution
  size_t *sol_1d = arguments.fptr_sol_start(cost_mat_2d, mat_cost_dim);

  t_cost_delta cost = computeCost(cost_mat_2d, sol_1d, mat_cost_dim);

  /* starts time measurement */
  clock_t start = clock();

  if (arguments.algo == VND) {

    PVERB("Running variable neighborhood descent algorithm with pivot rule %s "
          "and %zu neighborhood(s).\n",
          arguments.pivot_rule == FIRST ? "first" : "best",
          arguments.n_neighb_vnd);

    cost += vnd_lop(cost_mat_2d, mat_cost_dim, sol_1d, arguments.pivot_rule,
                    arguments.fptr_neighb_exploration, arguments.n_neighb_vnd);

  } else if (arguments.algo == ILS) {

    PVERB("Running iterated local search algorithm\n");

    cost += ils(cost_mat_2d, sol_1d, mat_cost_dim, arguments.ils_perturb_rate,
                arguments.ils_n_try, arguments.ils_worse, arguments.pivot_rule,
                arguments.fptr_neighb_exploration, arguments.n_neighb_vnd);

  } else if (arguments.algo == MEMETIC) {

    PVERB("Running memetic algorithm\n");
    cost = memetic(
        cost_mat_2d, sol_1d, mat_cost_dim, arguments.memetic_n_population,
        arguments.memetic_n_diversi_try, arguments.memetic_n_mean_try,
        arguments.memetic_offspring_cross_mut, arguments.memetic_n_offspring,
        arguments.memetic_mutation_rate, arguments.memetic_cross_rate,
        arguments.pivot_rule, arguments.fptr_neighb_exploration,
        arguments.n_neighb_vnd);
  }

  const double elapsed_seconds = end_clock(start);
  /* stop time measurement */

  assert(cost == computeCost(cost_mat_2d, sol_1d, mat_cost_dim));

  //////////////////////////////////////////////////////////////////////////////
  // print results
  //////////////////////////////////////////////////////////////////////////////
  printf("RESULT cost=%ld time=%g solution=", cost, elapsed_seconds);
  print_array_1d((long int *)sol_1d, mat_cost_dim);

  free(sol_1d);
  free((size_t *const)cost_mat_2d);
  return 0;
}
