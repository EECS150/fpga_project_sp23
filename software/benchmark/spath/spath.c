#include "types.h"
#include "memory_map.h"
#include "benchmark.h"
#include "ascii.h"
#include "uart.h"

#define LOG2N 11
#define N (1 << LOG2N)

uint16_t amat[N][LOG2N];
uint16_t dist[N];
uint16_t loc[N];
uint16_t heap[N];
uint16_t heapsize = 0;

void upheapify(uint16_t i) {
  if(!i) return;
  uint16_t parent = (i - 1) >> 1;
  while(dist[heap[parent]] > dist[heap[i]]) {
    loc[heap[i]] = parent;
    loc[heap[parent]] = i;
    uint16_t tmp = heap[parent];
    heap[parent] = heap[i];
    heap[i] = tmp;
    i = parent;
    parent = (i - 1) >> 1;
  }
}
void downheapify(uint16_t i) {
  uint16_t smallest = i;
  uint16_t left = i + i + 1;
  uint16_t right = i + i + 2;
  if(left < heapsize && dist[heap[smallest]] > dist[heap[left]])
    smallest = left;
  if(right < heapsize && dist[heap[smallest]] > dist[heap[right]])
    smallest = right;
  while(i != smallest){
    loc[heap[i]] = smallest;
    loc[heap[smallest]] = i;
    uint16_t tmp = heap[smallest];
    heap[smallest] = heap[i];
    heap[i] = tmp;
    i = smallest;
    left = i + i + 1;
    right = i + i + 2;
    if(left < heapsize && dist[heap[smallest]] > dist[heap[left]])
      smallest = left;
    if(right < heapsize && dist[heap[smallest]] > dist[heap[right]])
      smallest = right;
  }
}
void insert(uint16_t a){
  heap[heapsize++] = a;
  upheapify(heapsize - 1);
}
uint16_t extract() {
  uint16_t a = heap[0];
  heap[0] = heap[--heapsize];
  downheapify(0);
  return a;
}

uint32_t spath() {
  uint16_t s = 0;
  dist[s] = 0;
  for(uint16_t i = 0; i < s; i++) {
    dist[i] = 0xffff;
    heap[i] = i;
    loc[i] = i;
  }
  for(uint16_t i = s + 1; i < N; i++) {
    dist[i] = 0xffff;
    heap[i - 1] = i;
    loc[i] = i - 1;
  }
  heapsize = N - 1;
  uint16_t i = s;
  while(1) {
    for(uint16_t j = 0; j < LOG2N; j++) {
      if(dist[amat[i][j]] > dist[i]) {
        dist[amat[i][j]] = dist[i] + 1;
        upheapify(loc[amat[i][j]]);
        downheapify(loc[amat[i][j]]);
      }
    }
    if(!heapsize) break;
    i = extract();
  }
  uint32_t count = 0;
  for(uint16_t i = 0; i < N; i++)
    count += dist[i];
  return count;
}

void init_graph() {
  uint16_t mask = N - 1;
  uint16_t y = 1;
  for(uint16_t i = 0; i < N; i++) {
    for(uint16_t j = 0; j < LOG2N; j++) {
      amat[i][j] = y & mask;
      y ^= y >> 7;
      y ^= y << 9;
      y ^= y >> 13;
    }
  }
}

#define CONST 0x00001b79

typedef void (*entry_t)(void);

int main() {
  init_graph();
  run_and_time(&spath);
  csr_tohost(CONST);
  csr_tohost(0);
  // go back to the bios - using this function causes a jr to the addr,
  // the compiler "jals" otherwise and then cannot set PC[31:28]
  uint32_t bios = ascii_hex_to_uint32("40000000");
  entry_t start = (entry_t) (bios);
  start();
  return 0;
}
