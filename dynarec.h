/**
 * dynarec.h
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#ifndef __DYNAREC__
#define __DYNAREC__

#include "cache.h"

void dyn_translate(BASIC_BLOCK *block, unsigned char *program);
void dyn_template(BASIC_BLOCK *block, unsigned char *program);

#endif
