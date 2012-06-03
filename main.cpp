/**
 * main.cpp
 *
 * (c) Copyright 2010, P. Jakubco
 *
 * How to build
 * ^^^^^^^^^^^^
 *   - do not use code align
 *   - all microcode functions must have only 1 return instruction
 */
 
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <getopt.h>

#include "bin.hpp"
#include "ram.h"
#include "cache.h"
#include "dynarec.h"
#include "compiler.h"
#include "clprogram.hpp"
#include "emucl.hpp"
#include "main.hpp"

static struct option options[] =	{{"help", no_argument, NULL, 'h' },
                           {"save-code", optional_argument, NULL, 'S'},
                           {"verbose", no_argument, NULL, 'v'},
                           {"summary", no_argument, NULL, 's'},
                           {"interpret", no_argument, NULL, 'i'},
                           {"compile", required_argument, NULL, 'c'},
                           {"compile-only", no_argument, NULL, 'C'},
                           {"log-time",optional_argument,NULL,'l'},
                           {"loop", required_argument, NULL, 'L'},
                           {"open-cl", no_argument, NULL, 'o'},
                           {"dynamic", no_argument, NULL, 'd'},
                           {"gpu", no_argument, NULL, 'g'},
                           {0,0,0,0}};
int cmd_options;
char *code_filename;
char *input_filename;

static RAMBin bin;

int main(int argc, char *argv[])
{
  int status;
  clock_t start, end;
  double overall_time = 0;
  int log_iter = -1; // log time iteration
  int log_counter = 0;
  int loops = 0;
  FILE *flog;
  
  BASIC_BLOCK *tmp;
  char input_str[INPUT_CHARS];
  int opt_index, opt;
  
  code_filename = NULL;
  while(1) {
    opt = getopt_long(argc, argv, "hS::vsl::L:idgoc:C", options, &opt_index);
    
    if (opt == -1)
      break;
      
    switch(opt) {
      case 'h':
        printf("dynamic 0.22b\nDynamic emulator of RAM programs\n\n" \
"Usage: dynamic [hS::vsl::L:idoc:C] path/to/ram/program\n\n" \
"Options:\n" \
"  -h --help                  - This help screen.\n" \
"  -S --save-code [[filename]]- Lets the dynamic to save generated\n" \
"                               code to file(s).\n" \
"  -v --verbose               - Hides all output (prints only the\n" \
"                               results from the output tape).\n" \
"  -s --summary               - Prints summary information while\n" \
"                               performing the translation.\n" \
"  -l --log-time [[n]]        - Log time for every n-th iteration.\n" \
"                               If n isn't given, log overall time only.\n" \
"                               The file 'log-time.txt' will be created.\n" \
"  -L --loop [n]              - Loop n times.\n" \
"  -i --interpret             - Perform interpretation.\n" \
"  -d --dynamic               - Perform dynamic translation.\n" \
"                               If no of the 'i','d','o' options are passed,\n" \
"                               this is the default option.\n" \
"  -o --open-cl               - Perform OpenCL RAM emulation.\n" \
"  -g --gpu                   - Run the OpenCL emulation on GPU\n" \
"  -c --compile [source_file] - Compile a source file into the output file.\n" \
"  -C --compile-only          - Do not perform emulation after compile.\n\n");
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
      case 'l':
        cmd_options |= CMD_LOGTIME;
        if (optarg)
          log_iter = atoi(optarg);
        else
          log_iter = -1;
        break;
      case 'L':
        cmd_options |= CMD_LOOPS;
        loops = atoi(optarg);
        break;
      case 'i':
        cmd_options |= CMD_INTERPRET;
        break;
      case 'd':
        cmd_options |= CMD_DYNAMIC;
        break;
      case 'o':
        cmd_options |= CMD_OPENCL;
        break;
      case 'c':
        cmd_options |= CMD_COMPILE;
        input_filename = optarg;
        break;
      case 'g':
        cmd_options |= CMD_GPU;
        break;
      case 'C':
        cmd_options |= CMD_COMPILE_ONLY;
        break;
      default: opt = -1; break;
    }
    if (opt == -1)
      break;
  }

  if (optind >= argc) {
    if (!(cmd_options & CMD_VERBOSE))
      printf("Error: Missing RAM program file name\n\n(Type 'dynamic --help' for help)\n");
    return ERROR_MISSING_ARGS;
  }

  /* first - compile */
  if (cmd_options & CMD_COMPILE) {
    if (!(cmd_options & CMD_VERBOSE))
      printf("Compiling file '%s'...\n", input_filename);
    if (compile(input_filename, argv[optind])) {
      if (!(cmd_options & CMD_VERBOSE))
        printf("\nFix the errors and try again!\n");
      return ERROR_COMPILE;
    } else {
      if (!(cmd_options & CMD_VERBOSE))
        printf("\nCompile runned OK\n");
    }
    
    if (cmd_options & CMD_COMPILE_ONLY)
      return OK;
  }

  /* load RAM program */
  if (cmd_options & CMD_SUMMARY)
    printf("Loading file: %s\n", argv[optind]);
  if (!bin.bin_load(argv[optind])) {
    if (!(cmd_options & CMD_VERBOSE))
      printf("Error loading file: %s\n", argv[optind]);
    return ERROR_LOAD;
  }

  int ram_size = bin.get_size();
  const char *prog = (const char*)bin.get_program();

  if (cmd_options & CMD_SUMMARY) {
    printf("\nEmulating program:\n");
    bin.bin_print();
  }

  if (cmd_options & CMD_SUMMARY)
    printf("Initializing...\n");
  ram_init(INPUT_CHARS);
  cache_init(ram_size);

  if (cmd_options & CMD_LOGTIME) {
    if ((flog = fopen("log-time.txt", "w")) == NULL) {
      printf("Error: cannot open file 'log-time.txt' !\n");
      return ERROR_LOGFILE;
    }
  }

  do {
    if (loops > 0) {
      if (!(cmd_options & CMD_VERBOSE))
        printf("Loop: #%d\n", loops);
      loops--;
    }
  
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
      log_counter = 0;
      overall_time = 0;
      
      while ((status = ram_interpret(prog, ram_size)) == RAM_OK) {
        if (cmd_options & CMD_LOGTIME) {
          if (log_counter < log_iter)
            log_counter++;
          else if (log_counter == log_iter) {
            log_counter = 0;
            end = clock();
            fprintf(flog, "%lf\n", (double)((double)end-(double)start));
            overall_time += (double)((double)end-(double)start);
            start = end;
          }
        }
      }
      end = clock();
      overall_time += (double)((double)end-(double)start);

      if (!(cmd_options & CMD_VERBOSE)) {
        printf("\nDone.\n\tError code: %d (%s)\n", status, ram_error(status));
        if ((cmd_options & CMD_LOGTIME) && (log_iter == -1))
          fprintf(flog, "%lf\n", overall_time);
      }

      if (!(cmd_options & CMD_VERBOSE))
        printf("\nOutput tape:\n\t");
      
      ram_output();
      printf("\n");
  
      char *input = ram_env.input;
      ram_init(INPUT_CHARS);
      strcpy(ram_env.input, input);
      free(input);
    }
  
    if ((cmd_options & CMD_DYNAMIC) 
         || !(cmd_options & (CMD_INTERPRET | CMD_OPENCL))) {
      if (!(cmd_options & CMD_VERBOSE))
        printf("Emulating using dynamic translation...\n");

      start = clock();
      log_counter = 0;
      overall_time = 0;
      do {
        tmp = cache_get_block(ram_env.pc);
        if (tmp == NULL) {
          if ((tmp = cache_create_block(ram_env.pc)) == NULL) {
            cache_flush();
            tmp = cache_create_block(ram_env.pc);
          }
          dyn_translate(tmp, prog, ram_size);
        } else if (tmp->size > 0)
          (*(void (*)())tmp->code)();
        else
          ram_env.state = ram_interpret(prog, ram_size);

        if (cmd_options & CMD_LOGTIME) {
          if (log_counter < log_iter)
            log_counter++;
          else if (log_counter == log_iter) {
            log_counter = 0;
            end = clock();
            fprintf(flog, "%lf\n", (double)((double)end-(double)start));
            overall_time += (double)((double)end-(double)start);
            start = end;
          }
        }
      } while ((ram_env.state == RAM_OK) && (ram_env.pc < ram_size));
      end = clock();
      overall_time += (double)((double)end-(double)start);
  
      if (!(cmd_options & CMD_VERBOSE)) {
        printf("\nDone.\n\tError code: %d (%s)\n", ram_env.state,
          ram_error(ram_env.state));
        if ((cmd_options & CMD_LOGTIME) && (log_iter == -1))
          fprintf(flog,"%lf\n", overall_time);    
      }
  
      if (!(cmd_options & CMD_VERBOSE))
        printf("\nOutput tape:\n\t");
    
      ram_output();
      printf("\n");

      char *input = ram_env.input;
      ram_init(INPUT_CHARS);
      strcpy(ram_env.input, input);
      free(input);
      cache_flush();
    }

    if (cmd_options & CMD_OPENCL) {    
      if (!(cmd_options & CMD_VERBOSE))
        printf("Emulating using OpenCL...\n\n");
        cl_execute(prog, ram_size, cmd_options, flog);
    }
  } while (loops > 0);
  
  if (cmd_options & CMD_LOGTIME)
    fclose(flog);
  
  cache_destroy();
  ram_destroy();

  return OK;
}
