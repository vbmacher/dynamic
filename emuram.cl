/**
 * emuram.cl
 *
 * (c) Copyright 2010, P. Jakubco <pjakubco@gmail.com>
 *
 * KISS, YAGNI
 *
 */

#pragma OPENCL EXTENSION cl_amd_printf : enable

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
 *   The RAM program binary, assuming that it is not null
 * @param pc 
 *   Program counter register.
 * @param r
 *   Array of all RAM registers (finite, say 100 is enough).
 * @param ram_size
 *   Size of the program in bytes, assuming ram_size > 0.
 * @param input
 *   Input tape
 * @param output
 *   Output tape
 * @param p_input
 *   Pointer to input tape
 * @param p_output
 *   Pointer to output tape
 * @param RAM_OUTPUT_SIZE
 *   Max. output tape size
 * @param status
 *   The status of the RAM machine.
 */
__kernel void ramCL(__constant uchar *program, __global ushort *pc, 
                    __global ushort *r, ushort ram_size,
                    __constant uchar *input, __global uchar *output,
                    __global uint *p_input, __global uint *p_output,
                    ushort RAM_OUTPUT_SIZE, __global ushort *status) {

  size_t i = get_global_id(0);

  if (i > 0) return;
  if (get_local_id(0) > 0) return;

  int c, t;

  status[0] = 0;
  p_output[0] = 0;
  
  while ((pc[0] < ram_size) && (status[0] == 0)) {
    if (pc[0] >= ram_size) {
//      cout << \"Error: Address fallout.\" << endl;
      status[0] = 2;
      return;
    }
    c = program[pc[0]++];
    if ((c > 0) && (pc[0] >= ram_size)) {
//      cout << \"Error: Address fallout.\" << endl;
      status[0] = 2;
      return;
    }

//    print_instr(program, pc[0]-1);
    switch (c) {
      case 0: /* HALT */
        status[0] = 3;
        return;
      case 1: /* READ i */
        r[program[pc[0]++]] = input[p_input[0]++];
        break;
      case 2: /* READ *i */
        r[r[program[pc[0]++]]] = input[p_input[0]++];
        break;
      case 3: /* WRITE =i */
        if (p_output[0] >= RAM_OUTPUT_SIZE) {
//            cout << \"Error: Output tape is full.\" << endl;
            status[0] = 4;
            return;
        }
        output[p_output[0]++] = program[pc[0]++];
        break;
      case 4: /* WRITE i */
        if (p_output[0] >= RAM_OUTPUT_SIZE) {
//            cout << \"Error: Output tape is full.\" << endl;
            status[0] = 4;
            return;
        }
        output[p_output[0]++] = r[program[pc[0]++]];
        break;
      case 5: /* WRITE *i */
        if (p_output[0] >= RAM_OUTPUT_SIZE) {
//            cout << \"Error: Output tape is full.\" << endl;
            status[0] = 4;
            return;
        }
        output[p_output[0]++] = r[r[program[pc[0]++]]];
        break;
      case 6: /* LOAD =i */
        r[0] = program[pc[0]++];
        break;
      case 7: /* LOAD i */
        r[0] = r[program[pc[0]++]];
        break;
      case 8: /* LOAD *i */
        r[0] = r[r[program[pc[0]++]]];
        break;
      case 9: /* STORE i */
        r[program[pc[0]++]] = r[0];
        break;
      case 10: /* STORE *i */
        r[r[program[pc[0]++]]] = r[0];
        break;
      case 11: /* ADD =i */
        r[0] += program[pc[0]++];
        break;
      case 12: /* ADD i */
        r[0] += r[program[pc[0]++]];
        break;
      case 13: /* ADD *i */
        r[0] += r[r[program[pc[0]++]]];
        break;
      case 14: /* SUB =i */
        r[0] -= program[pc[0]++];
        break;
      case 15: /* SUB i */
        r[0] -= r[program[pc[0]++]];
        break;
      case 16: /* SUB *i */
        r[0] -= r[r[program[pc[0]++]]];
        break;
      case 17: /* MUL =i */
        r[0] *= program[pc[0]++];
        break;
      case 18: /* MUL i */
        r[0] *= r[program[pc[0]++]];
        break;
      case 19: /* MUL *i */
        r[0] *= r[r[program[pc[0]++]]];
        break;
      case 20: /* DIV =i */
        t = program[pc[0]++];
        if (!t) {
//          cout << \"Error: division by zero.\" << endl;
          status[0] = 5;
          return;
        }
        r[0] /= t;
        break;
      case 21: /* DIV i */
        t = r[program[pc[0]++]];
        if (!t) {
//          cout << \"Error: division by zero.\" << endl;
          status[0] = 5;
          return;
        }
        r[0] /= t;
        break;
      case 22: /* DIV *i */
        t = r[r[program[pc[0]++]]];
        if (!t) {
//          cout << \"Error: division by zero.\" << endl;
          status[0] = 5;
          return;
        }
        r[0] /= t;
        break;
      case 23: /* JMP i */
        pc[0] = program[pc[0]];
        break;
      case 24: /* JGTZ i */
        pc[0] = (r[0] > 0) ? program[pc[0]] : pc[0] + 1;
        break;
      case 25: /* JZ i */
        pc[0] = (r[0] == 0) ? program[pc[0]] : pc[0] + 1;
        break;
      default:
//        cout << \"Error: unknown insruction.\" << endl;
        status[0] = 1;
        return;
    }
    status[0] = 0;
  }
  output[p_output[0]] = 0;
  if (pc[0] >= ram_size) {
  //  cout << \"Error: Address fallout.\" << endl;
    status[0] = 2;
    return;
  }
  return;
}