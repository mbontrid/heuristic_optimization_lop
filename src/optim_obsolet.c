
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include "optimization.h"
#include "utilities.h"

#ifdef __MINGW32__
#include <float.h>
#define MAX_FLOAT FLT_MAX
#else
#define MAX_FLOAT MAXFLOAT
#endif

size_t *neighb_transpose_deltas(size_t *const n_rows, const size_t n_columns) {

  *n_rows = get_n_transpose(n_columns);
  size_t *const transposes_deltas_2d =
      malloc(*n_rows * n_columns * sizeof(size_t));
  assert(transposes_deltas_2d);

  DPRINTF("%zu transoposes possibilities allocated\n", *n_rows);

  for (ulong i = 0; i < *n_rows; i++) {
    for (size_t j = 0; j < n_columns; j++) {
      transposes_deltas_2d[n_columns * i + j] = j;
    }

    size_t temp = transposes_deltas_2d[n_columns * i + i];
    transposes_deltas_2d[n_columns * i + i] =
        transposes_deltas_2d[n_columns * i + (i + 1) % n_columns];
    transposes_deltas_2d[n_columns * i + (i + 1) % n_columns] =
        temp; // the % alows the transpose to go around the array.
  }

#ifndef NDEBUG
  DPRINTF("all transposes : \n");
  for (size_t i = 0; i < *n_rows; i++) {
    DPRINTF("transpose %zu : ", i)
    // PARRAY(&transposes_deltas_2d[n_columns * i], n_columns);
  }

  for (size_t i = 0; i < *n_rows; i++) {
    for (size_t j = 0; j < n_columns; j++) {
      assert(transposes_deltas_2d[n_columns * i + j] <= n_columns);
    }
  }
#endif
  return transposes_deltas_2d;
}
size_t *neighb_exchange_deltas(size_t *const n_rows, const size_t n_columns) {

  *n_rows = get_n_exchange(n_columns);
  size_t *restrict exchanges_deltas_2d =
      malloc(*n_rows * n_columns * sizeof(size_t));
  assert(exchanges_deltas_2d);

  DPRINTF("%zu exchanges possibilities allocated\n", *n_rows);

#pragma omp simd
  for (ulong i = 0; i < *n_rows * n_columns; i++) {
    exchanges_deltas_2d[i] = i % n_columns;
  }

  uint row = 0;
  for (size_t i = 0; i < n_columns - 1; i++) {
    for (size_t j = i + 1; j < n_columns; j++) {
      size_t temp = exchanges_deltas_2d[n_columns * row + i];
      exchanges_deltas_2d[n_columns * row + i] =
          exchanges_deltas_2d[n_columns * row + j];
      exchanges_deltas_2d[n_columns * row + j] = temp;
      row++;
    }
  }

  DPRINTF("executing for %zu collumns and %zu rows\n", n_columns, *n_rows);
#ifndef NDEBUG
  if (*n_rows < 100) {
    for (ulong i = 0; i < *n_rows; i++) {
      for (size_t j = 0; j < n_columns; j++) {
        printf("%zu ", exchanges_deltas_2d[n_columns * i + j]);
      }
      printf("\n");
    }
  }
#endif

  return exchanges_deltas_2d;
}

size_t *neighb_insert_deltas(size_t *const n_rows, const size_t n_columns) {

  *n_rows = get_n_inserts(n_columns);
  DPRINTF("executing for %zu collumns and %zu rows\n", n_columns, *n_rows);

  size_t *inserts_delta_2d = malloc(n_columns * *n_rows * sizeof(size_t));
  assert(inserts_delta_2d);
  DPRINTF("%zu inserts possibilities allocated\n", *n_rows);

  // only increment a row if condition completed, so no for loop possible for
  // the row.
  ulong row = 0;

  // which number has to move
  for (size_t i = 0; i < n_columns; i++) {
    // to which number i has to move
    for (size_t j = 0; j < n_columns; j++) {
      // avoid duplicates
      if (i != j && i != j + 1) {
        // go through each number of a row
        for (size_t x = 0; x < n_columns; x++) {
          // "move"" number to the left
          if (i < j && x >= i && x < j) {
            inserts_delta_2d[n_columns * row + x] = x + 1;
          } else if (i > j && x > j && x <= i) { // "move" number to the right
            inserts_delta_2d[n_columns * row + x] = x - 1;
          } else if (x == j) { // put i to j
            inserts_delta_2d[n_columns * row + x] = i;
          } else {
            inserts_delta_2d[n_columns * row + x] = x;
          }
        }
#ifndef NDEBUG
        if (n_columns < 100) {
          for (size_t x = 0; x < n_columns; x++) {
            printf("%zu ", inserts_delta_2d[n_columns * i + x]);
          }
          printf(" for i=%zu, j=%zu\n", i, j);
        }
#endif
        row++;
      }
    }
  }

  DPRINTF("for %zu collumns; total number of insert: %zu\n", n_columns,
          *n_rows);

  return inserts_delta_2d;
}

size_t *sol_start_cw_tentative(const t_mat_cell *const restrict cost_mat_2d,
                               size_t size) {
  DPRINTF("executting cw\n");

  t_cost *const restrict r_1d = calloc(size, sizeof(t_cost));
  t_cost *const restrict c_1d = calloc(size, sizeof(t_cost));
  Item *const restrict s_1d = malloc(size * sizeof(Item));
  assert(s_1d);

  size_t *restrict sol_1d = malloc(size * sizeof(size_t));
  assert(sol_1d);

/*
 * r[i] = sum j w_ij (i=/=j)
 * c[i] = sum j w_ji (i=/=j)
 * s[i] = r[i] - c[i]
 */
#pragma omp simd
  for (size_t x = 0; x < size * size; x++) {
    size_t i = x / size;
    size_t j = x % size;
    assert(MAX_COST_CELL - cost_mat_2d[size * i + j] >= r_1d[i]);
    r_1d[i] += cost_mat_2d[size * i + j];
    assert(MAX_COST_CELL - cost_mat_2d[size * j + i] >= c_1d[i]);
    c_1d[i] += cost_mat_2d[size * j + i];
  }

#pragma omp simd
  for (size_t i = 0; i < size; i++) {
    s_1d[i].value = r_1d[i] - c_1d[i] - 2 * cost_mat_2d[size * i + i];
    assert(s_1d[i].index <= size);
    s_1d[i].index = i;
  }

  qsort(s_1d, size, sizeof(Item), cmp_desc);

#pragma omp simd
  for (size_t i = 0; i < size; i++) {
    sol_1d[i] = s_1d[i].index;
  }

  DPRINTF("cw done, sol= ");
#ifndef NDEBUG
  print_array_1d((long int *)sol_1d, size);
#endif

  free(r_1d);
  free(c_1d);
  free(s_1d);
  return sol_1d;
}
