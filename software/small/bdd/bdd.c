#include "types.h"
#include "memory_map.h"
#include "benchmark.h"
#include "ascii.h"
#include "uart.h"


#define N 4
#define M (2 * N)
#define NUMVAR (N * N)
#define ALLOC (2 * N * N * N - 3 * N * N - 2 * N + 7)
#define LOG2_1(n) (((n) >= 2) ? 1 : 0)
#define LOG2_2(n) (((n) >= 1<<2 ) ? (2  + LOG2_1((n)>>2 )) : LOG2_1(n))
#define LOG2_4(n) (((n) >= 1<<4 ) ? (4  + LOG2_2((n)>>4 )) : LOG2_2(n))
#define LOG2_8(n) (((n) >= 1<<8 ) ? (8  + LOG2_4((n)>>8 )) : LOG2_4(n))
#define LOG2(n)   (((n) >= 1<<16) ? (16 + LOG2_8((n)>>16)) : LOG2_8(n))
#define UTSIZE (1 << LOG2(ALLOC))
#define UTMASK (UTSIZE - 1)


uint16_t nObjs;            // number of used nodes
uint16_t pUnique[UTSIZE];  // unique table for nodes
uint8_t  pVars[ALLOC];     // array of variables for each node
uint16_t pObjs[2 * ALLOC]; // array of pairs <cof0, cof1> for each node
uint16_t pNexts[ALLOC];    // array of next pointers for each node
uint8_t  pMarks[ALLOC];    // array of marks for each node

uint16_t pOutputs[M];      // array of output nodes
uint16_t pBuf[2 * M];      // array of nodes
uint16_t pBuf2[2 * M];     // array of nodes
uint8_t  pSkip[2 * M];     // array of marks


uint16_t BddThen( uint16_t i )               { return pObjs[(i >> 1) << 1] ^ (i & 1);       }
uint16_t BddElse( uint16_t i )               { return pObjs[((i >> 1) << 1) + 1] ^ (i & 1); }
uint16_t BddMark( uint16_t i )               { return (int)pMarks[i >> 1];                  }
void     BddSetMark( uint16_t i, uint8_t m ) { pMarks[i >> 1] = m;                          }
uint16_t BddHash( uint8_t Arg0, uint16_t Arg1, uint16_t Arg2 ) {
  return (Arg0 << 7) + Arg0 + (Arg1 << 3) + Arg1 + (Arg2 << 1) + Arg2;
}
uint16_t BddUniqueCreateInt( uint8_t Var, uint16_t Then, uint16_t Else ) {
  uint16_t *q = pUnique + (BddHash( Var, Then, Else ) & UTMASK);
  for ( ; *q; q = pNexts + *q )
    if ( pVars[*q] == Var && pObjs[*q+*q] == Then && pObjs[*q+*q+1] == Else )
      return *q << 1;
  *q = nObjs++;
  pVars[*q] = Var;
  pObjs[*q+*q] = Then;
  pObjs[*q+*q+1] = Else;
  return *q << 1;
}
uint16_t BddUniqueCreate( uint8_t Var, uint16_t Then, uint16_t Else ) {
  if ( Then == Else ) return Else;
  if ( !(Else & 1) ) return BddUniqueCreateInt( Var, Then, Else );
  return BddUniqueCreateInt( Var, Then ^ 1, Else ^ 1 ) ^ 1;
}
uint16_t BddCount_stack( uint16_t i ) {
  // overwrite unique table to count nodes
  uint16_t count = 0, nstack = 0;
  pUnique[nstack++] = i;
  while ( nstack-- ) {
    i = pUnique[nstack];
    if ( i < 2 || BddMark( i ) ) continue;
    BddSetMark( i, 1 );
    count++;
    pUnique[nstack++] = BddElse( i );
    pUnique[nstack++] = BddThen( i );
  }
  return count;
}
void BddUnmark_rec( uint16_t i ) {
  if ( i < 2 || !BddMark( i ) ) return;
  BddSetMark( i, 0 );
  BddUnmark_rec(  BddElse( i) );
  BddUnmark_rec(  BddThen( i) );
}
void BddUnmark_stack( uint16_t i ) {
  uint16_t nstack = 0;
  pUnique[nstack++] = i;
  while ( nstack-- ) {
    i = pUnique[nstack];
    if ( i < 2 || !BddMark( i ) ) continue;
    BddSetMark( i, 0 );
    pUnique[nstack++] = BddElse( i );
    pUnique[nstack++] = BddThen( i );
  }
}
uint16_t BddCountNodesArray( uint16_t * vNodes, uint16_t nNodes ) {
  uint16_t i, Count = 0;
  for ( i = 0; i < nNodes; i++ )
    Count += BddCount_stack(  vNodes[i] );
  for ( i = 0; i < nNodes; i++ )
    BddUnmark_stack(  vNodes[i] );
  return Count + 1;
}
uint16_t gennode( uint8_t Var, uint16_t Then, uint16_t Else, uint8_t fUse ) {
  if ( !fUse ) return Else;
  return BddUniqueCreate( Var, Then, Else );
}


uint16_t floorlog2(uint16_t i) {
  uint16_t t = 0;
  i >>= 1;
  while ( i ) {
    i >>= 1;
    t++;
  }
  return t;
}


uint32_t bdd() {
  uint8_t fSkip, fUse;
  uint16_t nbuf, nbuf_next;
  uint16_t * buf = pBuf, * buf_next = pBuf2;
  uint16_t levelsize, levelsize_top, levelsize_topnext;
  uint8_t Var;
  uint8_t Var2 = N - 1;
  for(uint16_t m = M; m > 0; m--) {
    nbuf = nbuf_next = 0;
    buf[0] = 1;
    Var = NUMVAR - 1;
    for(uint16_t i = 1; i < m - 1; i++) { // column
      if(i < N) Var -= i;
      else Var -= M - i - 1;
      for(uint16_t j = 0; j <= i; j++) { // row
	fUse = (M - 1 - i >= N - j && j < N);
	if(j < i) {
	  levelsize = (m - i - 1 > floorlog2(i + j))? i + j: 1u << (m - i - 1);
          fSkip = 0;
	  if(i >= N && j == N - 1) {
	    // keep else only, the others can be skipped where dummy is inserted
            fSkip = 1;
	    levelsize_top = (m - i - 2 > floorlog2(i))? i: 1u << (m - i - 2);
	    levelsize_topnext = (m - i - 1 > floorlog2(i + i - 1))? i + i - 1: 1u << (m - i - 1);
	    if(levelsize_top << 1 == levelsize_topnext)
	      for(uint16_t k = 0; k < levelsize_topnext; k += 2)
		pSkip[k] = 1;
	    else {
	      pSkip[0] = 1;
	      for(uint16_t k = 1; k < levelsize_topnext; k += 2)
		pSkip[k] = 1;
	    }
	    uint16_t nshift = i - 1 - j;
	    while(levelsize_topnext < levelsize + nshift) {
	      uint8_t tmpc = pSkip[levelsize_topnext - 1];
	      for(uint16_t k = levelsize_topnext - 1; k > 0; k--)
		pSkip[k] = pSkip[k - 1];
	      pSkip[0] = tmpc;
	      nshift--;
	    }
	    while(nshift) {
	      pSkip[0] = pSkip[levelsize_topnext - 1];
              pSkip[levelsize_topnext - 1] = 0;
	      levelsize_topnext--;
	      nshift--;
	    }
	  }
	  for(uint16_t k = 0; k < levelsize; k++) {
	    if(fSkip && pSkip[k]) {
	      buf_next[nbuf_next++] = 0;
              pSkip[k] = 0;
	    }
	    else if(levelsize == nbuf) {
	      if(k == 0)
		buf_next[nbuf_next++] = gennode(Var - j, buf[k], buf[k + 1] ^ 1, fUse);
	      else if(k != levelsize - 1)
		buf_next[nbuf_next++] = gennode(Var - j, buf[k], buf[k + 1], fUse);
	      else
		buf_next[nbuf_next++] = gennode(Var - j, buf[k], buf[0], fUse);
	    }
	    else {
	      if(k == 0)
		buf_next[nbuf_next++] = gennode(Var - j, buf[k], 0, fUse);
	      else if(k == 1 && k != levelsize - 1)
		buf_next[nbuf_next++] = gennode(Var - j, 1, buf[k], fUse);
	      else if(k == 1)
		buf_next[nbuf_next++] = gennode(Var - j, 1, buf[0], fUse);
	      else if(k != levelsize - 1)
		buf_next[nbuf_next++] = gennode(Var - j, buf[k - 1], buf[k], fUse);
	      else
		buf_next[nbuf_next++] = gennode(Var - j, buf[k - 1], buf[0], fUse);
	    }
	  }
          uint16_t * tmpptr = buf;
	  buf = buf_next;
	  buf_next = tmpptr;
	  nbuf = nbuf_next;
	  nbuf_next = 0;
	}
	else {
	  levelsize = (m - i - 2 > floorlog2(i))? i: 1u << (m - i - 2);
	  for(uint16_t k = 0; k < levelsize; k++) {
	    if(levelsize << 1 == nbuf) {
	      if(k == 0)
		buf_next[nbuf_next++] = gennode(Var - j, buf[k + k], buf[k + k + 1] ^ 1, fUse);
	      else
		buf_next[nbuf_next++] = gennode(Var - j, buf[k + k], buf[k + k + 1], fUse);
	    }
	    else {
	      if(k == 0)
		buf_next[nbuf_next++] = gennode(Var - j, buf[k + k], 0, fUse);
	      else 
		buf_next[nbuf_next++] = gennode(Var - j, buf[k + k - 1], buf[k + k], fUse);
	    }
	  }
	  uint16_t * tmpptr = buf;
	  buf = buf_next;
	  buf_next = tmpptr;
	  nbuf = nbuf_next;
	  nbuf_next = 0;
	}
      }
    }
    for(uint16_t j = 0; j < m; j++) {
      fUse = (2 * N - m >= N - j && j < N);
      buf[0] = gennode(Var2 - j, buf[0], buf[0] ^ 1, fUse);
    }
    if(m <= N) Var2 += m - 1;
    else Var2 += M - m;
    if(m != 1)
      buf[0] = buf[0] ^ 1;
    pOutputs[m - 1] = buf[0];
  }
  return BddCountNodesArray( pOutputs, 2 * N );
}


#define CONST 0x4f

typedef void (*entry_t)(void);

int main(int argc, char**argv) {
  pVars[0]    = 0xff;
  nObjs       = 1;
  for(uint16_t i = 0; i < UTSIZE; i++)
    pUnique[i] = 0;
  for(uint16_t i = 0; i < ALLOC; i++) {
    pNexts[i] = 0;
    pMarks[i] = 0;
  }
  for(uint16_t i = 0; i < M + M; i++)
    pSkip[i] = 0;
  run_and_time(&bdd);
  csr_tohost(CONST);
  csr_tohost(0);
  // go back to the bios - using this function causes a jr to the addr,
  // the compiler "jals" otherwise and then cannot set PC[31:28]
  uint32_t bios = ascii_hex_to_uint32("40000000");
  entry_t start = (entry_t) (bios);
  start();
  return 0;
}
