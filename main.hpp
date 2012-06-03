/**
 * main.hpp
 *
 * (c) Copyright 2010, P. Jakubco
 *
 */

#ifndef __MAIN__
#define __MAIN__

#define INPUT_CHARS 100

#define ERROR_MISSING_ARGS 1
#define ERROR_LOAD         2
#define ERROR_COMPILE      3
#define ERROR_LOGFILE      4
#define ERROR_INIT         5
#define ERROR_EXECUTE      6
#define OK                 0

#define CMD_SAVE_CODE      1
#define CMD_TEMPLATE       2
#define CMD_VERBOSE        4
#define CMD_SUMMARY        8
#define CMD_INTERPRET     16
#define CMD_COMPILE       32
#define CMD_COMPILE_ONLY  64
#define CMD_LOGTIME      128
#define CMD_LOOPS        256
#define CMD_OPENCL       512
#define CMD_DYNAMIC     1024
#define CMD_GPU         2048

extern int cmd_options;
extern char *code_filename;

#endif
