#ifndef _ARG_PARSER_H_
#define _ARG_PARSER_H_

#include "utilities.h"
#include <argp.h>
#include <stdbool.h>
#include <sys/types.h>

static const char doc[] = "Variable neighborhood descent LOP instance resolver";
static const char args_doc[] = "";

struct arguments {
  bool is_pos_arg;
  char *pos_args[1];
  char *instance_file;
  char *out_file;
  bool verbose;
  bool result;

  unsigned short n_neighb_vnd;
  enum pivot_enum pivot_rule;
  enum neighb_enum neighb_exploration[10];
  enum start_enum start_rule;

  t_fptr_delta_neigh_exploration fptr_neighb_exploration[10];
  t_fptr_sol_start fptr_sol_start;

  enum algo_enum algo;

  float ils_perturb_rate;
  size_t ils_n_try;
  t_cost ils_worse;

  size_t memetic_n_population;
  size_t memetic_n_diversi_try;
  ushort memetic_n_mean_try;
  float memetic_cross_rate_mut;
  size_t memetic_n_offspring;
  float memetic_mutation_rate;
  float memetic_cross_rate;
};

enum {
  KEY_VERBOSE = 'v',
  KEY_RESULT = 'r',
  KEY_INSTANCE = 'i',
  KEY_PIVOT = 'p',
  KEY_NEIGHB = 'n',
  KEY_SOL_START = 's',
  KEY_ALGO = 'a',

  KEY_ILS_PETURB_RATE = 128,
  KEY_ILS_N_TRY,
  KEY_ILS_WORST,

  KEY_MEMETIC_N_POPULATION,
  KEY_MEMETIC_N_DIVERSI_TRY,
  KEY_MEMETIC_N_MEAN_TRY,
  KEY_MEMETIC_CROSS_RATE_MUT,
  KEY_MEMETIC_N_OFFSPRING,
  KEY_MEMETIC_MUTATION_RATE,
  KEY_MEMETIC_CROSS_RATE,
};

extern struct arguments arguments;

void init_arguments(struct arguments *args);
void parse_arguments(int argc, char **argv, struct arguments *args);
void verbose_printf(const char *func_name, const char *fmt, ...);

void print_result(const t_cost cost, const float elapsed_seconds,
                  const size_t *const sol, const size_t sol_size);
const bool is_print_result();
void set_result_clock();
void result_printer(const t_cost cost, const size_t *const sol,
                    const size_t sol_size, const bool force);

#define PVERB(...)                                                             \
  do {                                                                         \
    verbose_printf(__func__, __VA_ARGS__);                                     \
  } while (0)

#endif //_ARG_PARSER_H_
