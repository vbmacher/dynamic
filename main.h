/**
 * main.h
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#ifndef __MAIN__
#define __MAIN__

#define INPUT_CHARS 100

#define ERROR_MISSING_ARGS 1
#define ERROR_LOAD         2
#define ERROR_COMPILE      3
#define OK                 0

#define CMD_SAVE_CODE      1
#define CMD_TEMPLATE       2
#define CMD_VERBOSE        4
#define CMD_SUMMARY        8
#define CMD_INTERPRET     16
#define CMD_COMPILE       32
#define CMD_COMPILE_ONLY  64
#define CMD_LOGTIME      128

extern int cmd_options;
extern char *code_filename;

#endif
