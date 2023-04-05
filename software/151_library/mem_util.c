#include "types.h"

void * memmove(void * dst, const void * src, uint32_t n) {
  if((uint32_t)src < (uint32_t)dst)
    for(uint32_t i = n; i > 0; i--)
      ((uint8_t*)dst)[i-1] = ((uint8_t*)src)[i-1];
  else if((uint32_t)dst < (uint32_t)src)
    for(uint32_t i = 0; i < n; i++)
      ((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
}

void * memset(void * ptr, int val, uint32_t n) {
  if(val) {
    val &= 0xff;
    val = (val << 8) | val;
    val = (val << 16) | val;
  }
  while((((uint32_t)ptr) & 3) && n) {
    *((uint8_t*)ptr) = (uint8_t)val;
    ptr = (void *)((uint32_t)ptr + 1);
    n--;
  }
  while(n > 3) {
    *((uint32_t*)ptr) = val;
    ptr = (void *)((uint32_t)ptr + 4);
    n -= 4;
  }
  while(n) {
    *((uint8_t*)ptr) = (uint8_t)val;
    ptr = (void *)((uint32_t)ptr + 1);
    n--;
  }
}
