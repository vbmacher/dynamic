/**
 * cache.h
 *
 * (c) Copyright 2010, P. Jakubco
 *
 */

#ifndef __CACHE__
#define __CACHE__

#define CACHE_BLOCKS 500
#define CACHE_CODE_SIZE 1024

typedef struct bBAS_BLOCK {
  int address;          // original address
  unsigned char *code;  // ?? 1K for single block
  int size;             // the size of original instructions
 // struct bBAS_BLOCK *next;
} __attribute__((packed)) BASIC_BLOCK;

extern int cache_code_size;

#ifdef  __cplusplus
extern "C" {
#endif

void cache_init(int program_size);
void cache_destroy();
void cache_flush();
BASIC_BLOCK *cache_get_block(int address);
BASIC_BLOCK *cache_create_block(int address);

#ifdef  __cplusplus
}
#endif

#endif
