/**
 * dynarec.h
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#ifndef __DYNAREC__
#define __DYNAREC__

#include "cache.h"

typedef struct {
  unsigned char* code;
  int size;
} gen_code_struct;

void dyn_translate(BASIC_BLOCK *block, unsigned char *program);

#endif
