#include "L2AssociativeCache.h"

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

  for (int i = 0; i < (L1_SIZE / BLOCK_SIZE); i++) {
    L1.lines[i].Valid = 0;
    L1.lines[i].Tag = 0;
    L1.lines[i].Dirty = 0;
    for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
      L1.lines[i].Block_Data[j] = 0;
    }
  }
  L1.init = 1;
  
  for (int i = 0; i < NUM_SETS; i++) {
      for (int way = 0; way < L2_WAYS; way++){
          L2.ways[i].lines[way].Valid = 0;
          L2.ways[i].lines[way].Tag = 0;
          L2.ways[i].lines[way].Dirty = 0;
          L2.ways[i].lines[way].Time = 0;
          for (int j = 0; j < BLOCK_SIZE; j += WORD_SIZE) {
              L2.ways[i].lines[way].Block_Data[j] = 0;
          }
      }
      L2.ways[i].init = 1;            
  }
  L2.init = 1;
}

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index_L1, Tag_L1, offset_L1;
  
  offset_L1 = address % BLOCK_SIZE;
  index_L1 = (address / BLOCK_SIZE) % (L1_SIZE / BLOCK_SIZE);
  Tag_L1 = address / L1_SIZE;

  CacheLine *Line_L1 = &L1.lines[index_L1];

  //access L1 Cache

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
    uint32_t hit = 0, correct_way;

    offset_L2 = address % BLOCK_SIZE;
    index_L2 =  (address / BLOCK_SIZE) % NUM_SETS;
    Tag_L2 = address / (L2_SIZE / L2_WAYS);

    //access L2 Cache

    for (int way = 0; way < L2_WAYS; way++){ // Check for hit
      CacheLine *Line_L2 = &L2.ways[index_L2].lines[way];
  
      if(Line_L2->Valid && Line_L2->Tag == Tag_L2) {
        hit = 1;
        correct_way = way;
        break;
      }
    } 
    
    
    if(!hit){ // In case of miss
  
      uint32_t LRU_time = L2.ways[index_L2].lines[0].Time;
      uint32_t LRU_way = 0;

      // Least Recently Used(LRU) policy
      for (int way = 1; way < L2_WAYS; way++){
          CacheLine *Line_L2 = &L2.ways[index_L2].lines[way];
          if(Line_L2->Time < LRU_time){
            LRU_time = Line_L2->Time;
            LRU_way = way;
          } 
      }
      correct_way = LRU_way;
      CacheLine *Line_L2 = &L2.ways[index_L2].lines[LRU_way];
      
      if ((Line_L2->Valid) && (Line_L2->Dirty)) { // Line has dirty block
          accessDRAM(address - offset_L2, Line_L2->Block_Data, MODE_WRITE); // then write back old block
      }

      accessDRAM(address - offset_L2, TempBlock, MODE_READ); // get new block from DRAM

      memcpy(Line_L2->Block_Data, TempBlock, BLOCK_SIZE); // copy new block to cache line
      
      Line_L2->Valid = 1;
      Line_L2->Tag = Tag_L2;
      Line_L2->Dirty = 0;
    }
   
    CacheLine *Line_L2 = &L2.ways[index_L2].lines[correct_way];
    
    if (mode == MODE_READ) {    // read data from cache line
      memcpy(data, &(Line_L2->Block_Data[offset_L2]), WORD_SIZE);
      time += L2_READ_TIME;
    }

    if (mode == MODE_WRITE) { // write data from cache line
      memcpy(&(Line_L2->Block_Data[offset_L2]), data, WORD_SIZE);
      time += L2_WRITE_TIME;
      Line_L2->Dirty = 1;
    }
    
    Line_L2->Time = getTime();
    
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}

