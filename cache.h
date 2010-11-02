/**
 * cache.h
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#ifndef __CACHE__
#define __CACHE__

#define CACHE_BLOCKS 500
#define CACHE_CODE_SIZE 1024

typedef struct bBAS_BLOCK {
  int address;          // povodna adresa
  unsigned char *code;  // ?? 1K pre jeden blok
  int size;             // velkost povodnych instrukcii
 // struct bBAS_BLOCK *next;
} __attribute__((packed)) BASIC_BLOCK;

extern int cache_code_size;

void cache_init(int program_size);
void cache_destroy();
void cache_flush();
BASIC_BLOCK *cache_get_block(int address);
BASIC_BLOCK *cache_create_block(int address);


#endif
