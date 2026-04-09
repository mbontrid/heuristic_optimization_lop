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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include "utilities.h"

long int Seed;

t_mat_cell **createMatrixx(t_sizemat dim) {

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

t_sizemat *generate_inc_vector(t_sizemat size) {
  t_sizemat *new_vector = malloc(size * sizeof(size));

  for (t_sizemat i = 0; i < size; i++) {
    new_vector[i] = i;
  }
  return new_vector;
}

t_sizemat *generate_random_vector(t_sizemat dim)
/* Generates a random vector, quick and dirty */
{
  t_sizemat *random_vector;
  int i, help, node, tot_assigned = 0;
  double rnd;

  random_vector = malloc(dim * sizeof(t_sizemat));

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
    //     for (t_sizemat j = 0; j < dim; j++) {
    //       printf("%ld ", random_vector[j]);
    //     }
    //     printf("\n");
    // #endif
  }

#ifndef NDEBUG
  DPRINTF("random_vector : ");
  for (t_sizemat i = 0; i < dim; i++) {
    printf("%ld ", random_vector[i]);
  }
  printf("\n");
#endif

  return random_vector;
}

bool array_equal(const t_sizemat *const array_1d_1,
                 const t_sizemat *const array_1d_2, t_sizemat size) {
  for (t_sizemat i = 0; i < size; i++) {
    if (array_1d_1[i] != array_1d_2[i]) {
      return false;
    }
  }
  return true;
}

void print_array_1d(t_mat_cell *array, t_sizemat n_columns) {
  for (t_sizemat i = 0; i < n_columns; i++) {
    printf("%u ", array[i]);
  }
  printf("\n");
}

void print_array_2d(t_mat_cell *array_2d, t_sizemat n_rows,
                    t_sizemat n_columns) {
  for (t_sizemat i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      t_mat_cell value = array_2d[n_columns * i + j];
      printf("%u ", value);
    }
    printf("\n");
  }
}

void print_array_2d2(t_mat_cell **array_2d, t_sizemat n_rows,
                     t_sizemat n_columns) {
  for (t_sizemat i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
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
