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
 * This method waits for specific event that might happen in any time.
 *
 * The method fires up (exits) when specific event is activated (should be fired).
 * The firing up notifies the host that the event happened.
 *
 * The event with index 0 is called 'cancel all' and causes that all pending events
 * will be cancelled.
 *
 * The event with index 1 is not a real event number, but it holds the error code
 * after event satisfaction.
 *
 * The values of the array are as follows:
 *   0 - means nothing/satisfied
 *   1 - means fire
 *
 * @param eventslist List of all available events (array)
 * @param event The index of the event
 */
__kernel void waitForEvent(__global uchar *eventslist, ushort event) {
  barrier(CLK_GLOBAL_MEM_FENCE);
  printf("CL: I am here!\n");
  while (!eventslist[event] && !eventslist[0]) {
    printf("CL: waiting...(eventslist[%d] = %d, eventslist[0] = %d)\n", event, eventslist[event], eventslist[0]);
    ;
  }
  eventslist[0] = 0; // the cancelAll event has nothing common with the host
}

/**
 * This method waits for satisfying an event in the eventslist. It is not a kernel.
 *
 * @param eventslist The array of all events
 * @param event the index of the event
 */
short waitForSatisfy(__global uchar *eventslist, ushort event) {
  // TODO: memory fence on eventslist in the beginning and let it out at the end
  printf("CL: Firing the event %d\n", event);
  
  eventslist[event] = 1;  // TODO: synchronization??
  barrier(CLK_GLOBAL_MEM_FENCE);

  printf("CL satisfy: After calling barrier()\n");

  eventslist[1] = 0;
  while (eventslist[event]) // TODO: what about cancelAll ?
    ;
  printf("CL: Event completed with error code %d.\n", eventslist[1]);
  return eventslist[1];
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
 * @param output
 *   Output tape
 * @param p_output
 *   Pointer to output tape
 * @param RAM_OUTPUT_SIZE
 *   Max. output tape size
 * @param status
 *   The status of the RAM machine.
 * @param eventslist
 *   Array of events available for the RAM emulator.
 *   Structure (indexes):
 *     0 - cancelAll
 *     1 - satisfyError
 *     2 - wait for input
 * @param event_data
 *   Data that will be transferred when satisfying the input event
 */
__kernel void ramCL(__constant uchar *program, __global ushort *pc, 
                    __global ushort *r, ushort ram_size, __global uchar *output,
                    __global uint *p_output, ushort RAM_OUTPUT_SIZE, __global ushort *status,
                    __global uchar *eventslist, __global ushort *event_data) {

  size_t i = get_global_id(0);

  if (i > 0) return;
  if (get_local_id(0) > 0) return;

  int c, t;

  status[0] = 0;
  p_output[0] = 0;
  
  while ((pc[0] < ram_size) && (status[0] == 0)) {
    if (pc[0] >= ram_size) {
      printf("CL: Error: Address fallout.\n");
      status[0] = 2;
      eventslist[0] = 1; // cancelAll
      return;
    }
    c = program[pc[0]++];
    if ((c > 0) && (pc[0] >= ram_size)) {
      printf("CL: Error: Address fallout.\n");
      status[0] = 2;
      eventslist[0] = 1; // cancelAll
      return;
    }

    print_instr(program, pc[0]-1);
    switch (c) {
      case 0: /* HALT */
        status[0] = 3;
        eventslist[0] = 1; // cancelAll
        return;
      case 1: /* READ i */
        if (!waitForSatisfy(eventslist, 2))
            r[program[pc[0]++]] = event_data[0];
        else {
            eventslist[0] = 1; //cancelAll
            return;
        }
        break;
      case 2: /* READ *i */
        if (!waitForSatisfy(eventslist, 2))
            r[r[program[pc[0]++]]] = event_data[0];
        else {
            eventslist[0] = 1; //cancelAll
            return;
        }
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
        eventslist[0] = 1;
        return;
    }
    status[0] = 0;
  }
  output[p_output[0]] = 0;
  if (pc[0] >= ram_size) {
  //  cout << \"Error: Address fallout.\" << endl;
    status[0] = 2;
    eventslist[0] = 1;
    return;
  }
  eventslist[0] = 1;
}
