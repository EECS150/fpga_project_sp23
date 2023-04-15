#include "types.h"
#include "memory_map.h"
#include "benchmark.h"
#include "ascii.h"
#include "uart.h"

#define N 6

uint8_t x[N];
uint8_t y[N];
uint8_t route[N];
uint16_t min_d;
uint8_t tmp[N];
uint8_t used[N];
uint16_t d[N];

uint16_t distance(uint8_t i, uint8_t j) {
  uint16_t k = 0;
  k += (x[i] > x[j])? x[i] - x[j]: x[j] - x[i];
  k += (y[i] > y[j])? y[i] - y[j]: y[j] - y[i];
  return k;
}

uint32_t tspex() {
  tmp[1] = 0;
  d[0] = 0;
  min_d = 0xffff;
  for(uint8_t i = 0; i < N; i++)
    used[i] = 0;
  uint8_t m = 0;
  tmp[m] = 0;
  while(1) {
    if(m == 1) m--;
    used[tmp[m]] = 0;
    for(tmp[m]++; tmp[m] < N && used[tmp[m]]; tmp[m]++);
    if(tmp[m] == N) {
      if(!m) break;
      m--;
      continue;
    }
    if(m == 2 && tmp[m] < tmp[0])
      continue;
    used[tmp[m]] = 1;
    if(!m) m++;
    d[m] = d[m-1] + distance(tmp[m-1], tmp[m]);
    if(d[m] > min_d)
      continue;
    if(m == N - 1) {
      uint16_t new_d = d[N-1] + distance(tmp[N-1], tmp[0]);
      if(new_d < min_d) {
        min_d = new_d;
        for(uint8_t i = 0; i < N; i++)
          route[i] = tmp[i];
      }
      continue;
    }
    m++;
    tmp[m] = 0;
  }
  return min_d;
}

void init_graph() {
  uint16_t z = 1;
  for(uint16_t i = 0; i < N; i++) {
    x[i] = z;
    z ^= z << 7;
    z ^= z >> 5;
    z ^= z << 3;
    y[i] = z;
    z ^= z << 7;
    z ^= z >> 5;
    z ^= z << 3;
  }
}

#define CONST 0x000002a0

typedef void (*entry_t)(void);

int main() {
  init_graph();
  run_and_time(&tspex);
  csr_tohost(CONST);
  csr_tohost(0);
  // go back to the bios - using this function causes a jr to the addr,
  // the compiler "jals" otherwise and then cannot set PC[31:28]
  uint32_t bios = ascii_hex_to_uint32("40000000");
  entry_t start = (entry_t) (bios);
  start();
  return 0;
}
