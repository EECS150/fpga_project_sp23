#include "types.h"
#include "memory_map.h"
#include "benchmark.h"
#include "ascii.h"
#include "uart.h"

#define N 60

uint16_t x[N];

uint32_t selection_sort() {
  uint32_t count = 0;
  for(uint16_t i = 0; i < N; i++) {
    uint16_t min_idx = i;
    for(uint16_t j = i + 1; j < N; j++)
      if(x[j] < x[min_idx])
        min_idx = j, count++;
    uint16_t tmp = x[i];
    x[i] = x[min_idx];
    x[min_idx] = tmp;
  }
  return count;
}

void init_array() {
  uint16_t y = 1;
  for(uint16_t i = 0; i < N; i++) {
    x[i] = y;
    y ^= y >> 7;
    y ^= y << 9;
    y ^= y >> 13;
  }
}

#define CONST 0x00000081

typedef void (*entry_t)(void);

int main() {
  init_array();
  run_and_time(&selection_sort);
  csr_tohost(CONST);
  csr_tohost(0);
  // go back to the bios - using this function causes a jr to the addr,
  // the compiler "jals" otherwise and then cannot set PC[31:28]
  uint32_t bios = ascii_hex_to_uint32("40000000");
  entry_t start = (entry_t) (bios);
  start();
  return 0;
}
