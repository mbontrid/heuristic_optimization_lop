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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"
#include "instance.h"
#include "optimization.h"
#include "timer.h"
#include "utilities.h"

int main(int argc, char **argv) {

  DPRINTF("Debug print activated.\n");

  /* Do not buffer output */
  // setbuf(stdout, NULL);
  // setbuf(stderr, NULL);

  // ---argument parsing----

  init_arguments(&arguments);
  parse_arguments(argc, argv, &arguments);

  PVERB("Verbose activated\n");

  //--- end args passing ----

  /* Read instance file */
  t_sizemat mat_cost_dim = 0;
  t_mat_cell *const cost_mat_2d =
      readInstance(arguments.instance_file, &mat_cost_dim);
  PVERB("Data have been read from instance file. Size of instance = %u.\n\n",
        mat_cost_dim);

  /* initialize random number generator, deterministically based on instance.
   * To do this we simply set the seed to the sum of elements in the matrix,
   so it is constant per-instance, but (most likely) varies between instances
 */
  Seed = (long int)0;
  for (t_sizemat i = 0; i < mat_cost_dim; ++i)
    for (t_sizemat j = 0; j < mat_cost_dim; ++j)
      Seed += (long int)cost_mat_2d[mat_cost_dim * i + j];
  PVERB("Seed used to initialize RNG: %ld.\n\n", Seed);

  // lop measurement
  // ------------------------------------------------------------

  // generating first solution
  t_sizemat *sol_1d = arguments.fptr_sol_start(cost_mat_2d, mat_cost_dim);

  /* starts time measurement */
  start_timers();
  t_cost cost =
      vnd_lop(cost_mat_2d, mat_cost_dim, sol_1d, arguments.fptr_pivoting_rule,
              arguments.fptrs_neighborhood, arguments.n_neighb_vnd);
  const double elapsed_seconds = elapsed_time(VIRTUAL);
  /* stop time measurement */

  printf("RESULT cost=%u time=%g solution=\n", cost, elapsed_seconds);
  print_array_1d(sol_1d, mat_cost_dim);

  free(sol_1d);
  return 0;
}
