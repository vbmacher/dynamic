/**
 * main.c
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 * How to build
 * ^^^^^^^^^^^^
 *   - do not use code align
 *   - all microcode functions must have only 1 return instruction
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "bin.h"
#include "ram.h"
#include "cache.h"
#include "dynarec.h"

#define INPUT_CHARS 100

#define ERROR_MISSING_ARGS 1
#define ERROR_LOAD 2
#define OK 0


static unsigned char *program; // loaded program, corresponds with operating memory

int main(int argc, char *argv[])
{
  int status;
  clock_t start, end;
  BASIC_BLOCK *tmp;
  char input_str[INPUT_CHARS];
  
  printf("dynamic 0.1b\n\tDynamic Translator and emulator of RAM programs\n\n");

  if (argc < 2) {
    printf("Usage: dynamic [path/to/ram/program]\n");
    return ERROR_MISSING_ARGS;
  }

  /* load RAM program */
  program = (unsigned char *)bin_load(argv[1]);

  if (program == NULL)
    return ERROR_LOAD;

  printf("\nEmulating program:\n");
  bin_print(program);

  printf("Initializing...\n");
  ram_init(INPUT_CHARS);
  cache_init();
  
  printf("Enter input tape (max. %d chars):", INPUT_CHARS);
  scanf("%s", input_str);

  int i,l=strlen(input_str);
  for (i = 0; i < l; i++)
    ram_env.input[i] = input_str[i]-'0';

  printf("Interpreting...\n");
  
  start = clock();
  while ((status = ram_interpret(program)) == RAM_OK)
    ;
  end = clock();

  printf("\nDone.\n\tError code: %d (%s)\n\tTime: %lf\n", status,
    ram_error(status), (double)((double)end-(double)start));

  printf("\nOutput tape:\n\t");
  ram_output();
  
  ram_destroy();
  ram_init(INPUT_CHARS);
  printf("Enter input tape (max. %d chars):", INPUT_CHARS);
  scanf("%s", ram_env.input);
  
  l=strlen(input_str);
  for (i = 0; i < l; i++)
    ram_env.input[i] = input_str[i]-'0';

  printf("Emulating using dynamic translation...\n");
  start = clock();

  do {    
    tmp = cache_get_block(ram_env.pc);
    if (tmp == NULL) {
      tmp = cache_create_block(ram_env.pc);
      if (tmp == NULL) {
        cache_flush();
        tmp = cache_create_block(ram_env.pc);
      }
      dyn_translate(tmp, program);
    } else if (tmp->size) {
      (*(void (*)())tmp->code)();
      ram_env.pc += tmp->size;
    } else
      ram_env.state = ram_interpret(program);
  } while ((ram_env.state == RAM_OK) && (ram_env.pc < ram_size));
  end = clock();

  printf("\nDone.\n\tError code: %d (%s)\n\tTime: %lf\n", status,
    ram_error(status), (double)((double)end-(double)start));
  
  printf("\nOutput tape:\n\t");
  ram_output();
  
  cache_destroy();
  ram_destroy();
  
  return OK;
}
