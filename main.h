/**
 * main.h
 *
 * (c) Copyright 2010, P. Jakub�o
 *
 */

#ifndef __MAIN__
#define __MAIN__

#define INPUT_CHARS 100

#define ERROR_MISSING_ARGS 1
#define ERROR_LOAD 2
#define OK 0

#define CMD_SAVE_CODE 1
#define CMD_TEMPLATE 2
#define CMD_VERBOSE 4
#define CMD_SUMMARY 8
#define CMD_INTERPRET 16

extern int cmd_options;
extern char *code_filename;

#endif