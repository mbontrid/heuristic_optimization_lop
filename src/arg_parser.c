#include <argp.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "arg_parser.h"
#include "utilities.h"

struct arguments arguments;

static const char doc[] = "LOP instance resolver";
static const char args_doc[] = "";

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
  struct arguments *args = state->input;

  switch (key) {
  case 'v':
    args->verbose = true;
    break;
  case 'o':
    args->out_file = arg;
    break;
  case 'i':
    args->instance_file = arg;
    break;
  case 'p':
    if (strcmp(arg, "first") == 0) {
      args->fptr_pivoting_rule = pivot_first;
      DEBUG_PRINT("pivoting rule set to first");
    } else if (strcmp(arg, "best") == 0) {
      args->fptr_pivoting_rule = pivot_best;
      DEBUG_PRINT("pivoting rule set to best");
    } else {
      argp_error(state, "Invalid pivoting_rule option: %s", arg);
    }
    break;
  case 'n':
    if (strcmp(arg, "transpose") == 0) {
      args->fptr_neighborhood = neighb_transpose_deltas;
      DEBUG_PRINT("neighborhood set to transpose");
    } else if (strcmp(arg, "exchange") == 0) {
      args->fptr_neighborhood = neighb_exchange_deltas;
      DEBUG_PRINT("neighborhood set to exchange");
    } else if (strcmp(arg, "insert") == 0) {
      args->fptr_neighborhood = neighb_insert_deltas;
      DEBUG_PRINT("neighborhood set to insert");
    } else {
      argp_error(state, "Invalid neighborhood option: %s", arg);
    }
    break;
  case 's':
    if (strcmp(arg, "random") == 0) {
      args->fptr_sol_start = sol_start_random;
      DEBUG_PRINT("solution start set to random");
    } else if (strcmp(arg, "c_and_w") == 0) {
      args->fptr_sol_start = sol_start_c_and_w;
      DEBUG_PRINT("solution start set to c_and_w");
    } else {
      argp_error(state, "Invalid initial solution option: %s", arg);
    }
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
    if (args->is_pos_arg &&
        state->arg_num <
            (int)(sizeof(args->pos_args) / sizeof(args->pos_args[0]))) {
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
  args->instance_file = "instances/N-be75eec_150";
  args->out_file = "data/last_results";
  args->fptr_pivoting_rule = pivot_first;
  args->fptr_neighborhood = neighb_transpose_deltas;
  args->fptr_sol_start = sol_start_random;
}

void parse_arguments(int argc, char **argv, struct arguments *args) {
  assert(args != NULL);
  argp_parse(&argp, argc, argv, 0, 0, args);
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
