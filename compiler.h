/*
 * compiler.h
 *
 * (c) Copyright 2010, P. Jakubco
 */
 
#ifndef __COMPILER__
#define __COMPILER__

#include <stdio.h>

/* Symbols used by lexical analyzer */
#define TEXT          1
#define LABELTEXT     2
#define NUMBER        4
#define HALT          8
#define READ         16
#define WRITE        32
#define LOAD         64
#define STORE       128
#define ADD         256
#define SUB         512
#define MUL        1024
#define DIV        2048
#define JMP        4096
#define JGTZ       8192
#define JZ        16384
#define ASTERISK  32768
#define EQUAL     65536
#define COLON    131072
#define COMMENT  262144
#define END      524288

/* FIRST sets */
#define F_Instruction (HALT|READ|WRITE|LOAD|STORE|ADD|SUB|MUL|DIV|JMP|JGTZ|JZ)
#define F_Label       LABELTEXT
#define F_Row         (F_Label|F_Instruction)
#define F_Start       F_Row

/* FOLLOW sets */
#define FO_Start       END
#define FO_Row         (F_Start|FO_Start)
#define FO_Label       F_Instruction
#define FO_Instruction FO_Row

/* Error constants */
#define COMPILER_OK     0
#define COMPILER_ERROR  1

/* Compiler code generator codes */
#define C_HALT   0
#define C_READ   1
#define C_WRITE  4
#define C_LOAD   7
#define C_STORE  9
#define C_ADD   12
#define C_SUB   15
#define C_MUL   18
#define C_DIV   21
#define C_JMP   23
#define C_JGTZ  24
#define C_JZ    25

typedef unsigned long SET;

#ifdef  __cplusplus
extern "C" {
#endif

int compile(const char *input, const char* output);

#ifdef  __cplusplus
}
#endif

#endif
