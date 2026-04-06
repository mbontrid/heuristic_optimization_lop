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

#include <argp.h>
// #include <limits>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"
#include "instance.h"
#include "optimization.h"
#include "timer.h"
#include "utilities.h"

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Returns verbose output"},
    {"output", 'o', "FILE", 0,
     "Returns output to file instead of standard input"},
    {"instance", 'i', "FILE", 0, "Instance file to use"},
    {"pivot", 'p', "CHOICE", 0, "Pivoting rule: first|best"},
    {"neighborhood", 'n', "CHOICE", 0,
     "Neighborhood strategy: transpose|exchange|insert"},
    {"sol_start", 's', "CHOICE", 0, "Initial solution: random|c_and_w"},
    {0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key) {
  case 'v':
    arguments->verbose = true;
    break;
  case 'o':
    arguments->out_file = arg;
    break;
  case 'i':
    arguments->instance_file = arg;
    break;
  case 'p':
    if (strcmp(arg, "first") == 0) {
      arguments->pivoting_rule = first;
      arguments->fptr_pivoting_rule = pivot_first;
      DEBUG_PRINT("pivoting rule set to first");
    } else if (strcmp(arg, "best") == 0) {
      arguments->pivoting_rule = best;
      arguments->fptr_pivoting_rule = pivot_best;
      DEBUG_PRINT("pivoting rule set to best");
    } else {
      argp_error(state, "Invalid pivoting_rule option: %s", arg);
      DEBUG_PRINT("pivoting choice no found");
    }
    break;
  case 'n':
    if (strcmp(arg, "transpose") == 0) {
      arguments->neighborhood = transpose;
      arguments->fptr_neighborhood = neighborhood_tranpose;
      DEBUG_PRINT("neighborhood set to transpose");
    } else if (strcmp(arg, "exchange") == 0) {
      arguments->neighborhood = exchange;
      arguments->fptr_neighborhood = neighborhood_exchange;
      DEBUG_PRINT("neighborhood set to exchange");
    } else if (strcmp(arg, "insert") == 0) {
      arguments->neighborhood = insert;
      arguments->fptr_neighborhood = neighborhood_insert;
      DEBUG_PRINT("neighborhood set to insert");
    } else {
      argp_error(state, "Invalid neighborhood option: %s", arg);
      DEBUG_PRINT("neighborhood choice not found");
    }
    break;
  case 's':
    if (strcmp(arg, "random") == 0) {
      arguments->sol_start = randome;
      arguments->fptr_sol_start = sol_start_random;
      DEBUG_PRINT("solution start set to random");
    } else if (strcmp(arg, "c_and_w") == 0) {
      arguments->sol_start = c_and_w;
      arguments->fptr_sol_start = sol_start_c_and_w;
      DEBUG_PRINT("solution start set to c_and_w");
    } else {
      argp_error(state, "Invalid initial solution option: %s", arg);
      DEBUG_PRINT("solution start choice not found");
    }
  case ARGP_KEY_ARG:
    if (arguments->is_pos_arg &&
        state->arg_num >=
            sizeof(arguments->pos_args) / sizeof(arguments->pos_args[0])) {
      argp_usage(state);
    }
    arguments->pos_args[state->arg_num] = arg;
    break;
  // case of fewer arguments where given than required.
  case ARGP_KEY_END:
    if (arguments->is_pos_arg &&
        state->arg_num <
            sizeof(arguments->pos_args) / sizeof(arguments->pos_args[0])) {
      printf("No enought positional arguments.");
      argp_usage(state);
    }
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {

  DEBUG_PRINT("Debug print activated.\n");

  long int i, j;
  long int *currentSolution;
  int cost, newCost, temp, firstRandomPosition, secondRandomPosition;

  /* Do not buffer output */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  // ---argument parsing----

  struct arguments arguments;
  arguments.verbose = false;
  arguments.instance_file = "instances/N-be75eec_150";
  arguments.out_file = "data/last_results";
  arguments.pivoting_rule = first;
  arguments.neighborhood = transpose;
  arguments.sol_start = randome;
  arguments.fptr_pivoting_rule = pivot_first;
  arguments.fptr_neighborhood = neighborhood_tranpose;
  arguments.fptr_sol_start = sol_start_random;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  printf("verbose: %s\ninstance file: %s\noutput file: %s\npos arguments: "
         "%s\npivoting rule: %u\nneighborhood: %u\ninitial solution: %u\n",
         arguments.verbose ? "yes" : "no", arguments.instance_file,
         arguments.out_file, arguments.pos_args[0], arguments.pivoting_rule,
         arguments.neighborhood, arguments.sol_start);

  //--- end args passing ----

  /* Read instance file */
  CostMat = readInstance(arguments.instance_file);
  printf("Data have been read from instance file. Size of instance = %ld.\n\n",
         PSize);

  /* initialize random number generator, deterministically based on instance.
   * To do this we simply set the seed to the sum of elements in the matrix,
   so it is constant per-instance, but (most likely) varies between instances
 */
  Seed = (long int)0;
  for (i = 0; i < PSize; ++i)
    for (j = 0; j < PSize; ++j)
      Seed += (long int)CostMat[i][j];
  printf("Seed used to initialize RNG: %ld.\n\n", Seed);

  DPRINTF("debug of neighborhood and sol_start\n");
  arguments.fptr_neighborhood(0, 0, CostMat);
  arguments.fptr_sol_start(CostMat, PSize);

  /* starts time measurement */
  start_timers();

  /* A solution is just a vector of int with the same size as the instance */
  currentSolution = (long int *)malloc(PSize * sizeof(long int));

  /* Create an initial random solution.
     The only constraint is that it should always be a permutation */
  createRandomSolution(currentSolution);

  /* Print solution */
  printf("Initial solution:\n");
  for (j = 0; j < PSize; j++)
    printf(" %ld", currentSolution[j]);
  printf("\n");

  /* Compute cost of solution and print it */
  cost = computeCost(currentSolution);
  printf("Cost of this initial solution: %d\n\n", cost);

  /* Example: apply an exchange operation of two elements at random position
   */
  firstRandomPosition = randInt(0, PSize - 1);
  // Ensure second position is different from first one:
  secondRandomPosition = firstRandomPosition + randInt(1, (PSize - 2));
  if (secondRandomPosition >= PSize)
    secondRandomPosition -= PSize;

  printf("Two positions exchanged: %d and %d. ", firstRandomPosition,
         secondRandomPosition);

  temp = currentSolution[firstRandomPosition];
  currentSolution[firstRandomPosition] = currentSolution[secondRandomPosition];
  currentSolution[secondRandomPosition] = temp;

  printf("Solution after exchange:\n");
  for (j = 0; j < PSize; j++)
    printf(" %ld", currentSolution[j]);
  printf("\n");

  /* Recompute cost of solution after the exchange move */
  /* There are some more efficient way to do this, instead of recomputing
   * everything... */
  newCost = computeCost(currentSolution);
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
