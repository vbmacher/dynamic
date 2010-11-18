/**
 * cache.c
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "cache.h"
#include "main.hpp"

static BASIC_BLOCK *cache;       /* Prekladova cache, to je zaklad. */
static int freeBlock = 0; /* index dalsieho volneho bloku*/

int cache_code_size;

/**
 * Funkcia inicializuje prekladovu cache - vytvori pamat pre N blokov.
 */
void cache_init(int program_size) {
  cache = (BASIC_BLOCK *)malloc(CACHE_BLOCKS * sizeof(BASIC_BLOCK));
  cache_flush();
  freeBlock = 0;
  cache_code_size = (program_size < CACHE_CODE_SIZE) ? CACHE_CODE_SIZE : program_size;
}

/**
 * Tato funkcia uvolni cache.
 */
void cache_destroy() {
  free(cache);
}

/**
 * Vyprazdni cache. Vyprazdnuje sa vtedy, ked je plna.
 */
void cache_flush() {
  memset(cache, 0, CACHE_BLOCKS * sizeof(BASIC_BLOCK));
  freeBlock = 0;
}

/**
 * Funkcia na zaklade adresy najde dany blok.
 *
 * @param address - Adresa povodneho programu, na ktorej ma zacinat blok.
 * @return NULL ak sa blok nenasiel, inak blok.
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
 * Tato funkcia vytvori blok, ak sa da.
 *
 * @param address - Adresa povodneho programu, na ktorej zacina blok
 * @return blok ak sa to podarilo, inak NULL.
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
