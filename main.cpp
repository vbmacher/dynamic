/**
 * main.cpp
 *
 * (c) Copyright 2010, P. Jakubèo
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
    opt = getopt_long(argc, argv, "hS::vsl::L:idoc:C", options, &opt_index);
    
    if (opt == -1)
      break;
      
    switch(opt) {
      case 'h':
        printf("dynamic 0.22b\nDynamic emulator of RAM programs\n\n" \
"Usage: dynamic [hS::vsl::L:ic:C] path/to/ram/program\n\n" \
"Options:\n" \
"\t-h --help                       - This help screen.\n" \
"\t-S --save-code [[filename_base]]- Lets the dynamic to save generated\n" \
"\t                                  code to file(s).\n" \
"\t-v --verbose                    - Hides all output (prints only the\n" \
"\t                                  results from the output tape).\n" \
"\t-s --summary                    - Prints summary information while\n" \
"\t                                  performing the translation.\n" \
"\t-l --log-time [[n]]             - Log time for every n-th iteration.\n" \
"\t                                  If n isn't given, log overall time only.\n" \
"\t                                  The file 'log-time.txt' will be created.\n" \
"\t-L --loop [n]                   - Loop n times.\n" \
"\t-i --interpret                  - Perform interpretation.\n" \
"\t-d --dynamic                    - Perform dynamic translation.\n" \
"\t                                  If no of the 'i','d','o' options are passed,\n" \
"\t                                  this is the default option."
"\t-o --open-cl                    - Perform OpenCL RAM emulation.\n" \
"\t-c --compile [source_file]      - Compile a source file into the output file.\n" \
"\t-C --compile-only               - Do not perform emulation after compile.\n\n");
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
      case 'C':
        cmd_options |= CMD_COMPILE_ONLY;
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

      CLprogram clprogram;
      printf("Initializing OpenCL...\n");
      if (!clprogram.initCL())
        return ERROR_INIT;
        
      printf("Loading emulator...\n");
      if (!clprogram.loadProgram(RAMBin::load_file("emuram.cl")))
        return ERROR_LOAD;

      // create memory objects
      const cl::Context context = clprogram.getContext();

      cl::Buffer pprogram(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            ram_size * sizeof(cl_uchar), (void *)prog);
      cl::Buffer pc(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_ushort), &ram_env.pc);
      cl::Buffer r(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            RAM_REGISTERS_COUNT * sizeof(cl_uchar), ram_env.r);
      cl::Buffer input(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            strlen(ram_env.input), (void *)ram_env.input);
      cl::Buffer output(context, CL_MEM_WRITE_ONLY,
            RAM_OUTPUT_SIZE * sizeof(cl_uchar));
      cl::Buffer p_input(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_ushort), &ram_env.p_input);
      cl::Buffer p_output(context, CL_MEM_READ_WRITE, sizeof(cl_ushort));
      cl::Buffer ram_status(context, CL_MEM_WRITE_ONLY, sizeof(cl_ushort));

      // setup parameter values
      cl::Kernel kernel = clprogram.getKernel("ramCL");
      unsigned short ram_output_size = RAM_OUTPUT_SIZE;

      int arg_status = 0;
      arg_status = kernel.setArg(0, sizeof(cl_mem), (void *)&pprogram);
      
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(RAM memory) = %d\n", arg_status);
      arg_status |= kernel.setArg(1, sizeof(cl_mem), (void *)&pc);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(PC) = %d\n", arg_status);
      arg_status |= kernel.setArg(2, sizeof(cl_mem), (void *)&r);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(registers) = %d\n", arg_status);
      arg_status |= kernel.setArg(3, sizeof(cl_ushort), (void *)&ram_size);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(RAM size) = %d\n", arg_status);
      arg_status |= kernel.setArg(4, sizeof(cl_mem), (void *)&input);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(Input tape) = %d\n", arg_status);
      arg_status |= kernel.setArg(5, sizeof(cl_mem), (void *)&output);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(Output tape) = %d\n", arg_status);
      arg_status |= kernel.setArg(6, sizeof(cl_mem), (void *)&p_input);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(Pointer to input tape) = %d\n", arg_status);
      arg_status |= kernel.setArg(7, sizeof(cl_mem), (void *)&p_output);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(Pointer to output tape) = %d\n", arg_status);
      arg_status |= kernel.setArg(8, sizeof(cl_ushort), (void *)&ram_output_size);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(Output tape size) = %d\n", arg_status);
      arg_status |= kernel.setArg(9, sizeof(cl_mem), (void *)&ram_status);
      if (cmd_options & CMD_SUMMARY)
        printf("arg_status(RAM state) = %d\n", arg_status);

      overall_time = 0;
      if (!clprogram.runKernel(kernel)) {
        if (!(cmd_options & CMD_VERBOSE))
          printf("Error: The kernel was not able to execute (arg_status=%d)!\n", arg_status);
        return ERROR_EXECUTE;
      }

      // copy results from device back to host
      clprogram.getCommandQueue().enqueueReadBuffer(output,CL_TRUE,0,
            RAM_OUTPUT_SIZE * sizeof(cl_uchar), ram_env.output);
      clprogram.getCommandQueue().enqueueReadBuffer(pc,CL_TRUE,0,
            sizeof(cl_ushort), &ram_env.pc);
      clprogram.getCommandQueue().enqueueReadBuffer(r,CL_TRUE,0,
            RAM_REGISTERS_COUNT * sizeof(cl_uchar), ram_env.r);
      clprogram.getCommandQueue().enqueueReadBuffer(p_input,CL_TRUE,0,
            sizeof(cl_ushort), &ram_env.p_input);
      clprogram.getCommandQueue().enqueueReadBuffer(p_output,CL_TRUE,0,
            sizeof(cl_ushort), &ram_env.p_output);
      clprogram.getCommandQueue().enqueueReadBuffer(ram_status,CL_TRUE,0,
            sizeof(cl_ushort), &ram_env.state);

      clprogram.getCommandQueue().finish();

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

      ram_init(INPUT_CHARS);
      cache_flush();
    }
  } while (loops > 0);
  
  if (cmd_options & CMD_LOGTIME)
    fclose(flog);
  
  cache_destroy();
  ram_destroy();

  return OK;
}
