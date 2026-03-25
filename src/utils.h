

#ifndef _UTILS_H_
#define _UTILS_H_

/* inline function compiled only if NDEBUG is not defined.
 * Each inline definition has to have a empty definiation in the else branch.
 */
#ifndef NDEBUG

#define DEBUG_PRINT(msg) printf("DEBUG: %s\n", msg);

#else

#define DEBUG_PRINT(msg)

#endif

#endif
