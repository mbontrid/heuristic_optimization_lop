#ifndef _ARG_PARSER_H_
#define _ARG_PARSER_H_

#include "optimization.h"
#include <stdbool.h>

enum pivoting_rule {
  first,
  best,
};

enum neighborhood {
  transpose,
  exchange,
  insert,
};

enum sol_start {
  randome,
  c_and_w,
};

struct arguments {
  bool is_pos_arg;
  char *pos_args[1]; // positional argumuments of the command line calla.
  char *instance_file;
  char *out_file;
  bool verbose;

  enum pivoting_rule pivoting_rule;
  enum neighborhood neighborhood;
  enum sol_start sol_start;

  t_fptr_pivot_rule fptr_pivoting_rule;
  t_fptr_neighborhood fptr_neighborhood;
  t_fptr_sol_start fptr_sol_start;
};

static const char doc[] = "LOP instance resolver";
static const char args_doc[] = "";

#endif //_ARG_PARSER_H_
