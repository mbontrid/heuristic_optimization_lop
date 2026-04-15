#ifndef _ARG_PARSER_H_
#define _ARG_PARSER_H_

#include "utilities.h"
#include <stdbool.h>
#include <values.h>

struct arguments {
  bool is_pos_arg;
  char *pos_args[1];
  char *instance_file;
  char *out_file;
  bool verbose;

  unsigned short n_neighb_vnd;
  enum pivot_enum pivot_rule;
  enum neighb_enum neighb_exploration[MAXSHORT * 2];
  enum start_enum start_rule;

  t_fptr_delta_neigh_exploration fptr_neighb_exploration[MAXSHORT * 2];
  t_fptr_sol_start fptr_sol_start;
};

extern struct arguments arguments;

void init_arguments(struct arguments *args);
void parse_arguments(int argc, char **argv, struct arguments *args);
void verbose_printf(const char *func_name, const char *fmt, ...);

#define PVERB(...)                                                             \
  do {                                                                         \
    verbose_printf(__func__, __VA_ARGS__);                                     \
  } while (0)

#endif //_ARG_PARSER_H_
