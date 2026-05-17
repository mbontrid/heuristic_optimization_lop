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
#include <time.h>
#define PREV 0
#define NEXT 1
#define E_START 0
#define E_END 1

typedef unsigned int t_mat_cell;
typedef unsigned int t_cost;
typedef long int t_cost_delta;

#define MAX_COST UINT_MAX
#define MAX_COST_CELL UINT_MAX
#define MAX_SIZEMAT UINT_MAX

#define max(A, B) ((A) > (B) ? (A) : (B))
#define min(A, B) ((A) < (B) ? (A) : (B))

typedef struct {
  int index;
  double value;
} Item;

enum pivot_enum {
  BEST = false,
  FIRST = true,
};

enum neighb_enum {
  TRANSPOSE,
  INSERT,
  EXCHANGE,
};

enum start_enum {
  RANDOM,
  C_AND_W,
};

typedef size_t *(*t_fptr_sol_start)(t_mat_cell *CostMat, size_t size);

typedef t_cost_delta (*t_fptr_delta_neigh_exploration)(t_mat_cell *cost_mat_2d,
                                                       size_t *const sol_1d,
                                                       size_t size,
                                                       bool is_first);

t_mat_cell **createMatrixx(size_t i);
int rand0N(int limit);

extern void fatal(char *s);

extern double ran01(long *idum);

extern int randInt(int minimum, int maximum);

extern bool rand_bool();

size_t *generate_incr_vector(size_t size);
extern size_t *generate_random_vector(size_t dim);

/**
 * @brief Swap two element in a array
 *
 * @param array_1d Array to be modified.
 * @param i First index to swap.
 * @param j Scond index to swap.
 */
void swap(size_t *array_1d, size_t i, size_t j);
bool array_equal(const size_t *array_1d_1, const size_t *array_1d_2,
                 size_t size);

void ascending_sort(size_t *const array, const size_t size);

/**
 * @brief Print the elmements of a 1d array.
 *
 * @param array Array to be printed.
 * @param n_columns size of the array.
 */
void print_array_1d(const long int *const array, const size_t n_columns);
/**
 * @brief Print the elements of a 2d array stored as a 1d array.
 *
 * @param array_2d Array to be printed.
 * @param n_rows Number of rows in the array.
 * @param n_columns Number of columns in the array.
 */
void print_array_2d(t_mat_cell *array_2d, size_t n_rows, size_t n_columns);
/**
 * @brief Print the elements of a 2d array storsd as a pointer array of pointer.
 *
 * @param array_2d Array to be printed.
 * @param n_rows Number of rows in the array.
 * @param n_columns Number of columns in the array.
 */
void print_array_2d2(t_mat_cell **array_2d, size_t n_rows, size_t n_columns);

unsigned int factorial(unsigned int N);

// Comparator for descending order
int cmp_desc(const void *a, const void *b);
// Comparator for ascending order
int cmp_asc(const void *a, const void *b);

double end_clock(clock_t start);

/**
 * @brief reorder an array based on the indice of another array. The resulting
 * array and the shuffle array have to be of the same size.
 */
void array_apply_shuffle(size_t *const modified, const size_t *const shuffle,
                         const size_t *const to_shuffle, const size_t size);

size_t get_max_array(const size_t *const array, const size_t size);
size_t get_min_array(const size_t *const array, const size_t size);

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

#define DPRINTF(...)                                                           \
  printf("DEBUG: %s: ", __func__);                                             \
  printf(__VA_ARGS__);

#define DNPRINTF(...) printf(__VA_ARGS__);

#define PARRAY(array, n_columns)                                               \
  do {                                                                         \
    printf("DEBUG: %s: ", __func__);                                           \
    print_array_1d(array, n_columns);                                          \
  } while (0)

#else

#define DPRINTF(...)

#define DNPRINTF(...)

#define PARRAY(array, n_columns)

#endif

#endif
