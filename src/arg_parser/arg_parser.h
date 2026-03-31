#ifndef _ARG_PARSER_H_
#define _ARG_PARSER_H_

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
  char *pos_args[1]; // positional argumuments of the command line calla.
  char *instance_file;
  char *out_file;
  bool verbose;

  enum pivoting_rule pivoting_rule;
  enum neighborhood neighborhood;
  enum sol_start sol_start;

  int (*fptr_pivoting_rule)(int, int);
  int (*fptr_neighborhood)(int, int);
  int (*fptr_sol_start)(int, int);
};

static const char doc[] = "LOP instance resolver";
static const char args_doc[] = "";

#endif //_ARG_PARSER_H_
