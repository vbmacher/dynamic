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
  const char* code;
  int size;
} gen_code_struct;

#ifdef  __cplusplus
extern "C" {
#endif

void dyn_translate(BASIC_BLOCK *block, const char *program, int ram_size);

#ifdef  __cplusplus
}
#endif

#endif
