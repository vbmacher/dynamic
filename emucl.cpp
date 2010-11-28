/*
 * emucl.cpp
 *
 * (c) Copyright 2010, P. Jakubco <pjakubco@gmail.com>
 *
 * The OpenCL CPU emulator
 */

#include <cstdio>

#include "clprogram.hpp"
#include "main.hpp"
#include "bin.hpp"
#include "ram.h"
#include "cache.h"

int cl_execute(const char *prog, int ram_size, int cmd_options, FILE *flog) {
  CLprogram clprogram;
  double overall_time;
  
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
    if (cmd_options & CMD_LOGTIME)
      fprintf(flog,"%lf\n", overall_time);    
  }
  
  if (!(cmd_options & CMD_VERBOSE))
    printf("\nOutput tape:\n\t");
  
  ram_output();
  printf("\n");
  
  ram_init(INPUT_CHARS);
  cache_flush();  
}
