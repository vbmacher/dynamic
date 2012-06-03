/**
 * cache.c
 *
 * (c) Copyright 2010, P. Jakubco
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "cache.h"
#include "main.hpp"

static BASIC_BLOCK *cache; /* Translation cache, this is the base. */
static int freeBlock = 0;  /* index od next free block  */

int cache_code_size;

/**
 * The function initializes the translation cache - it allocates memory for N
 * blocks.
 */
void cache_init(int program_size) {
  cache = (BASIC_BLOCK *)malloc(CACHE_BLOCKS * sizeof(BASIC_BLOCK));
  cache_flush();
  freeBlock = 0;
  cache_code_size = (program_size < CACHE_CODE_SIZE) ? CACHE_CODE_SIZE : program_size;
}

/**
 * The function frees the cache memory.
 */
void cache_destroy() {
  free(cache);
}

/**
 * The function flushes the cache. It is flushed only if it is full.
 */
void cache_flush() {
  memset(cache, 0, CACHE_BLOCKS * sizeof(BASIC_BLOCK));
  freeBlock = 0;
}

/**
 * The function finds a block based on address.
 *
 * @param address - The address of original program where the block should begin
 * @return NULL if the block is not found, the block otherwise.
 */
BASIC_BLOCK *cache_get_block(int address) {
  register int i;
  for (i = 0; i < freeBlock; i++) {
    if (cache[i].address == address)
      return &cache[i];
  }
  return NULL;
}

/**
 * This function creates block, if it is possible.
 *
 * @param address - The address of original program, where the block begins
 * @return block if the operation was successful, NULL otherwise.
 */
BASIC_BLOCK *cache_create_block(int address) {
  if (freeBlock >= CACHE_BLOCKS)
    return NULL;
  if (cmd_options & CMD_SUMMARY)
    printf("\t\tCreating block (freeBlock = %d)\n", freeBlock);
  cache[freeBlock].address = address;
  cache[freeBlock++].code = (unsigned char *)calloc(1, cache_code_size);
  if (cmd_options & CMD_SUMMARY)
    printf("\t\tBlock: code=%x, beg.=%x, PC addr.=%x, size=%d\n", 
      cache[freeBlock-1].code,&(cache[freeBlock-1]), cache[freeBlock-1].address,
       cache_code_size);
  return &cache[freeBlock-1];
}
