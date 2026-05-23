#include <argp.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"
#include "optimization.h"
#include "utilities.h"

struct arguments arguments;

static struct argp_option options[] = {
    {"verbose", KEY_VERBOSE, 0, 0, "Stdout verbose."},
    {"result", KEY_RESULT, 0, 0, "Stdout the result at each improvement."},
    {"instance", KEY_INSTANCE, "FILE", 0,
     "Instance file to use. Default=data/input/instances/N-be75eec_150"},
    {"pivot", KEY_PIVOT, "CHOICE", 0,
     "Pivoting rule: first|best. Default : first"},
    {"neighborhood", KEY_NEIGHB, "CHOICE", 0,
     "Neighborhood strategy: (transpose|exchange|insert). Multiple -n CHOICE "
     "can be set for a VND algorith. Default : exahange"},
    {"sol_start", KEY_SOL_START, "CHOICE", 0,
     "Initial solution: random|c_and_w"},
    {"algo", KEY_ALGO, "CHOICE", 0, "Algorithme to use. (vnd|ils|memetic)"},
    {"ils_perturb_rate", KEY_ILS_PETURB_RATE, "FLOAT", 0,
     "Perturbation rate for iterated local search. Default : 0.1"},
    {"ils_n_try", KEY_ILS_N_TRY, "SIZE_T", 0,
     "Number of tries without accepting for iterated local search.If reached, "
     "current solution is returned. Default : 10"},
    {"ils_worst", KEY_ILS_WORST, "COST", 0,
     "Worst bracket for accepting worse solution in iterated local search. "
     "Default : 0"},
    {"meme_pop", KEY_MEMETIC_N_POPULATION, "SIZE_T", 0,
     "Number of individuals in the population based memetic algorithms. "
     "Default:20"},
    {"meme_divers_try", KEY_MEMETIC_N_DIVERSI_TRY, "SIZE_T", 0,
     "Number of diversification with no improvement before terminating the "
     "memetic algorithms. Default: 5"},
    {"meme_mean_try", KEY_MEMETIC_N_MEAN_TRY, "SIZE_T", 0,
     "Number of same mean population before diversification for the memetic "
     "algorithms. "
     "Default:10"},
    {"meme_cross_rate_mut", KEY_MEMETIC_CROSS_RATE_MUT, "FLOAT", 0,
     "Offspring crossover at each generation in the memetic algorithms. The "
     "rest of the offspring will be mutated."
     "Default:0.8"},
    {"meme_offspring", KEY_MEMETIC_N_OFFSPRING, "SIZE_T", 0,
     "Number of offspring in the memetic algorithms. "
     "Default:10"},
    {"meme_mut_rate", KEY_MEMETIC_MUTATION_RATE, "FLOAT", 0,
     "Mutation rate in the memetic algorithms. Rate of random swap of a "
     "solution of LOP."
     "Default:0.1"},
    {"meme_cross_rate", KEY_MEMETIC_CROSS_RATE, "FLOAT", 0,
     "Crossover rate in the memetic algorithms."
     "Default:0.5"},
    {0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *args = state->input;

  switch (key) {
  case KEY_VERBOSE:
    args->verbose = true;
    break;
  case KEY_RESULT:
    args->result = true;
    break;
  // case 'o':
  //   args->out_file = arg;
  //   break;
  case 'i':
    args->instance_file = arg;
    break;
  case KEY_PIVOT:
    if (strcmp(arg, "first") == 0) {
      args->pivot_rule = FIRST;
      DPRINTF("pivoting rule set to first\n");
    } else if (strcmp(arg, "best") == 0) {
      args->pivot_rule = BEST;
      DPRINTF("pivoting rule set to best\n");
    } else {
      argp_error(state, "Invalid pivoting_rule option: %s", arg);
    }
    break;
  case KEY_NEIGHB:
    assert(args->n_neighb_vnd < 10);
    if (strcmp(arg, "transpose") == 0) {
      args->neighb_exploration[args->n_neighb_vnd] = TRANSPOSE;
      args->fptr_neighb_exploration[args->n_neighb_vnd++] =
          cost_delta_transpose;
    } else if (strcmp(arg, "exchange") == 0) {
      args->neighb_exploration[args->n_neighb_vnd] = EXCHANGE;
      args->fptr_neighb_exploration[args->n_neighb_vnd++] = cost_delta_exchange;
    } else if (strcmp(arg, "insert") == 0) {
      args->neighb_exploration[args->n_neighb_vnd] = INSERT;
      args->fptr_neighb_exploration[args->n_neighb_vnd++] = cost_delta_insert;
    } else {
      argp_error(state, "Invalid neighborhood option: %s", arg);
    }
    DPRINTF("neighborhood %d method added : %s\n", args->n_neighb_vnd - 1, arg);
    break;
  case KEY_SOL_START:
    if (strcmp(arg, "random") == 0) {
      args->fptr_sol_start = sol_start_random;
      DPRINTF("solution start set to random\n");
    } else if (strcmp(arg, "c_and_w") == 0) {
      args->fptr_sol_start = sol_start_cw;
      DPRINTF("solution start set to c_and_w\n");
    } else {
      argp_error(state, "Invalid initial solution option: %s", arg);
    }
    break;
  case KEY_ALGO:
    if (strcmp(arg, "vnd") == 0) {
      args->algo = VND;
      DPRINTF("algorithm set to vnd\n");
    } else if (strcmp(arg, "ils") == 0) {
      args->algo = ILS;
      DPRINTF("algorithm set to ils\n");
    } else if (strcmp(arg, "memetic") == 0) {
      args->algo = MEMETIC;
      DPRINTF("algorithm set to memetic\n");
    } else {
      argp_error(state, "Invalid algorithm option: %s", arg);
    }
    break;
  case KEY_ILS_PETURB_RATE:
    args->ils_perturb_rate = strtof(arg, NULL);
    DPRINTF("ILS perturbation rate set to %f\n", args->ils_perturb_rate);
    break;
  case KEY_ILS_N_TRY:
    args->ils_n_try = (size_t)strtoul(arg, NULL, 10);
    DPRINTF("ILS number of try set to %zu\n", args->ils_n_try);
    break;
  case KEY_ILS_WORST:
    args->ils_worse = (t_cost)strtoul(arg, NULL, 10);
    DPRINTF("ILS worse bracket set to %u\n", args->ils_worse);
    break;
  case KEY_MEMETIC_N_POPULATION:
    args->memetic_n_population = (size_t)strtoul(arg, NULL, 10);
    DPRINTF("Memetic population size set to %zu\n", args->memetic_n_population);
    break;
  case KEY_MEMETIC_N_DIVERSI_TRY:
    args->memetic_n_diversi_try = (size_t)strtoul(arg, NULL, 10);
    DPRINTF("Memetic number of try for diversity set to %zu\n",
            args->memetic_n_diversi_try);
    break;
  case KEY_MEMETIC_N_MEAN_TRY:
    args->memetic_n_mean_try = (ushort)strtoul(arg, NULL, 10);
    DPRINTF("Memetic number of try for mean set to %u\n",
            args->memetic_n_mean_try);
    break;
  case KEY_MEMETIC_CROSS_RATE_MUT:
    args->memetic_cross_rate_mut = strtof(arg, NULL);
    DPRINTF("Memetic offspring cross mutation rate set to %f\n",
            args->memetic_cross_rate_mut);
    break;
  case KEY_MEMETIC_N_OFFSPRING:
    args->memetic_n_offspring = (size_t)strtoul(arg, NULL, 10);
    DPRINTF("Memetic number of offspring set to %zu\n",
            args->memetic_n_offspring);
    break;
  case KEY_MEMETIC_MUTATION_RATE:
    args->memetic_mutation_rate = strtof(arg, NULL);
    DPRINTF("Memetic mutation rate set to %f\n", args->memetic_mutation_rate);
    break;
  case KEY_MEMETIC_CROSS_RATE:
    args->memetic_cross_rate = strtof(arg, NULL);
    DPRINTF("Memetic crossover rate set to %f\n", args->memetic_cross_rate);
    break;
  case ARGP_KEY_ARG:
    if (!args->is_pos_arg) {
      argp_usage(state);
    }
    if (state->arg_num >=
        (int)(sizeof(args->pos_args) / sizeof(args->pos_args[0]))) {
      argp_usage(state);
    }
    args->pos_args[state->arg_num] = arg;
    break;
  case ARGP_KEY_END:
    if (args->is_pos_arg && state->arg_num < (int)(sizeof(args->pos_args) /
                                                   sizeof(args->pos_args[0]))) {
      argp_error(state, "Not enough positional arguments.");
    }
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

void init_arguments(struct arguments *args) {
  assert(args != NULL);

  args->is_pos_arg = false;
  args->pos_args[0] = NULL;
  args->verbose = false;
  args->result = false;
  args->instance_file = "data/input/instances/N-be75eec_150";
  // args->out_file = "data/output/benchmark.csv";
  args->n_neighb_vnd = 0;
  args->pivot_rule = FIRST;
  args->start_rule = C_AND_W;
  args->neighb_exploration[0] = EXCHANGE;
  args->fptr_neighb_exploration[0] = cost_delta_exchange;
  args->fptr_sol_start = sol_start_cw;
  args->algo = VND;
  args->ils_perturb_rate = 0.1;
  args->ils_n_try = 10;
  args->ils_worse = 0;

  args->memetic_n_population = 20;
  args->memetic_n_offspring = 10;
  args->memetic_n_diversi_try = 5;
  args->memetic_n_mean_try = 10;
  args->memetic_cross_rate_mut = 0.8;
  args->memetic_mutation_rate = 0.1;
  args->memetic_cross_rate = 0.5;
}

void parse_arguments(int argc, char **argv, struct arguments *args) {
  assert(args != NULL);
  argp_parse(&argp, argc, argv, 0, 0, args);

  if (args->n_neighb_vnd == 0) {
    // If no neighborhood was specified, use the default one (exchange)
    DPRINTF("no neighborhood method specified, using default : exchange\n");
  }
}

void verbose_printf(const char *func_name, const char *fmt, ...) {
  if (!arguments.verbose) {
    return;
  }

  printf("verbose: %s: ", func_name);

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void print_result(const t_cost cost, const float elapsed_seconds,
                  const size_t *const sol, const size_t sol_size) {
  printf("RESULT cost=%u time=%f solution=", cost, elapsed_seconds);
  print_array_1d((const long int *const)sol, sol_size);
}

inline const bool is_print_result() { return arguments.result; }

static clock_t result_clock = 0;
void set_result_clock() { result_clock = clock(); }

void result_printer(const t_cost cost, const size_t *const sol,
                    const size_t sol_size, const bool force) {
  if (is_print_result() || force) {
    print_result(cost, get_elapsed_s(result_clock), sol, sol_size);
  }
}
