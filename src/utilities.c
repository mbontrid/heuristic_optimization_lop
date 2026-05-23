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
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <values.h>

#include "utilities.h"

long int Seed;
static volatile sig_atomic_t g_interrupt_requested = 0;

static void handle_interrupt(int signal_number) {
  (void)signal_number;
  g_interrupt_requested = 1;
}

void install_interrupt_handler(void) {
  signal(SIGINT, handle_interrupt);
  signal(SIGTERM, handle_interrupt);
}

bool is_interrupt_requested(void) { return g_interrupt_requested != 0; }

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

size_t *generate_incr_vector(const size_t size) {
  size_t *new_vector = malloc(size * sizeof(*new_vector));

  for (size_t i = 0; i < size; i++) {
    new_vector[i] = i;
  }
  return new_vector;
}

void randomize_vector(size_t *restrict const array, const size_t size) {
  for (size_t i = 0; i < size - 1; i++) {
    const size_t j = randInt(i + 1, size);
    swap(array, i, j);
  }
}

size_t *generate_rand_no_rep_array(const size_t size)
/* Generates a random vector, quick and dirty */
{
  size_t *random_vector = generate_incr_vector(size);

  randomize_vector(random_vector, size);

  assert(get_max_array(random_vector, size) < size);
  return random_vector;
}

size_t *gener_no_rep_rand(const size_t min, const size_t max) {
  size_t *random_vector = generate_rand_no_rep_array(max - min + 1);
  for (size_t i = 0; i < max - min + 1; i++) {
    random_vector[i] += min;
  }
  return random_vector;
}

void swap(size_t *array_1d, const size_t i, const size_t j) {
  size_t tmp = array_1d[i];
  array_1d[i] = array_1d[j];
  array_1d[j] = tmp;
}

void rand_swap(size_t *restrict const array, const size_t size, size_t *const i,
               size_t *const j) {
  *i = randInt(0, size - 1);
  *j = randInt(0, size - 1);
  swap(array, *i, *j);
}

bool array_equal(const size_t *restrict const array_1d_1,
                 const size_t *restrict const array_1d_2, const size_t size) {

  return !memcmp(array_1d_1, array_1d_2, size * sizeof(size_t));
}

size_t find_array_in_arrays(const size_t *restrict const array,
                            const size_t *restrict const array_of_arrays,
                            const size_t size, const size_t n_arrays) {
  for (size_t i = 0; i < n_arrays; i++) {
    if (array_equal(array, &array_of_arrays[i * size], size)) {
      return i;
    }
  }
  return n_arrays;
}

bool is_array_in_arrays(const size_t *restrict const array,
                        const size_t *restrict const array_of_arrays,
                        const size_t size, const size_t n_arrays) {
  return find_array_in_arrays(array, array_of_arrays, size, n_arrays) <
         n_arrays;
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

void print_array_2d(const t_mat_cell *const array_2d, const size_t n_rows,
                    const size_t n_columns) {
  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < n_columns; j++) {
      t_mat_cell value = array_2d[n_columns * i + j];
      printf("%u ", value);
    }
    printf("\n");
  }
}

void print_array_2d2(const t_mat_cell *const *const array_2d,
                     const size_t n_rows, const size_t n_columns) {
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

double get_elapsed_s(clock_t start) {
  const clock_t end = clock();
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

size_t get_max_id(const size_t *restrict const array, const size_t size) {
  size_t max = 0;
  for (size_t i = 0; i < size; i++) {
    if (array[i] > array[max]) {
      max = i;
    }
  }
  return max;
}

size_t get_min_id(const size_t *restrict const array, const size_t size) {
  size_t min = 0;
  for (size_t i = 0; i < size; i++) {
    if (array[i] < array[min]) {
      min = i;
    }
  }
  return min;
}

size_t get_max_array(const size_t *const array, const size_t size) {
  return array[get_max_id(array, size)];
}

size_t get_min_array(const size_t *const array, const size_t size) {
  return array[get_min_id(array, size)];
}

size_t *get_n_best_sorted(const size_t *restrict const array, const size_t n,
                          const size_t size) {
  size_t *restrict const best_indexes = malloc(n * sizeof(size_t));
  bool *restrict const selected = calloc(size, sizeof(*selected));

  for (size_t i = 0; i < n; i++) {
    size_t best_id = 0;
    bool found = false;
    for (size_t j = 0; j < size; j++) {
      if (selected[j]) {
        continue;
      }
      if (!found || array[j] > array[best_id]) {
        best_id = j;
        found = true;
      }
    }
    assert(found);
    selected[best_id] = true;
    best_indexes[i] = best_id;
  }
#ifndef NDEBUG
  for (size_t i = 0; i < n - 1; i++) {
    assert(array[best_indexes[i]] >= array[best_indexes[i + 1]]);
  }
#endif
  free(selected);
  return best_indexes;
}

t_cost get_max_array_cost(const t_cost *const array, const size_t size) {
  size_t max_id = 0;
  for (size_t i = 1; i < size; i++) {
    if (array[i] > array[max_id]) {
      max_id = i;
    }
  }
  return array[max_id];
}

size_t *get_n_best_sorted_cost(const t_cost *restrict const array,
                               const size_t n, const size_t size) {
  size_t *restrict const best_indexes = malloc(n * sizeof(size_t));
  bool *restrict const selected = calloc(size, sizeof(*selected));

  for (size_t i = 0; i < n; i++) {
    size_t best_id = 0;
    bool found = false;
    for (size_t j = 0; j < size; j++) {
      if (selected[j]) {
        continue;
      }
      if (!found || array[j] > array[best_id]) {
        best_id = j;
        found = true;
      }
    }
    assert(found);
    selected[best_id] = true;
    best_indexes[i] = best_id;
  }
#ifndef NDEBUG
  for (size_t i = 0; i + 1 < n; i++) {
    assert(array[best_indexes[i]] >= array[best_indexes[i + 1]]);
  }
#endif
  free(selected);
  return best_indexes;
}

double get_mean(const t_cost *restrict const array, const size_t size) {
  double mean = 0;
  for (size_t i = 0; i < size; i++) {
    mean += array[i];
  }
  return mean / size;
}

bool is_array_overlap(const void *const array1, const size_t size1,
                      const void *const array2, const size_t size2) {
  char *a_start = (char *)array1;
  char *a_end = a_start + size1;

  char *b_start = (char *)array2;
  char *b_end = b_start + size2;

  return (a_start < b_end) && (b_start < a_end);
}
