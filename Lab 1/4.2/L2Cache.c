#include "L2Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1_Cache L1; 
L2_Cache L2;

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

/*********************** Caches *************************/

void initCache() { 
    L1.init = 0;
    L2.init = 0;
}

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index_L1, Tag_L1, offset_L1;
  
  /* init both caches */
  if (L1.init == 0) {
    for (int i = 0; i < (L1_SIZE / BLOCK_SIZE); i++) {
      L1.lines[i].Valid = 0;
      L1.lines[i].Tag = 0;
      L1.lines[i].Dirty = 0;
      for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
        L1.lines[i].Block_Data[j] = 0;
      }
    }
    L1.init = 1;
  }

  if (L2.init == 0) {
    for (int i = 0; i < (L2_SIZE / BLOCK_SIZE); i++) {
      L2.lines[i].Valid = 0;
      L2.lines[i].Tag = 0;
      L2.lines[i].Dirty = 0;
      for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
        L2.lines[i].Block_Data[j] = 0;
      }
    }
    L2.init = 1;
  }
  
  offset_L1 = address % BLOCK_SIZE;
  index_L1 = (address / BLOCK_SIZE) % (L1_SIZE / BLOCK_SIZE);
  Tag_L1 = address / L1_SIZE;

  CacheLine *Line_L1 = &L1.lines[index_L1];

  /*access L1 Cache*/

  if (!Line_L1->Valid || Line_L1->Tag != Tag_L1) {   // if block not present - miss
    accessL2(address, Line_L1->Block_Data, MODE_READ); // get new block from L2

    if ((Line_L1->Valid) && (Line_L1->Dirty)) { // line has dirty block
      accessL2(address, Line_L1->Block_Data, MODE_WRITE); // then write back old block
    }

    Line_L1->Valid = 1;
    Line_L1->Tag = Tag_L1;
    Line_L1->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(Line_L1->Block_Data[offset_L1]), WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(Line_L1->Block_Data[offset_L1]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line_L1->Dirty = 1;
  }
}

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index_L2, Tag_L2, offset_L2;
  uint8_t TempBlock[BLOCK_SIZE];

  offset_L2 = address % BLOCK_SIZE;
  index_L2 = (address / BLOCK_SIZE) % (L2_SIZE / BLOCK_SIZE);
  Tag_L2 = address / L2_SIZE;

  
  CacheLine *Line_L2 = &L2.lines[index_L2];

  /*access L2 Cache*/

  if (!Line_L2->Valid || Line_L2->Tag != Tag_L2) {   // if block not present - miss
    accessDRAM(address - offset_L2, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line_L2->Valid) && (Line_L2->Dirty)) { // Line has dirty block
      accessDRAM(address - offset_L2, Line_L2->Block_Data, MODE_WRITE); // then write back old block
    }

    memcpy(Line_L2->Block_Data, TempBlock, BLOCK_SIZE); // copy new block to cache line
    Line_L2->Valid = 1;
    Line_L2->Tag = Tag_L2;
    Line_L2->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(Line_L2->Block_Data[offset_L2]), WORD_SIZE);
    time += L2_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(Line_L2->Block_Data[offset_L2]), data, WORD_SIZE);
    time += L2_WRITE_TIME;
    Line_L2->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}

