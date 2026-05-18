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
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <values.h>

#include "utilities.h"

long int Seed;

t_mat_cell **createMatrixx(size_t dim) {

  int k;
  t_mat_cell **result = (t_mat_cell **)calloc(dim, sizeof(long int *));

  for (k = 0; k < dim; ++k) {
    result[k] = (t_mat_cell *)calloc(dim, sizeof(long int));
  }

  return result;
}

void fatal(char *s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

double ran01(long *idum) {
  /*
        FUNCTION:      returns a pseudo-random number
        INPUT:         a pointer to the seed variable
        OUTPUT:        a pseudo-random number uniformly distributed in [0,1]
        (SIDE)EFFECTS: changes the value of seed
  */
  long k;
  double ans;

  k = (*idum) / IQ;
  *idum = IA * (*idum - k * IQ) - IR * k;
  if (*idum < 0)
    *idum += IM;
  ans = AM * (*idum);
  // DPRINTF(" : %f\n", ans);
  return ans;
}

/* Return random integer in the (inclusive) range
 * [minimum, maximum]
 */
int randInt(int minimum, int maximum) {
  return ((int)(ran01(&Seed) * (maximum - minimum + 1)) + minimum);
}

extern bool rand_bool() { return randInt(0, 1) == 1; }

size_t *generate_incr_vector(size_t size) {
  size_t *new_vector = malloc(size * sizeof(size));

  for (size_t i = 0; i < size; i++) {
    new_vector[i] = i;
  }
  return new_vector;
}

size_t *generate_random_vector(size_t dim)
/* Generates a random vector, quick and dirty */
{
  size_t *random_vector;
  int i, help, node, tot_assigned = 0;
  double rnd;

  random_vector = malloc(dim * sizeof(size_t));

  if (!random_vector) {
    fatal("Error on random_vector malloc\n");
  }

  for (i = 0; i < dim; i++)
    random_vector[i] = i;

  for (i = 0; i < dim; i++) {
    /* find (randomly) an index for a free unit */
    rnd = ran01(&Seed);
    node = (long int)(rnd * (dim - tot_assigned));
    help = random_vector[i];
    random_vector[i] = random_vector[i + node];
    random_vector[i + node] = help;
    tot_assigned++;
    // #ifndef NDEBUG
    //     DPRINTF("Construction of randome vector at step %i: ", i);
    //     for (size_t j = 0; j < dim; j++) {
    //       printf("%du ", random_vector[j]);
    //     }
    //     printf("\n");
    // #endif
  }

#ifndef NDEBUG
  DPRINTF("random_vector : ");
  for (size_t i = 0; i < dim; i++) {
    printf("%ld ", random_vector[i]);
  }
  printf("\n");
#endif

  return random_vector;
}

size_t *gener_no_rep_rand(size_t min, size_t max) {
  size_t *random_vector = generate_random_vector(max - min + 1);
  for (size_t i = 0; i < max - min + 1; i++) {
    random_vector[i] += min;
  }
  return random_vector;
}

void swap(size_t *array_1d, size_t i, size_t j) {
  size_t tmp = array_1d[i];
  array_1d[i] = array_1d[j];
  array_1d[j] = tmp;
}

bool array_equal(const size_t *const array_1d_1, const size_t *const array_1d_2,
                 size_t size) {

  int a = memcmp(array_1d_1, array_1d_2, size * sizeof(size_t));

  return !a;
}

bool is_array_in_arrays(const size_t *const array,
                        const size_t *const array_of_arrays, const size_t size,
                        const size_t n_arrays) {
  for (size_t i = 0; i < n_arrays; i++) {
    if (array_equal(array, &array_of_arrays[i * size], size)) {
      return true;
    }
  }
  return false;
}

void ascending_sort(size_t *const array, const size_t size) {
  qsort(array, size, sizeof(size_t), cmp_asc);
}

void print_array_1d(const long int *const array, const size_t n_columns) {
  for (size_t i = 0; i < n_columns; i++) {
    printf("%ld ", array[i]);
  }
  printf("\n");
}

void print_array_2d(t_mat_cell *array_2d, size_t n_rows, size_t n_columns) {
  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < n_columns; j++) {
      t_mat_cell value = array_2d[n_columns * i + j];
      printf("%u ", value);
    }
    printf("\n");
  }
}

void print_array_2d2(t_mat_cell **array_2d, size_t n_rows, size_t n_columns) {
  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < n_columns; j++) {
      t_mat_cell value = array_2d[i][j];
      printf("%u ", value);
    }
    printf("\n");
  }
}

unsigned int factorial(unsigned int N) {
  unsigned int fact = 1, i;

  for (i = 1; i <= N; i++) {
    fact *= i;
  }
  return fact;
}

int cmp_desc(const void *a, const void *b) {
  double diff = ((Item *)b)->value - ((Item *)a)->value;
  if (diff > 0)
    return 1;
  if (diff < 0)
    return -1;
  return 0;
}

int cmp_asc(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

double end_clock(clock_t start) {
  clock_t end = clock();
  return (double)(end - start) / CLOCKS_PER_SEC;
}

void array_apply_shuffle(size_t *const result, const size_t *const shuffle,
                         const size_t *const to_shuffle, const size_t size) {
#pragma omp simd
  for (size_t i = 0; i < size; i++) {
    // DPRINTF("shuffle=%d | to_shuffle=%d\n", shuffle[i], to_shuffle[i]);
    assert(shuffle[i] < size);
    result[shuffle[i]] = to_shuffle[i];
  }
}

size_t get_max_array(const size_t *const array, const size_t size) {
  size_t max = 0;
  for (size_t i = 0; i < size; i++) {
    if (array[i] > max) {
      max = array[i];
    }
  }
  return max;
}

size_t get_min_array(const size_t *const array, const size_t size) {
  size_t min = UINT_MAX;
  for (size_t i = 0; i < size; i++) {
    if (array[i] < min) {
      min = array[i];
    }
  }
  return min;
}
