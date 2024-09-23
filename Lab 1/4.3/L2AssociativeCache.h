#ifndef SIMPLECACHE_H
#define SIMPLECACHE_H

#define L2_WAYS 2
#define NUM_SETS (L2_SIZE/(BLOCK_SIZE * L2_WAYS))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../Cache.h"

void resetTime();

uint32_t getTime();


/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void accessL1(uint32_t, uint8_t *, uint32_t);
void accessL2(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint32_t Time;
  uint8_t Block_Data[BLOCK_SIZE];
} CacheLine;

typedef struct Set_Blocks{
  uint32_t init;
  CacheLine lines[L2_WAYS];
} Set_Blocks;

typedef struct L1_Cache {
  uint32_t init;
  CacheLine lines[L1_SIZE/BLOCK_SIZE];
} L1_Cache;

typedef struct L2_Cache {
  uint32_t init;
  Set_Blocks ways[NUM_SETS];
} L2_Cache;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
