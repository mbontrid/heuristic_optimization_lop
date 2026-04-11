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

#include "arg_parser.h"
#include "instance.h"
#include "optimization.h"
#include "timer.h"
#include "utilities.h"

int main(int argc, char **argv) {

  DEBUG_PRINT("Debug print activated.\n");

  long int i, j;
  t_sizemat *currentSolution;
  int cost, newCost, temp, firstRandomPosition, secondRandomPosition;

  /* Do not buffer output */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  // ---argument parsing----

  init_arguments(&arguments);
  parse_arguments(argc, argv, &arguments);

  PVERB("Verbose activated\n");

  //--- end args passing ----

  /* Read instance file */
  t_sizemat cost_mat_size = 0;
  t_mat_cell *const cost_mat_2d =
      readInstance(arguments.instance_file, &cost_mat_size);
  printf("Data have been read from instance file. Size of instance = %u.\n\n",
         cost_mat_size);

  /* initialize random number generator, deterministically based on instance.
   * To do this we simply set the seed to the sum of elements in the matrix,
   so it is constant per-instance, but (most likely) varies between instances
 */
  Seed = (long int)0;
  for (i = 0; i < cost_mat_size; ++i)
    for (j = 0; j < cost_mat_size; ++j)
      Seed += (long int)cost_mat_2d[cost_mat_size * i + j];
  printf("Seed used to initialize RNG: %ld.\n\n", Seed);

  DPRINTF("debug of neighborhood and sol_start\n");

  t_sizemat *sol =
      lop(cost_mat_2d, cost_mat_size, arguments.fptr_sol_start,
          arguments.fptr_pivoting_rule, arguments.fptr_neighborhood);
  cost = computeCost(cost_mat_2d, sol, cost_mat_size);

  printf("lop cost found : %u with sol : \n", cost);
  print_array_1d(sol, cost_mat_size);

  free(sol);

  /* starts time measurement */
  start_timers();

  currentSolution = generate_random_vector(cost_mat_size);

  /* Print solution */
  printf("Initial solution:\n");
  for (j = 0; j < cost_mat_size; j++)
    printf(" %u", currentSolution[j]);
  printf("\n");

  /* Compute cost of solution and print it */
  cost = computeCost(cost_mat_2d, currentSolution, cost_mat_size);
  printf("Cost of this initial solution: %d\n\n", cost);

  /* Example: apply an exchange operation of two elements at random position
   */
  firstRandomPosition = randInt(0, cost_mat_size - 1);
  // Ensure second position is different from first one:
  secondRandomPosition = firstRandomPosition + randInt(1, (cost_mat_size - 2));
  if (secondRandomPosition >= cost_mat_size)
    secondRandomPosition -= cost_mat_size;

  printf("Two positions exchanged: %d and %d. ", firstRandomPosition,
         secondRandomPosition);

  temp = currentSolution[firstRandomPosition];
  currentSolution[firstRandomPosition] = currentSolution[secondRandomPosition];
  currentSolution[secondRandomPosition] = temp;

  printf("Solution after exchange:\n");
  for (j = 0; j < cost_mat_size; j++)
    printf(" %u", currentSolution[j]);
  printf("\n");

  /* Recompute cost of solution after the exchange move */
  /* There are some more efficient way to do this, instead of recomputing
   * everything... */
  newCost = computeCost(cost_mat_2d, currentSolution, cost_mat_size);
  printf("Cost of this solution after applying the exchange move: %d\n",
         newCost);

  if (newCost == cost)
    printf("Second solution is as good as first one\n");
  else if (newCost > cost)
    printf("Second solution is better than first one\n");
  else
    printf("Second solution is worse than first one\n");

  printf("Time elapsed since we started the timer: %g\n\n",
         elapsed_time(VIRTUAL));

  return 0;
}
