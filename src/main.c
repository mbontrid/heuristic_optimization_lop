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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instance.h"
#include "optimization.h"
#include "timer.h"
#include "utilities.h"
#include "utils.h"

static const char doc[] = "LOP instance resolver";
static const char args_doc[] = "";

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Returns verbose output"},
    {"output", 'o', "FILE", 0,
     "Returns output to file instead of standard input"},
    {"instance", 'i', "FILE", 0, "Instance file to use"},
    {0},
};

struct arguments {
  char *pos_args[1]; // positional argumuments of the command line calla.
  char *FileName;
  char *out_file;
  bool verbose;
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
    arguments->FileName = arg;
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >=
        sizeof(arguments->pos_args) / sizeof(arguments->pos_args[0])) {
      argp_usage(state);
    }
    arguments->pos_args[state->arg_num] = arg;
    break;
  // case of fewer arguments where given than required.
  case ARGP_KEY_END:
    if (state->arg_num <
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

  DEBUG_PRINT("Debug print compiled.\n");

  long int i, j;
  long int *currentSolution;
  int cost, newCost, temp, firstRandomPosition, secondRandomPosition;

  /* Do not buffer output */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  struct arguments arguments;
  arguments.verbose = false;
  arguments.FileName = "instances/N-be75eec_150";
  arguments.out_file = "data/last_results";

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  printf("verbose: %s\ninstance file: %s\noutput file: %s\npos arguments: %s\n",
         arguments.verbose ? "yes" : "no", arguments.FileName,
         arguments.out_file, arguments.pos_args[0]);

  if (argc < 2) {
    printf("No instance file provided (use -i <instance_name>). Exiting.\n");
    exit(1);
  }

  DEBUG_PRINT("-----fin argp-----");

  /* Read parameters */
  // readOpts(argc, argv);

  /* Read instance file */
  CostMat = readInstance(arguments.FileName);
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
