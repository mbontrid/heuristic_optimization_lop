
#include <assert.h>
#include <stdlib.h>

#include "memetic.h"
#include "optimization.h"
#include "utilities.h"

size_t *memetic(size_t **population, size_t size_pop, size_t size) {}

void dpx_crossover(size_t *p1_offspring, size_t *p2, size_t size) {
  assert(p1_offspring);
  assert(p2);

  for (size_t i = 0; i < size; i++) {
    if (p1_offspring[i] != p2[i]) {
    }
  }
}

size_t rand_swap(size_t *array, size_t size, size_t i) {
  assert(i < size);
  assert(array);
  size_t j = randInt(0, size - 1);
  swap(array, i, j);
  return j;
}

void mutate(size_t *array, size_t size, float rate) {
  assert(rate <= 1 || rate >= 0);
  assert(array);
  for (size_t i = 0; i < size; i++) {
    if ((float)ran01(&Seed) < rate) {
      size_t j = rand_swap(array, size, i);
    }
  }
}
