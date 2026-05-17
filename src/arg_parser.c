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

static const char doc[] = "Variable neighborhood descent LOP instance resolver";
static const char args_doc[] = "";

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Returns verbose output."},
    // {"output", 'o', "FILE", 0,
    // "Returns output to file instead of standard input"},
    {"instance", 'i', "FILE", 0,
     "Instance file to use. Default=data/input/instances/N-be75eec_150"},
    {"pivot", 'p', "CHOICE", 0, "Pivoting rule: first|best"},
    {"neighborhood", 'n', "CHOICE", 0,
     "Neighborhood strategy: (transpose|exchange|insert). Multiple -n CHOICE "
     "can be set for a VND algorith. Default : exahange"},
    {"sol_start", 's', "CHOICE", 0, "Initial solution: random|c_and_w"},
    {"algo", 'a', "CHOICE", 0, "Algorithme to use. (vnd|ils|memetic)"},
    {"ils_perturb_rate", 'r', "FLOAT", 0,
     "Perturbation rate for iterated local search. Default : 0.1"},
    {"ils_n_try", 't', "SIZE_T", 0,
     "Number of tries for iterated local search. Default : 10"},
    {"worst", 'w', "COST", 0,
     "Worst bracket for accepting worse solution in iterated local search. "
     "Default : 0"},
    {0},
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *args = state->input;

  switch (key) {
  case 'v':
    args->verbose = true;
    break;
  // case 'o':
  //   args->out_file = arg;
  //   break;
  case 'i':
    args->instance_file = arg;
    break;
  case 'p':
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
  case 'n':
    assert(args->n_neighb_vnd < MAXSHORT * 2);
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
  case 's':
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
  case 'a':
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
  case 'r':
    args->ils_perturb_rate = strtof(arg, NULL);
    DPRINTF("ILS perturbation rate set to %f\n", args->ils_perturb_rate);
    break;
  case 't':
    args->ils_n_try = (size_t)strtoul(arg, NULL, 10);
    DPRINTF("ILS number of try set to %zu\n", args->ils_n_try);
    break;
  case 'w':
    args->ils_worse = (t_cost)strtoul(arg, NULL, 10);
    DPRINTF("ILS worse bracket set to %lu\n", args->ils_worse);
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
  args->instance_file = "data/input/instances/N-be75eec_150";
  // args->out_file = "data/output/benchmark.csv";
  args->n_neighb_vnd = 0;
  args->pivot_rule = BEST;
  args->start_rule = C_AND_W;
  args->neighb_exploration[0] = EXCHANGE;
  args->fptr_neighb_exploration[0] = cost_delta_exchange;
  args->fptr_sol_start = sol_start_cw;
  args->algo = VND;
  args->ils_perturb_rate = 0.1;
  args->ils_n_try = 10;
  args->ils_worse = 0;
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
