

#ifndef _UTILS_H_
#define _UTILS_H_

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
