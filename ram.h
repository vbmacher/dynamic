/**
 * ram.h
 *
 * (c) Copyright 2010, P. Jakubco
 */
 
#ifndef __RAM_ENV__
#define __RAM_ENV__
 
#define RAM_OK 0
#define RAM_UNKNOWN_INSTRUCTION 1
#define RAM_ADDRESS_FALLOUT 2
#define RAM_HALT 3
#define RAM_OUTPUT_FULL 4
#define RAM_DIVISION_BY_ZERO 5

#define RAM_OUTPUT_SIZE 100
#define RAM_REGISTERS_COUNT 100

typedef struct {
  unsigned int pc;            /* program counter */
  unsigned int r[RAM_REGISTERS_COUNT]; /* registers */
  char *input;                /* input tape */
  char output[RAM_OUTPUT_SIZE];  /* output tape, max. 100 chars */
  int p_input;              /* pointer to the next cell within the input tape */
  int p_output;            /* pointer to the next cell within the output tape */
  int state;                /* state of the RAM machine, used only in dyntran */
} RAM_env;

#ifdef  __cplusplus
extern "C" {
#endif

void ram_init(int size);
void ram_destroy();
int ram_interpret(const char *program, int ram_size);
const char *ram_error(int error_code);
void ram_output();

#ifdef  __cplusplus
}
#endif

extern RAM_env ram_env;

#endif
