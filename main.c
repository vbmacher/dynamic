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
#include <getopt.h>

#include "bin.h"
#include "ram.h"
#include "cache.h"
#include "dynarec.h"
#include "compiler.h"

#include "main.h"

static struct option options[] =	{{"help", no_argument, NULL, 'h' },
                           {"save-code", optional_argument, NULL, 'S'},
                           {"verbose", no_argument, NULL, 'v'},
                           {"summary", no_argument, NULL, 's'},
                           {"interpret", no_argument, NULL, 'i'},
                           {"compile", required_argument, NULL, 'c'},
                           {"compile-only", no_argument, NULL, 'C'},
                           {0,0,0,0}};
int cmd_options;
char *code_filename;
char *input_filename;

static unsigned char *program; // loaded program, corresponds with operating memory

int main(int argc, char *argv[])
{
  int status;
  clock_t start, end;
  BASIC_BLOCK *tmp;
  char input_str[INPUT_CHARS];
  int opt_index, opt;
  
  code_filename = NULL;
  while(1) {
    opt = getopt_long(argc, argv, "hS::vsic:C", options, &opt_index);
    
    if (opt == -1)
      break;
      
    switch(opt) {
      case 'h':
        printf("dynamic 0.21b\nDynamic Translator and emulator of RAM programs\n\n" \
        "Usage: dynamic [hS::vsic:C] path/to/ram/program\n\n" \
        "Options:\n" \
        "\t-h --help                       - This help screen\n" \
        "\t-S --save-code [[filename_base]]- Lets the dynamic to save generated\n" \
        "\t                                  code to file(s).\n" \
        "\t-v --verbose                    - Hides all output (prints only the\n" \
        "\t                                  results from the output tape).\n" \
        "\t-s --summary                    - Prints summary information while\n" \
        "\t                                  performing the translation.\n" \
        "\t-i --interpret                  - Perform interpretation.\n" \
        "\t-c --compile [source_file]      - Compile a source file into the output file.\n" \
        "\t-C --compile-only               - Do not perform emulation after compile.\n\n");
//        "\t-p --parse-only                 - Do not compile, but parse only\n\n");
        return 0;
      case 'S':
        cmd_options |= CMD_SAVE_CODE;
        if (optarg)
          code_filename = optarg;
        break;
      case 'v':
        cmd_options |= CMD_VERBOSE;
        break;
      case 's':
        cmd_options |= CMD_SUMMARY;
        break;
      case 'i':
        cmd_options |= CMD_INTERPRET;
        break;
      case 'c':
        cmd_options |= CMD_COMPILE;
        input_filename = optarg;
        break;
      case 'C':
        cmd_options |= CMD_COMPILE_ONLY;
      default: opt = -1; break;
    }
    if (opt == -1)
      break;
  }

  if (optind >= argc) {
    printf("Error: Missing RAM program file name\n\n(Type 'dynamic --help' for help)\n");
    return ERROR_MISSING_ARGS;
  }

  /* first - compile */
  if (cmd_options & CMD_COMPILE) {
    printf("Compiling file '%s'...\n", input_filename);
    if (compile(input_filename, argv[optind])) {
      printf("\nFix the errors and try again!\n");
      return ERROR_COMPILE;
    } else
      printf("\nCompile runned OK\n");
    
    if (cmd_options & CMD_COMPILE_ONLY)
      return OK;
  }

  /* load RAM program */
  if (cmd_options & CMD_SUMMARY)
    printf("Loading file: %s\n", argv[optind]);
  program = (unsigned char *)bin_load(argv[optind]);

  if (program == NULL) {
    if (cmd_options & CMD_SUMMARY)
      printf("Error loading file: %s\n", argv[optind]);
    return ERROR_LOAD;
  }

  if (cmd_options & CMD_SUMMARY) {
    printf("\nEmulating program:\n");
    bin_print(program);
  }

  if (cmd_options & CMD_SUMMARY)
    printf("Initializing...\n");
  ram_init(INPUT_CHARS);
  cache_init();
  
  if (!(cmd_options & CMD_VERBOSE))
    printf("Enter input tape (max. %d chars):", INPUT_CHARS);
  scanf("%s", input_str);

  // convert the input tape into binary form
  int i,l=strlen(input_str);
  for (i = 0; i < l; i++)
    ram_env.input[i] = input_str[i]-'0';

  if (cmd_options & CMD_INTERPRET) {
    if (!(cmd_options & CMD_VERBOSE))
      printf("Interpreting...\n");
  
    start = clock();
    while ((status = ram_interpret(program)) == RAM_OK)
      ;
    end = clock();

    if (!(cmd_options & CMD_VERBOSE))
      printf("\nDone.\n\tError code: %d (%s)\n\tTime: %lf\n", status,
        ram_error(status), (double)((double)end-(double)start));

    if (!(cmd_options & CMD_VERBOSE))
      printf("\nOutput tape:\n\t");
    ram_output();
  
    char *input = ram_env.input;
    ram_init(INPUT_CHARS);
    strcpy(ram_env.input, input);
    free(input);
  }
  
  if (!(cmd_options & CMD_VERBOSE))
    printf("Emulating using dynamic translation...\n");

  start = clock();
  do {    
    tmp = cache_get_block(ram_env.pc);
    if (tmp == NULL) {
//      if (cmd_options & CMD_SUMMARY)
  //      printf("\tTranslating (PC=%d)...\n", ram_env.pc);
      if ((tmp = cache_create_block(ram_env.pc)) == NULL) {
    //    if (cmd_options & CMD_SUMMARY)
      //    printf("\t\tFlushing cache...\n");
        cache_flush();
        tmp = cache_create_block(ram_env.pc);
      }
      dyn_translate(tmp, program);
    } else if (tmp->size > 0) {
//      if (cmd_options & CMD_SUMMARY)
  //      printf("\tExecuting code (PC=%d)...\n", tmp->address);
      (*(void (*)())tmp->code)();
      ram_env.pc += tmp->size;
    } else {
    //  if (cmd_options & CMD_SUMMARY)
      //  printf("\tInterpreting (PC=%d)...\n", ram_env.pc);
      ram_env.state = ram_interpret(program);
    }
  } while ((ram_env.state == RAM_OK) && (ram_env.pc < ram_size));
  end = clock();
  
  if (!(cmd_options & CMD_VERBOSE))
    printf("\nDone.\n\tError code: %d (%s)\n\tTime: %lf\n", ram_env.state,
      ram_error(ram_env.state), (double)((double)end-(double)start));
  
  if (!(cmd_options & CMD_VERBOSE))
    printf("\nOutput tape:\n\t");
  ram_output();
  
  cache_destroy();
  ram_destroy();
  free(program);

  return OK;
}
