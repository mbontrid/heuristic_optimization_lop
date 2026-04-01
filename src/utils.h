

#ifndef _UTILS_H_
#define _UTILS_H_

typedef int (*t_fptr_pivot_rule)(long int **matrix);
typedef int (*t_fptr_neighborhood)(int a, int b, long int **matrix);
typedef int (*t_fptr_sol_start)(int, int);

/* inline function compiled only if NDEBUG is not defined.
 * Each inline definition has to have a empty definiation in the else branch.
 */
#ifndef NDEBUG

#define DEBUG_PRINT(msg) printf("DEBUG: %s\n", msg);

#define DPRINTF(...)                                                           \
  printf("DEBUG: ");                                                           \
  printf(__VA_ARGS__);

#else

#define DEBUG_PRINT(msg)

#define DPRINTF(...)

#endif

#endif
