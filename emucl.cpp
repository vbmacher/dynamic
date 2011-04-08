/*
 * emucl.cpp
 *
 * (c) Copyright 2010-2011, P. Jakubco <pjakubco@gmail.com>
 * KISS, YAGNI
 *
 * The OpenCL RAM emulator
 
 The architecture of PRAM is as follows:
    
    P0,P1,P2,...,Pn    - RAM kernels, including own private register tape
    
    P[0],P[1],...,P[n] - Programs for RAM kernels
    
    M[0],M[1],...,M[k] - shared registers for PRAM,
                         there can be input and output
 
 
 */

#include <cstdio>

#include "clprogram.hpp"
#include "main.hpp"
#include "bin.hpp"
#include "ram.h"
#include "cache.h"

using namespace std;

typedef struct {
  cl::Buffer pc;
  cl::Buffer M;
  cl::Buffer ram_status;
  cl::Buffer ram_size;
} CLRAM;

CLRAM clram;

int cl_execute(const char *prog, int ram_size, int cmd_options, FILE *flog) {
  CLprogram clprogram;
  double overall_time;
  unsigned char events[1] = {0}; // events list for the RAM emulator
  unsigned short event_data[1] = {0}; // data for events satisfaction
  
  printf("Initializing OpenCL...\n");
  if (!clprogram.initCL())
    return ERROR_INIT;
    
  printf("Loading emulator...\n");
  if (!clprogram.loadProgram(RAMBin::load_file("emuram.cl")))
    return ERROR_LOAD;
  
  // prepare program as kernel argument (N times the same program)
  char *arg_prog = new char[ram_size * processors];
  int i;

  for (i =0;i < processors; i++) 
    memcpy(arg_prog + i*ram_size, prog, ram_size);

  // prepare PC registers for all processors
  cl_ushort *arg_pc = new cl_ushort[processors];
  for (i =0;i < processors; i++)
    arg_pc[i] = (cl_ushort)(i*ram_size);

  // prepare all statuses for all processors
  cl_ushort *arg_ram_state = new cl_ushort[processors];

  // prepare all RAM sizes for all processors
  cl_ushort *arg_ram_size = new cl_ushort[processors];
  cl_ushort tmp;
  for (i =0;i < processors; i++) 
    arg_ram_size[i]=(cl_ushort)ram_size;

  // prepare INPUT tape
  cl_ushort arg_M[RAM_REGISTERS_COUNT];
  for (i = 0; i < INPUT_CHARS; i++) {
    arg_M[i] = (cl_ushort) ram_env.input[i];
    printf("Input: %d\n", (int)arg_M[i]);
  }

  // create memory objects
  const cl::Context context = clprogram.getContext();

//__kernel void ramCL(__constant uchar *program, __global ushort *pc, 
  //                  __global ushort *M, __global ushort *ram_size, 
    //                ushort INPUT_SIZE, __global ushort *status, ushort debug) {

  
  cl::Buffer pprogram(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        ram_size * processors * sizeof(cl_uchar), (void *)arg_prog);
  clram.pc = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        processors * sizeof(cl_ushort), (void *)arg_pc);
  clram.M = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, // the shared memory
        RAM_REGISTERS_COUNT * sizeof(cl_ushort), (void*)arg_M);  
  clram.ram_status = cl::Buffer(context, CL_MEM_WRITE_ONLY, processors * sizeof(cl_ushort));
  clram.ram_size = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        processors * sizeof(cl_ushort), (void *)arg_ram_size);

  // setup parameter values
  cl::Kernel kernel;
  
  try {
    kernel = clprogram.getKernel("ramCL");
  } catch (exception& exc) {
    return ERROR_INIT;
  }
    
  int arg_status = 0;
  tmp = INPUT_CHARS;
  
  arg_status = kernel.setArg(0, sizeof(cl_mem), (void *)&pprogram);
  if (cmd_options & CMD_SUMMARY)
    printf("arg_status(RAM memory) = %d\n", arg_status);
  arg_status |= kernel.setArg(1, sizeof(cl_mem), (void *)&clram.pc);
  if (cmd_options & CMD_SUMMARY)
    printf("arg_status(PC) = %d\n", arg_status);
  arg_status |= kernel.setArg(2, sizeof(cl_mem), (void *)&clram.M);
  if (cmd_options & CMD_SUMMARY)
    printf("arg_status(shared memory) = %d\n", arg_status);
  arg_status |= kernel.setArg(3, sizeof(cl_mem), (void *)&clram.ram_size);
  if (cmd_options & CMD_SUMMARY)
    printf("arg_status(RAM size) = %d\n", arg_status);
  arg_status |= kernel.setArg(4, sizeof(cl_ushort), (void *)&tmp);
  if (cmd_options & CMD_SUMMARY)
    printf("arg_status(Input size) = %d\n", arg_status);
  arg_status |= kernel.setArg(5, sizeof(cl_mem), (void *)&clram.ram_status);
  if (cmd_options & CMD_SUMMARY)
    printf("arg_status(RAM state) = %d\n", arg_status);

  tmp = 1; // debug ON
  arg_status |= kernel.setArg(6, sizeof(cl_ushort), (void *)&tmp);
  if (cmd_options & CMD_SUMMARY)
    printf("arg_status(Debug) = %d\n", arg_status);

  overall_time = 0;
  cl::Event evt;
  
  if (cmd_options & CMD_SUMMARY)
    printf("HOST: Going to loop!\n");

  if (!clprogram.runKernel(kernel, processors, &evt)) {
    if (!(cmd_options & CMD_VERBOSE))
      printf("Error: The kernel was not able to execute (arg_status=%d)!\n", arg_status);
    return ERROR_EXECUTE;
  }

  if (cmd_options & CMD_SUMMARY)
    printf("HOST: Waiting for finish...\n");
  
  // wait for finish.
  if (evt.wait() != CL_SUCCESS) {
    if (!(cmd_options & CMD_VERBOSE))
        printf("Fatal Error: The kernel was not able to quit well (arg_status=%d)!\n", arg_status);
    return ERROR_EXECUTE;
  }
  
  if (cmd_options & CMD_SUMMARY)
    printf("HOST: kernel finished!\n");
  
 // clprogram.getCommandQueue().enqueueReadBuffer(clram.ram_status,CL_TRUE,0,
   // processors * sizeof(cl_ushort), &arg_ram_state);
//    if (arg_ram_state[0] != RAM_OK) // P0 must quit
  //    break;

  clprogram.finishAll(); // finish for finish :)
  
  // copy results from device back to the host
  clprogram.getCommandQueue().enqueueReadBuffer(clram.pc,CL_TRUE,0,
        processors * sizeof(cl_ushort), &arg_pc);
  clprogram.getCommandQueue().enqueueReadBuffer(clram.M,CL_TRUE,0,
        RAM_REGISTERS_COUNT * sizeof(cl_ushort), &arg_M);
  clprogram.getCommandQueue().enqueueReadBuffer(clram.ram_status,CL_TRUE,0,
        processors * sizeof(cl_ushort), &arg_ram_state);
  
  if (!(cmd_options & CMD_VERBOSE)) {
    printf("\nDone.\n\tError code: %d (%s)\n", arg_ram_state[0],
      ram_error(arg_ram_state[0]));
    if (cmd_options & CMD_LOGTIME)
      fprintf(flog,"%lf\n", overall_time);    
  }
  
  if (!(cmd_options & CMD_VERBOSE)) {
    printf("\nM[%d] = %d\n", INPUT_CHARS, arg_M[INPUT_CHARS]);
    printf("M[%d] = %d\n", INPUT_CHARS+1, arg_M[INPUT_CHARS+1]);
    printf("M[%d] = %d\n", INPUT_CHARS+2, arg_M[INPUT_CHARS+2]);
    printf("M[%d] = %d\n", INPUT_CHARS+3, arg_M[INPUT_CHARS+3]);
  }

  ram_init(INPUT_CHARS);
  cache_flush();  
}
