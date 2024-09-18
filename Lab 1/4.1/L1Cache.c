#include "L1Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache cache;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 cache *************************/

void initCache() { cache.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, offset;
  uint8_t TempBlock[BLOCK_SIZE];
  
  /* init cache */
  if (cache.init == 0) {
    for (int i = 0; i < (L1_SIZE / BLOCK_SIZE); i++) {
      cache.lines[i].Valid = 0;
      cache.lines[i].Tag = 0;
      cache.lines[i].Dirty = 0;
      for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
        cache.lines[i].Block_Data[j] = 0;
      }
    }
    cache.init = 1;
  }
  
  offset = address % BLOCK_SIZE;
  index = (address / BLOCK_SIZE) % (L1_SIZE / BLOCK_SIZE);
  Tag = address / L1_SIZE;
  
  CacheLine *Line = &cache.lines[index];

  /*access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {   // if block not present - miss
    accessDRAM(address - offset, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      accessDRAM(Line->Tag - offset, Line->Block_Data, MODE_WRITE); // then write back old block
    }

    memcpy(Line->Block_Data, TempBlock, BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(Line->Block_Data[offset]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(Line->Block_Data[offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}

