#include "clocker.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <values.h>

static clock_t clock_list[MAXSHORT * 2];
unsigned short get_index_clock() {
  for (unsigned short i = 0; i < MAXSHORT * 2; ++i) {
    if (clock_list[i] == 0) {
      return i;
    }
  }
  assert(0);
  printf("Error: no more clock available\n");
  return 0;
}

unsigned short start_clock() {
  unsigned short index = get_index_clock();
  clock_list[index] = clock();
  return index;
}

double stop_clock(unsigned short index) {
  clock_t end = clock();
  double elapsed = (double)(end - clock_list[index]) / CLOCKS_PER_SEC;
  clock_list[index] = 0;
  return elapsed;
}
