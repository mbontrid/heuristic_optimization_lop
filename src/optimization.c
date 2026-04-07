/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Lcaol Search Algorithm for Linear Ordering Problem
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include "instance.h"
#include "optimization.h"
#include "utilities.h"

#ifdef __MINGW32__
#include <float.h>
#define MAX_FLOAT FLT_MAX
#else
#define MAX_FLOAT MAXFLOAT
#endif

t_mat_cell **CostMat;

t_cost computeCost(t_sizemat *s) {
  int h, k;
  long long int sum = 0;

  /* Diagonal value are not considered */
  for (sum = 0, h = 0; h < PSize; h++)
    for (k = h + 1; k < PSize; k++)
      sum += CostMat[s[h]][s[k]];
  return (sum);
}

void createRandomSolution(t_sizemat *s) {
  int j;
  t_sizemat *random;

  random = generate_random_vector(PSize);
  for (j = 0; j < PSize; j++) {
    s[j] = random[j];
  }
  free(random);
}

t_mat_cell *prefix_sum_per_row_2d(t_mat_cell **mat, t_sizemat n_rows,
                                  t_sizemat n_columns) {

  t_mat_cell *sum_row_2d = malloc(n_columns * n_columns * sizeof(t_mat_cell));
  for (t_sizemat i = 0; i < n_columns; i++) {
    sum_row_2d[n_columns * i] = mat[i][0];
    for (t_sizemat j = 1; j < n_columns; j++) {
      sum_row_2d[n_columns * i + j] =
          sum_row_2d[n_columns * i + j - 1] + mat[i][j];
    }
  }

  return sum_row_2d;
}

ulong get_n_transpose(uint size) {

  if (size < 2) {
    return 0;
  }
  DPRINTF("get_n_transpose: for a %u array, there is %u transpose\n", size,
          (size));
  return size;
}

void get_transpose(t_sizemat *transposes, t_sizemat n_columns, ulong n_rows) {

  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      transposes[n_columns * i + j] = j;
    }
    t_sizemat temp = transposes[n_columns * i + i];
    transposes[n_columns * i + i] = transposes[n_columns * i + i + 1];
    transposes[n_columns * i + (i + 1) % n_columns] = temp;
  }

#ifndef NDEBUG
  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      printf("%ld ", transposes[n_columns * i + j]);
    }
    printf("\n");
  }
#endif
}

struct neighb neighborhood_tranpose(t_sizemat n_columns) {
  DPRINTF("executing neighborhood_tranpose\n");
  DPRINTF("allocating memory for transposes possibilities\n");
  ulong n_rows = get_n_transpose(n_columns);
  t_sizemat *transposes = malloc(n_rows * n_columns * sizeof(t_sizemat));
  DPRINTF("%lu transoposes possibilities allocated\n", n_rows);

  get_transpose(transposes, n_columns, n_rows);

  struct neighb neighb;
  neighb.neighborhood_modif_2d = transposes;
  neighb.n_rows = n_rows;
  neighb.n_columns = n_columns;
  return neighb;
}

uint get_n_exchange(t_sizemat size) {
  if (size < 2) {
    return 0;
  }
  return size * (size - 1) / 2;
}

void get_exchange(t_sizemat *exchanges, t_sizemat n_columns, uint n_rows) {

  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      exchanges[n_columns * i + j] = j;
    }
  }

  uint row = 0;
  for (t_sizemat i = 0; i < n_columns - 1; i++) {
    for (t_sizemat j = i + 1; j < n_columns; j++) {
      t_sizemat temp = exchanges[n_columns * row + i];
      exchanges[n_columns * row + i] = exchanges[n_columns * row + j];
      exchanges[n_columns * row + j] = temp;
      row++;
    }
  }

#ifndef NDEBUG
  DPRINTF("executing for %ld collumns and %ud rows=\n", n_columns, n_rows);
  for (ulong i = 0; i < n_rows; i++) {
    for (t_sizemat j = 0; j < n_columns; j++) {
      printf("%ld ", exchanges[n_columns * i + j]);
    }
    printf("\n");
  }
#endif
}

struct neighb neighborhood_exchange(t_sizemat n_columns) {
  DEBUG_PRINT("executing neighborhood_exchange");
  ulong n_rows = get_n_exchange(n_columns);
  t_sizemat *exchanges = malloc(n_rows * n_columns * sizeof(t_sizemat));
  DPRINTF("%lu exchanges possibilities allocated\n", n_rows);

  get_exchange(exchanges, n_columns, n_rows);

  struct neighb neighb;
  neighb.neighborhood_modif_2d = exchanges;
  neighb.n_rows = n_rows;
  neighb.n_columns = n_columns;

  return neighb;
}

ulong get_n_inserts(uint size) {
  if (size < 2) {
    return 0;
  } else if (size == 2) {
    return 3;
  }
  DPRINTF("get_n_insert: for a %u array, there is %u insert\n", size,
          ((size - 1) * (size - 1)));

  return (size - 1) * (size - 1);
}

void get_inserts(t_sizemat *insertions, t_sizemat n_columns, ulong n_rows) {

  DPRINTF("%s: executing for %ld collumns and %ld rows=\n", __func__, n_columns,
          n_rows);

  ulong row = 0;

  // which number has to move
  for (t_sizemat i = 0; i < n_columns; i++) {
    // to which number i has to move
    for (t_sizemat j = 0; j < n_columns; j++) {
      // avoid duplicates
      if (i != j && i != j + 1) {
        // go through each number of a row
        for (t_sizemat x = 0; x < n_columns; x++) {
          // "move"" number to the left
          if (i < j && x >= i && x < j) {
            insertions[n_columns * row + x] = x + 1;
          } else if (i > j && x > j && x <= i) { // "move" number to the right
            insertions[n_columns * row + x] = x - 1;
          } else if (x == j) { // put i to j
            insertions[n_columns * row + x] = i;
          } else {
            insertions[n_columns * row + x] = x;
          }
        }
#ifndef NDEBUG
        for (t_sizemat x = 0; x < n_columns; x++) {
          printf("%ld ", insertions[n_columns * i + x]);
        }
        printf(" for i=%ld, j=%ld\n", i, j);
#endif
        row++;
      }
    }
  }
}

struct neighb neighborhood_insert(t_sizemat n_columns) {
  DEBUG_PRINT("executing neighborhood_insert");

  // allocate malloc for the number of insertions possible.
  ulong n_rows = get_n_inserts(n_columns);
  t_sizemat *insertions = malloc(n_rows * n_columns * sizeof(t_sizemat));

  get_inserts(insertions, n_columns, n_rows);
  printf("for %u collumns; total number of insert: %lu\n", n_columns, n_rows);

  struct neighb neighb;
  neighb.neighborhood_modif_2d = insertions;
  neighb.n_rows = n_rows;
  neighb.n_columns = n_columns;
  return neighb;
}

t_sizemat *sol_start_random(t_mat_cell **mat, t_sizemat n_columns) {
  DEBUG_PRINT("executing sol_start_random");

  t_sizemat *new_random_vector = generate_random_vector(n_columns);

#ifndef NDEBUG
  DPRINTF("random generated starting solution vector\n")
  for (t_sizemat i = 0; i < n_columns; i++) {
    printf("%lu ", new_random_vector[i]);
  }
  printf("\n");
#endif

  return new_random_vector;
}

t_sizemat *sol_start_c_and_w(t_mat_cell **mat, t_sizemat n_columns) {
  DEBUG_PRINT("executing sol_start_c_and_w\n");

  t_mat_cell *sum_row_2d = prefix_sum_per_row_2d(mat, n_columns, n_columns);

#ifndef NDEBUG
  DPRINTF("cost matrix:\n");
  print_array_2d2(mat, n_columns, n_columns);
  DPRINTF("Prefix sum of matrix:\n");
  print_array_2d(sum_row_2d, n_columns, n_columns);
#endif

  t_sizemat *new_best_start_1d = generate_inc_vector(n_columns);

  for (t_sizemat i = 0; i < n_columns; i++) {
    t_sizemat best_pos = i;
    t_mat_cell best = 0;
    DPRINTF("sum for i=%ld : ", i);
    for (t_sizemat j = i; j < n_columns; j++) {
      t_sizemat sum_row_idx = new_best_start_1d[j];

      // current = mat[i][last] - mat[i][i]
      t_mat_cell last_sum = sum_row_2d[n_columns * sum_row_idx + n_columns - 1];
      t_mat_cell i_sum = sum_row_2d[n_columns * sum_row_idx + i];

      t_mat_cell current = last_sum - i_sum;

#ifndef NDEBUG
      printf("%ld ", current);
#endif

      if (current > best) {
        best_pos = j;
        best = current;
      }
    }
    DNPRINTF("\n");
    DPRINTF("best at %ld : %lu for value %lu\n", i, best_pos, best);

    t_sizemat tmp = new_best_start_1d[i];
    new_best_start_1d[i] = new_best_start_1d[best_pos];
    new_best_start_1d[best_pos] = tmp;

    DPRINTF("new_best_start_1d: ");
#ifndef NDEBUG
    print_array_1d(new_best_start_1d, n_columns);
#endif
  }

#ifndef NDEBUG
  DPRINTF("C_and_W solution\n");
  print_array_1d(new_best_start_1d, n_columns);
#endif

  free(sum_row_2d);
  return new_best_start_1d;
}

void neighb_modif(t_sizemat *new_sol_1d, const t_sizemat *sol_1d,
                  const t_sizemat *neighb_pot_1d, t_sizemat n_columns) {
  for (t_sizemat i = 0; i < n_columns; i++) {
    new_sol_1d[i] = sol_1d[neighb_pot_1d[i]];
  }
}

void pivot_first(unsigned long *cost, t_sizemat *new_sol_1d, t_sizemat *sol_1d,
                 struct neighb *neigh_pot, t_mat_cell **matrix) {
  DPRINTF("executing pivot_first\n");
}

void pivot_best(unsigned long *cost, t_sizemat *new_sol_1d, t_sizemat *sol_1d,
                struct neighb *neigh_pot, t_mat_cell **matrix) {
  DPRINTF("executing pivot_best\n");

  t_sizemat n_columns = neigh_pot->n_columns;
  *cost = 0;
  t_sizemat *best_neighb_modif;
  for (t_sizemat i = 0; i < neigh_pot->n_rows; i++) {

    t_sizemat *neighb_modi =
        &neigh_pot->neighborhood_modif_2d[neigh_pot->n_columns * i];

    neighb_modif(new_sol_1d, sol_1d, neighb_modi, n_columns);

    ulong new_cost = computeCost(new_sol_1d);

    if (new_cost >= *cost) {
      *cost = new_cost;
      best_neighb_modif = neighb_modi;
    }
  }

  neighb_modif(new_sol_1d, sol_1d, best_neighb_modif, n_columns);
}

void lop(t_fptr_sol_start fptr_sol_start, t_fptr_pivot_rule fptr_pivot_rule,
         t_fptr_neighborhood fptr_neighborhood) {
  DPRINTF("executing lop\n");

  t_mat_cell **cost_mat = CostMat;
  t_sizemat cost_mat_dim = PSize;

  // initial solution
  t_sizemat *sol_1d = fptr_sol_start(cost_mat, cost_mat_dim);

  // all possible modif to neighborhood
  struct neighb neigh_modif = fptr_neighborhood(cost_mat_dim);

  t_cost cost = computeCost(sol_1d);
  unsigned long new_cost = cost + 1;
  t_sizemat *new_sol_1d = malloc(cost_mat_dim * sizeof(t_sizemat));
  memcpy(new_sol_1d, sol_1d, cost_mat_dim * sizeof(t_sizemat));

  while (cost <= new_cost) {
    DPRINTF("lop while : cost=%lu and new_cost=%lu\n", cost, new_cost);

    DPRINTF("best sol: ");
    print_array_1d(new_sol_1d, cost_mat_dim);

    // TODO: cost and new_cost can have the same value. alows it but verify that
    // there is no comming back.
    cost = new_cost;
    memcpy(sol_1d, new_sol_1d, cost_mat_dim * sizeof(t_sizemat));

    fptr_pivot_rule(&new_cost, new_sol_1d, sol_1d, &neigh_modif, cost_mat);
    DPRINTF("lop while after pivot: cost=%lu and new_cost=%lu\n", cost,
            new_cost);
  }
  free(new_sol_1d);
  free(neigh_modif.neighborhood_modif_2d);
}
