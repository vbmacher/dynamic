/**
 * microcode.h
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#ifndef __MICROCODE__
#define __MICROCODE__

#include <stdlib.h>
#include <stdio.h>

#include "ram.h"

/* microcode excludes following instructions:
     halt, jmp, jz, jgtz
   poznatky:
      - nepouzivat lokalne premenne (potom gcc prida na konci "leave" instrukciu miesto "pop ebp")
      - nevolat funkcie
   co spomaluje:
      - ked je vela skokov
 */

/*#define M_JGTZ_I 35
static inline void micro_jgtz_i(int i) {
  ram_env.pc = (ram_env.r[0] > 0) ? program[i] : i+1;
}

#define M_JMP_I 35
static inline void micro_jmp_i(int i) {
  ram_env.pc = program[i];
}
*/
#define M_READ_I 35
static inline void micro_read_i(int i) {
  ram_env.r[i] = ram_env.input[ram_env.p_input++];
}

#define M_READ_II 42
static inline void micro_read_ii(int i) {
  ram_env.r[ram_env.r[i]] = ram_env.input[ram_env.p_input++];
}

#define M_WRITE_S 49
static inline void micro_write_s(int s) {
  if (ram_env.p_output >= RAM_OUTPUT_SIZE) {
    ram_env.state = RAM_OUTPUT_FULL;
    return;
  }
  ram_env.output[ram_env.p_output++] = s;
}

#define M_WRITE_I 56
static inline void micro_write_i(int i) {
  if (ram_env.p_output >= RAM_OUTPUT_SIZE) {
    ram_env.state = RAM_OUTPUT_FULL;
    return;
  }
  ram_env.output[ram_env.p_output++] = ram_env.r[i];
}

#define M_WRITE_II 63
static inline void micro_write_ii(int i) {
  if (ram_env.p_output >= RAM_OUTPUT_SIZE) {
    ram_env.state = RAM_OUTPUT_FULL;
    return;
  }
  ram_env.output[ram_env.p_output++] = ram_env.r[ram_env.r[i]];
}

#define M_LOAD_S 13
static inline void micro_load_s(int s) {
  ram_env.r[0] = s;
}

#define M_LOAD_I 20
static inline void micro_load_i(int i) {
  ram_env.r[0] = ram_env.r[i];
}

#define M_LOAD_II 27
static inline void micro_load_ii(int i) {
  ram_env.r[0] = ram_env.r[ram_env.r[i]];
}

#define M_STORE_I 20
static inline void micro_store_i(int i) {
  ram_env.r[i] = ram_env.r[0];
}

#define M_STORE_II 27
static inline void micro_store_ii(int i) {
  ram_env.r[ram_env.r[i]] = ram_env.r[0];
}

#define M_ADD_S 14
static inline void micro_add_s(int s) {
  ram_env.r[0] += s;
}

#define M_ADD_I 21
static inline void micro_add_i(int i) {
  ram_env.r[0] += ram_env.r[i];
}

#define M_ADD_II 28
static inline void micro_add_ii(int i) {
  ram_env.r[0] += ram_env.r[ram_env.r[i]];
}

#define M_SUB_S 14
static inline void micro_sub_s(int s) {
  ram_env.r[0] -= s;
}

#define M_SUB_I 21
static inline void micro_sub_i(int i) {
  ram_env.r[0] -= ram_env.r[i];
}

#define M_SUB_II 28
static inline void micro_sub_ii(int i) {
  ram_env.r[0] -= ram_env.r[ram_env.r[i]];
}

#define M_MUL_S 19
static inline void micro_mul_s(int s) {
  ram_env.r[0] *= s;
}

#define M_MUL_I 29
static inline void micro_mul_i(int i) {
  ram_env.r[0] *= ram_env.r[i];
}

#define M_MUL_II 33
static inline void micro_mul_ii(int i) {
  ram_env.r[0] *= ram_env.r[ram_env.r[i]];
}

#define M_DIV_S 37
static inline void micro_div_s(int s) {
  if (!s) {
    ram_env.state = RAM_DIVISION_BY_ZERO;
    return;
  }
  ram_env.r[0] /= s;
}

#define M_DIV_I 53
static inline void micro_div_i(int i) {
  register int t = ram_env.r[i];
  if (!t) {
    ram_env.state = RAM_DIVISION_BY_ZERO;
    return;
  }
  ram_env.r[0] /= t;
}

#define M_DIV_II 60
static inline void micro_div_ii(int i) {
  register int t = ram_env.r[ram_env.r[i]];
  if (!t) {
    ram_env.state = RAM_DIVISION_BY_ZERO;
    return;
  }
  ram_env.r[0] /= t;
}

#endif
