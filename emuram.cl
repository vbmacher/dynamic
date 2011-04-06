/**
 * emuram.cl
 *
 * (c) Copyright 2010-2011, P. Jakubco <pjakubco@gmail.com>
 *
 * KISS, YAGNI
 * 
 * OpenCL emulator of PRAM
 * 
 * How to pass program:
 *   - one big string, programs are in sequential order
 *   - another parameter is array of positions of start of all programs in the
 *     string       
 *
 */

#pragma OPENCL EXTENSION cl_amd_printf : enable

#define RAM_OK 0
#define RAM_UNKNOWN_INSTRUCTION 1
#define RAM_ADDRESS_FALLOUT 2
#define RAM_HALT 3
#define RAM_OUTPUT_FULL 4
#define RAM_DIVISION_BY_ZERO 5


/**
 * Dissassembles an instruction. 
 * 
 * @param program
 *    The program tape
 * @param pc
 *    Starting position of dissassembling  
 */
void print_instr(__constant uchar *program, int pc) {
  printf("%d: ", pc);
  switch (program[pc]) {
  case 0: printf("\tHALT\n");
      break;
  case 1: printf("\tREAD %d\n", (unsigned int) program[pc+1]);
      break;
  case 2: printf("\tREAD *%d\n", (unsigned int) program[pc+1]);
      break;
  case 3: printf("\tWRITE =%d\n", (unsigned int) program[pc+1]);
      break;
  case 4: printf("\tWRITE %d\n", (unsigned int) program[pc+1]);
      break;
  case 5: printf("\tWRITE *%d\n", (unsigned int) program[pc+1]);
      break;
  case 6: printf("\tLOAD =%d\n", (unsigned int) program[pc+1]);
      break;
  case 7: printf("\tLOAD %d\n", (unsigned int) program[pc+1]);
      break;
  case 8: printf("\tLOAD *%d\n", (unsigned int) program[pc+1]);
      break;
  case 9: printf("\tSTORE %d\n", (unsigned int) program[pc+1]);
      break;
  case 10: printf("\tSTORE *%d\n", (unsigned int) program[pc+1]);
      break;
  case 11: printf("\tADD =%d\n", (unsigned int) program[pc+1]);
      break;
  case 12: printf("\tADD %d\n", (unsigned int) program[pc+1]);
      break;
  case 13: printf("\tADD *%d\n", (unsigned int) program[pc+1]);
      break;
  case 14: printf("\tSUB =%d\n", (unsigned int) program[pc+1]);
      break;
  case 15: printf("\tSUB %d\n", (unsigned int) program[pc+1]);
      break;
  case 16: printf("\tSUB *%d\n", (unsigned int) program[pc+1]);
      break;
  case 17: printf("\tMUL =%d\n", (unsigned int) program[pc+1]);
      break;
  case 18: printf("\tMUL %d\n", (unsigned int) program[pc+1]);
      break;
  case 19: printf("\tMUL *%d\n", (unsigned int) program[pc+1]);
      break;
  case 20: printf("\tDIV =%d\n", (unsigned int) program[pc+1]);
      break;
  case 21: printf("\tDIV %d\n", (unsigned int) program[pc+1]);
      break;
  case 22: printf("\tDIV *%d\n", (unsigned int) program[pc+1]);
      break;
  case 23: printf("\tJMP %d\n", (unsigned int) program[pc+1]);
      break;
  case 24: printf("\tJGTZ %d\n", (unsigned int) program[pc+1]);
      break;
  case 25: printf("\tJZ %d\n", (unsigned int) program[pc+1]);
      break;
  default:
      printf("\t>> UNKNOWN INSTRUCTION <<\n");
      break;
  }
}

/**
 * The RAM emulator - interpreter.
 *
 * @param program
 *   Array of RAM binary programs in sequential order, assuming that it is
 *   not null.
 * @param pc 
 *   Array of Program counter register for all RAM programs.
 * @param M
 *   Array of all shared registers (finite, say 100 is enough).
 * @param ram_size
 *   Array of sizes of programs in bytes, assuming ram_size > 0.
 * @param INPUT_SIZE
 *   Input size - number of input symbols stored in M, starting at position 0
 * @param status
 *   The status of the RAM machine.
 * @param debug
 *   If set to 1, the emulator will disassemble and print instructions.
 */
__kernel void ramCL(__constant uchar *program, __global ushort *pc, 
                    __global ushort *M, __global ushort *ram_size, 
                    ushort INPUT_SIZE, __global ushort *status, ushort debug) {

  ushort r[100];             // private registers
  int p_output = INPUT_SIZE; // pointer to the output in M, TODO: bug?
  int p_input = 0;           // pointer to the input in M

  size_t i = get_global_id(0); // get processor index

  int opcode, t;
  
  status[i] = RAM_OK;        // status of the PRAM
  while ((pc[i] < ram_size[i]) && (status[i] == 0)) {
    if (pc[i] >= ram_size) {
      printf("CL Error: P_%d Address fallout.\n", i);
      status[i] = RAM_ADDRESS_FALLOUT;
      return;
    }
    opcode = program[pc[i]++];
    
    // test if the next instruction fits into the program size   
    if ((c > 0) && (pc[i] >= ram_size)) {
      printf("CL: Error: Address fallout.\n");
      status[i] = RAM_ADDRESS_FALLOUT;
      return;
    }

    if (debug == 1)
      print_instr(program, pc[i]-1);
    switch (opcode) {
      case 0: /* HALT */
        status[i] = RAM_HALT;
        return;
      case 1: /* READ i */
        mem_fence(CLK_GLOBAL_MEM_FENCE);
        r[program[pc[i]++]] = M[p_input++];
        if (debug == 1)          
          printf("CL: Input read = %d\n", M[p_input-1]);
        break;
      case 2: /* READ *i */
        mem_fence(CLK_GLOBAL_MEM_FENCE);
        r[r[program[pc[i]++]]] = M[p_input++];
        if (debug == 1)
          printf("CL: Input read = %d\n", M[p_input-1]);
        break;
      case 3: /* WRITE =i */
        if (p_output >= RAM_OUTPUT_SIZE) {
//            cout << \"Error: Output tape is full.\" << endl;
            status[i] = RAM_OUTPUT_FULL;
            return;
        }
        mem_fence(CLK_GLOBAL_MEM_FENCE);
        M[p_output++] = program[pc[i]++];
        break;
      case 4: /* WRITE i */
        if (p_output >= RAM_OUTPUT_SIZE) {
//            cout << \"Error: Output tape is full.\" << endl;
            status[i] = RAM_OUTPUT_FULL;
            return;
        }
        mem_fence(CLK_GLOBAL_MEM_FENCE);
        M[p_output++] = r[program[pc[i]++]];
        break;
      case 5: /* WRITE *i */
        if (p_output >= RAM_OUTPUT_SIZE) {
//            cout << \"Error: Output tape is full.\" << endl;
            status[i] = RAM_OUTPUT_FULL;
            return;
        }
        mem_fence(CLK_GLOBAL_MEM_FENCE);
        M[p_output++] = r[r[program[pc[i]++]]];
        break;
      case 6: /* LOAD =i */
        r[0] = program[pc[i]++];
        break;
      case 7: /* LOAD i */
        r[0] = r[program[pc[i]++]];
        break;
      case 8: /* LOAD *i */
        r[0] = r[r[program[pc[i]++]]];
        break;
      case 9: /* STORE i */
        r[program[pc[i]++]] = r[0];
        break;
      case 10: /* STORE *i */
        r[r[program[pc[i]++]]] = r[0];
        break;
      case 11: /* ADD =i */
        r[0] += program[pc[i]++];
        break;
      case 12: /* ADD i */
        r[0] += r[program[pc[i]++]];
        break;
      case 13: /* ADD *i */
        r[0] += r[r[program[pc[i]++]]];
        break;
      case 14: /* SUB =i */
        r[0] -= program[pc[i]++];
        break;
      case 15: /* SUB i */
        r[0] -= r[program[pc[i]++]];
        break;
      case 16: /* SUB *i */
        r[0] -= r[r[program[pc[i]++]]];
        break;
      case 17: /* MUL =i */
        r[0] *= program[pc[i]++];
        break;
      case 18: /* MUL i */
        r[0] *= r[program[pc[i]++]];
        break;
      case 19: /* MUL *i */
        r[0] *= r[r[program[pc[i]++]]];
        break;
      case 20: /* DIV =i */
        t = program[pc[i]++];
        if (!t) {
//          cout << \"Error: division by zero.\" << endl;
          status[i] = RAM_DIVISION_ZERO;
          return;
        }
        r[0] /= t;
        break;
      case 21: /* DIV i */
        t = r[program[pc[i]++]];
        if (!t) {
//          cout << \"Error: division by zero.\" << endl;
          status[i] = RAM_DIVISION_ZERO;
          return;
        }
        r[0] /= t;
        break;
      case 22: /* DIV *i */
        t = r[r[program[pc[i]++]]];
        if (!t) {
          status[i] = RAM_DIVISION_ZERO;
          return;
        }
        r[0] /= t;
        break;
      case 23: /* JMP i */
        pc[i] = program[pc[i]];
        break;
      case 24: /* JGTZ i */
        pc[i] = (r[0] > 0) ? program[pc[i]] : pc[i] + 1;
        break;
      case 25: /* JZ i */
        pc[i] = (r[0] == 0) ? program[pc[i]] : pc[i] + 1;
        break;
      default:
        status[i] = RAM_UNKNOWN_INSTRUCTION;
        return;
    }
    status[i] = RAM_OK;
  }
  mem_fence(CLK_GLOBAL_MEM_FENCE);
  M[p_output] = 0;
  if (pc[i] >= ram_size) {
    status[0] = RAM_ADDRESS_FALLOUT;
    return;
  }
}
