/**
 * ram.c
 *
 * (c) Copyright 2010, P. Jakubco
 *
 * This module handles the RAM state
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "ram.h"

RAM_env ram_env;

/**
 * The function initializes static state of RAM machine.
 */
void ram_init(int input_size) {
  ram_env.pc = 0;
  memset(ram_env.r, 0, 100);
  ram_env.input = (char *)calloc(1, input_size);
  memset(ram_env.output, 0, RAM_OUTPUT_SIZE);
  ram_env.p_input = 0;
  ram_env.p_output = 0;
  ram_env.state = RAM_OK;
}

/**
 * Frees memory
 */
void ram_destroy() {
  free(ram_env.input);
}

/**
 * The function prints the error code description.
 * @param error_code the error code
 */
const char *ram_error(int error_code) {
  switch (error_code) {
    case RAM_HALT: return "RAM_HALT";
    case RAM_UNKNOWN_INSTRUCTION: return "RAM_UNKNOWN_INSTRUCTION";
    case RAM_ADDRESS_FALLOUT: return "RAM_ADDRESS_FALLOUT";
    case RAM_OUTPUT_FULL: return "RAM_OUTPUT_FULL";
    case RAM_DIVISION_BY_ZERO: return "RAM_DIVISION_BY_ZERO";
    default: return "unknown";
  }
}

/**
 * The function prints out the output tape.
 */
void ram_output() {
  int stop = ram_env.p_output;
  int i;
  
  for (i = 0; i < stop; i++)
    printf("%d ", ram_env.output[i]);
  printf("\n");
}


/**
 * This function interprets single instruction. This is an interpreter.
 * It is assumed that the syntax and semantics are correct.
 *
 * @param program - the memory with program
 * @return error code (0 - ok)
 */
int ram_interpret(const char *program, int ram_size) {
  int c,t;

  if (ram_size < 0) {
    printf("Error: RAM program was not loaded properly.");
    return 1;
  }

  if (ram_env.pc >= ram_size) {
    printf("Error: Address fallout.\n");
    return 2;
  }

  c = program[ram_env.pc++];
  if ((c > 0) && (ram_env.pc >= ram_size)) {
    printf("Error: Address fallout.\n");
    return 2;
  }

/*  printf("PC: %d, Instr: %d\n", ram_env.pc, c); */
  switch (c) {
    case 0: return RAM_HALT;
    case 1: /* READ i */
      ram_env.r[program[ram_env.pc++]] = ram_env.input[ram_env.p_input++];
      break;
    case 2: /* READ *i */
      ram_env.r[ram_env.r[program[ram_env.pc++]]] = ram_env.input[ram_env.p_input++];
      break;
    case 3: /* WRITE =i */
      if (ram_env.p_output >= RAM_OUTPUT_SIZE) {
        printf("Error: Output tape is full.\n");
        return RAM_OUTPUT_FULL;
      }
      ram_env.output[ram_env.p_output++] = program[ram_env.pc++];
      break;
    case 4: /* WRITE i */
      if (ram_env.p_output >= RAM_OUTPUT_SIZE) {
        printf("Error: Output tape is full.\n");
        return RAM_OUTPUT_FULL;
      }
      ram_env.output[ram_env.p_output++] = ram_env.r[program[ram_env.pc++]];
      break;
    case 5: /* WRITE *i */
      if (ram_env.p_output >= RAM_OUTPUT_SIZE) {
        printf("Error: Output tape is full.\n");
        return RAM_OUTPUT_FULL;
      }
      ram_env.output[ram_env.p_output++] = ram_env.r[ram_env.r[program[ram_env.pc++]]];
      break;
    case 6: /* LOAD =i */
      ram_env.r[0] = (unsigned char)program[ram_env.pc++]; break;
    case 7: /* LOAD i */ 
      ram_env.r[0] = ram_env.r[(unsigned char)program[ram_env.pc++]]; break;
    case 8: /* LOAD *i */
      ram_env.r[0] = ram_env.r[ram_env.r[(unsigned char)program[ram_env.pc++]]]; break;
    case 9: /* STORE i */
      ram_env.r[(unsigned char)program[ram_env.pc++]] = ram_env.r[0]; break;
    case 10: /* STORE *i */
      ram_env.r[ram_env.r[(unsigned char)program[ram_env.pc++]]] = ram_env.r[0]; break;
    case 11: /* ADD =i */
      ram_env.r[0] += (unsigned char)program[ram_env.pc++]; break;
    case 12: /* ADD i */
      ram_env.r[0] += ram_env.r[(unsigned char)program[ram_env.pc++]]; break;
    case 13: /* ADD *i */
      ram_env.r[0] += ram_env.r[ram_env.r[(unsigned char)program[ram_env.pc++]]]; break;
    case 14: /* SUB =i */
      ram_env.r[0] -= (unsigned char)program[ram_env.pc++]; break;
    case 15: /* SUB i */
      ram_env.r[0] -= ram_env.r[(unsigned char)program[ram_env.pc++]]; break;
    case 16: /* SUB *i */
      ram_env.r[0] -= ram_env.r[ram_env.r[(unsigned char)program[ram_env.pc++]]]; break;
    case 17: /* MUL =i */
      ram_env.r[0] *= (unsigned char)program[ram_env.pc++]; break;
    case 18: /* MUL i */
      ram_env.r[0] *= ram_env.r[(unsigned char)program[ram_env.pc++]]; break;
    case 19: /* MUL *i */
      ram_env.r[0] *= ram_env.r[ram_env.r[(unsigned char)program[ram_env.pc++]]]; break;
    case 20: /* DIV =i */
      t = (unsigned char)program[ram_env.pc++];
      if (!t) {
        printf("Error: division by zero.\n");
        return RAM_DIVISION_BY_ZERO;
      }
      ram_env.r[0] /= t; break;    
    case 21: /* DIV i */
      t = ram_env.r[(unsigned char)program[ram_env.pc++]];
      if (!t) {
        printf("Error: division by zero.\n");
        return RAM_DIVISION_BY_ZERO;
      }
      ram_env.r[0] /= t; break;    
    case 22: /* DIV *i */
      t = ram_env.r[ram_env.r[(unsigned char)program[ram_env.pc++]]];
      if (!t) {
        printf("Error: division by zero.\n");
        return RAM_DIVISION_BY_ZERO;
      }
      ram_env.r[0] /= t; break;
    case 23: /* JMP i */
      ram_env.pc = (unsigned char)program[ram_env.pc]; break;
    case 24: /* JGTZ i */
      ram_env.pc = ((unsigned int)ram_env.r[0] > 0) ? (unsigned char)program[ram_env.pc] : ram_env.pc+1;
      break;
    case 25: /* JZ i */
      ram_env.pc = (ram_env.r[0] == 0) ? (unsigned char)program[ram_env.pc] : ram_env.pc+1;
      break;
    default:
      printf("Error: unknown insruction (%d).\n",c);
      return RAM_UNKNOWN_INSTRUCTION;
  }
  return RAM_OK;
}
