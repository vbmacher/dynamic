/**
 * bin.h
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#ifndef __BIN_HANDLER__
#define __BIN_HANDLER__

void *bin_load(const char *filename);
void bin_print(void *buffer);

extern int ram_size;

#endif
