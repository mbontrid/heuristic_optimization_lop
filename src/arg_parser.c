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
    {"method", 'm', "CHOICE", 0, "Methode of LOP : it_imp_lop|vnd_lop"},
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
  case 'm':
    if (strcmp(arg, "it_imp_lop") == 0) {
      args->method = "it_imp_lop";
      DPRINTF("algorithm set to it_imp_lop\n");
    } else if (strcmp(arg, "vnd_lop") == 0) {
      args->method = "vnd_lop";
      DPRINTF("algorithm set to vnd_lop\n");
    } else {
      argp_error(state, "Invalid algorithm option: %s", arg);
    }
    break;
  case 'p':
    if (strcmp(arg, "first") == 0) {
      args->fptr_pivoting_rule = pivot_first;
      DPRINTF("pivoting rule set to first\n");
    } else if (strcmp(arg, "best") == 0) {
      args->fptr_pivoting_rule = pivot_best;
      DPRINTF("pivoting rule set to best\n");
    } else {
      argp_error(state, "Invalid pivoting_rule option: %s", arg);
    }
    break;
  case 'n':
    assert(args->n_neighb_vnd < MAXSHORT * 2);
    DPRINTF("neighborhood method added : %s", arg);
    if (strcmp(arg, "transpose") == 0) {
      args->fptrs_neighborhood[args->n_neighb_vnd++] = neighb_transpose_deltas;
      DPRINTF("neighborhood set to transpose\n");
    } else if (strcmp(arg, "exchange") == 0) {
      args->fptrs_neighborhood[args->n_neighb_vnd++] = neighb_exchange_deltas;
      DPRINTF("neighborhood set to exchange\n");
    } else if (strcmp(arg, "insert") == 0) {
      args->fptrs_neighborhood[args->n_neighb_vnd++] = neighb_insert_deltas;
      DPRINTF("neighborhood set to insert\n");
    } else {
      argp_error(state, "Invalid neighborhood option: %s", arg);
    }
    break;
  case 's':
    if (strcmp(arg, "random") == 0) {
      args->fptr_sol_start = sol_start_random;
      DPRINTF("solution start set to random\n");
    } else if (strcmp(arg, "c_and_w") == 0) {
      args->fptr_sol_start = sol_start_c_and_w;
      DPRINTF("solution start set to c_and_w\n");
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
  args->method = "it_imp_lop";
  args->verbose = false;
  args->instance_file = "data/input/instances/N-be75eec_150";
  args->out_file = "data/output/benchmark.csv";
  args->fptr_pivoting_rule = pivot_first;
  args->n_neighb_vnd = 0;
  args->fptrs_neighborhood[0] = neighb_transpose_deltas;
  args->fptrs_neighborhood[1] = neighb_exchange_deltas;
  args->fptrs_neighborhood[2] = neighb_insert_deltas;
  args->fptr_sol_start = sol_start_random;
}

void parse_arguments(int argc, char **argv, struct arguments *args) {
  assert(args != NULL);
  argp_parse(&argp, argc, argv, 0, 0, args);

  if (strcmp(arguments.method, "it_imp_lop") == 0) {
    DPRINTF("method is it_imp_lop, setting n_neighb_vnd to 1\n");
    arguments.n_neighb_vnd = 1;
  } else if (strcmp(arguments.method, "vnd_lop") == 0) {
    DPRINTF("method is vnd_lop, setting n_neighb_vnd to 3 (transpose -> "
            "exchange -> insert) if not set by user\n");
    if (arguments.n_neighb_vnd == 0) {
      arguments.n_neighb_vnd = 3;
    }
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
