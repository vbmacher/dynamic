/**
 * microcode.c
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "ram.h"
#include "microcode.h"

/* microcode excludes following instructions:
     halt, jmp, jz, jgtz
 */

static inline void m_halt(void) {
  ram_env.state = RAM_HALT;
  return; // ?? test
  return;
}
static inline void m_halt(void);

static inline void micro_read_i(int i) {
  ram_env.r[i] = ram_env.input[ram_env.p_input++]+'0';
}
static inline void m_read_i(void) {};

static inline void micro_read_ii(int i) {
  ram_env.r[ram_env.r[i]] = ram_env.input[ram_env.p_input++]+'0';
}
static inline void m_read_ii(void) {};

static inline void micro_write_s(int s) {
  if (ram_env.p_output >= 100) {
 //   printf("Error: Output tape is full.\n");
    ram_env.state = RAM_OUTPUT_FULL;
    return;
  }
  ram_env.output[ram_env.p_output++] = s;
}
static inline void m_write_s(void) {};

static inline void micro_write_i(int i) {
  if (ram_env.p_output >= 100) {
//    printf("Error: Output tape is full.\n");
    ram_env.state = RAM_OUTPUT_FULL;
    return;
  }
  ram_env.output[ram_env.p_output++] = ram_env.r[i];
}
static inline void m_write_i(void) {};

static inline void micro_write_ii(int i) {
  if (ram_env.p_output >= 100) {
//    printf("Error: Output tape is full.\n");
    ram_env.state = RAM_OUTPUT_FULL;
    return;
  }
  ram_env.output[ram_env.p_output++] = ram_env.r[ram_env.r[i]];
}
static inline void m_write_ii(void) {};

static inline void micro_load_s(int s) {
  ram_env.r[0] = s;
}
static inline void m_load_s(void) {};

static inline void micro_load_i(int i) {
  ram_env.r[0] = ram_env.r[i];
}
static inline void m_load_i(void) {};

static inline void micro_load_ii(int i) {
  ram_env.r[0] = ram_env.r[ram_env.r[i]];
}
static inline void m_load_ii(void) {};

static inline void micro_store_i(int i) {
  ram_env.r[i] = ram_env.r[0];
}
static inline void m_store_i(void) {};

static inline void micro_store_ii(int i) {
  ram_env.r[ram_env.r[i]] = ram_env.r[0];
}
static inline void m_store_ii(void) {};

static inline void micro_add_s(int s) {
  ram_env.r[0] += s;
}
static inline void m_add_s(void) {};

static inline void micro_add_i(int i) {
  ram_env.r[0] += ram_env.r[i];
}
static inline void m_add_i(void) {};

static inline void micro_add_ii(int i) {
  ram_env.r[0] += ram_env.r[ram_env.r[i]];
}
static inline void m_add_ii(void) {};

static inline void micro_sub_s(int s) {
  ram_env.r[0] -= s;
}
static inline void m_sub_s(void) {};

static inline void micro_sub_i(int i) {
  ram_env.r[0] -= ram_env.r[i];
}
static inline void m_sub_i(void) {};

static inline void micro_sub_ii(int i) {
  ram_env.r[0] -= ram_env.r[ram_env.r[i]];
}
static inline void m_sub_ii(void) {};

static inline void micro_mul_s(int s) {
  ram_env.r[0] *= s;
}
static inline void m_mul_s(void) {};

static inline void micro_mul_i(int i) {
  ram_env.r[0] *= ram_env.r[i];
}
static inline void m_mul_i(void) {};

static inline void micro_mul_ii(int i) {
  ram_env.r[0] *= ram_env.r[ram_env.r[i]];
}
static inline void m_mul_ii(void) {};

static inline void micro_div_s(int s) {
  if (!s) {
//    printf("Error: division by zero.\n");
    ram_env.state = RAM_DIVISION_BY_ZERO;
    return;
  }
  ram_env.r[0] /= s;
}
static inline void m_div_s(void) {};

static inline void micro_div_i(int i) {
  register int t = ram_env.r[i];
  if (!t) {
//    printf("Error: division by zero.\n");
    ram_env.state = RAM_DIVISION_BY_ZERO;
    return;
  }
  ram_env.r[0] /= t;
}
static inline void m_div_i(void) {};

static inline void micro_div_ii(int i) {
  register int t = ram_env.r[ram_env.r[i]];
  if (!t) {
//    printf("Error: division by zero.\n");
    ram_env.state = RAM_DIVISION_BY_ZERO;
    return;
  }
  ram_env.r[0] /= t;
}
static inline void m_div_ii(void) {};
