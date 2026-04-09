/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <limits.h>
#include <stdbool.h>
#define PREV 0
#define NEXT 1
#define E_START 0
#define E_END 1

typedef unsigned int t_sizemat;
typedef unsigned int t_mat_cell;
typedef unsigned int t_cost;

#define MAX_COST UINT_MAX
#define MAX_COST_CELL UINT_MAX

t_mat_cell **createMatrixx(t_sizemat i);
int rand0N(int limit);

extern void fatal(char *s);

extern double ran01(long *idum);

extern int randInt(int minimum, int maximum);

t_sizemat *generate_inc_vector(t_sizemat size);
extern t_sizemat *generate_random_vector(t_sizemat dim);

bool array_equal(const t_sizemat *const array_1d_1,
                 const t_sizemat *const array_1d_2, t_sizemat size);

void print_array_1d(t_mat_cell *array, t_sizemat n_columns);
void print_array_2d(t_mat_cell *array_2d, t_sizemat n_rows,
                    t_sizemat n_columns);
void print_array_2d2(t_mat_cell **array_2d, t_sizemat n_rows,
                     t_sizemat n_columns);

unsigned int factorial(unsigned int N);

/* constants for a pseudo-random number generator, taken from
   Numerical Recipes in C book --- never trust the standard C random
   number generator */
#define IA 16807
#define IM 2147483647
#define AM (1.0 / IM)
#define IQ 127773
#define IR 2836

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

extern long int Seed; /* seed for the random number generator */

/* inline function compiled only if NDEBUG is not defined.
 * Each inline definition has to have a empty definiation in the else branch.
 */
#ifndef NDEBUG

#define DEBUG_PRINT(msg) printf("DEBUG: %s\n", msg);

#define DPRINTF(...)                                                           \
  printf("DEBUG: %s: ", __func__);                                             \
  printf(__VA_ARGS__);

#define DNPRINTF(...) printf(__VA_ARGS__);

#else

#define DEBUG_PRINT(msg)

#define DPRINTF(...)

#define DNPRINTF(...)

#endif

#endif
