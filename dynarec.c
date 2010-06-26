/**
 * dynarec.c
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "dynarec.h"
#include "cache.h"
#include "bin.h"
#include "microcode.h"

inline int dyn_gen_param(unsigned char *target, unsigned char param,
    unsigned char *f1, int size) {
  register unsigned int micro_size;

  // vlozim parameter do zasobnika (int, 4 byty)
  *target++ = 0x6A;
  *target++ = param; //asm("push $0x06");

  // posuniem zasobnik (akoze "call" + "push ebp") 
  *target++ = 0x83; // asm("sub $0x8,%esp");
  *target++ = 0xEC;
  *target++ = 0x08;

  micro_size = size - 1; // vynechavam "push %ebp"

  memcpy(target, f1+1, micro_size);
  target += (micro_size-2); // vynechavam "pop %ebp" a "ret"

  // vratim spat zasobnik (akoze "pop ebp" + "ret")
  *target++ = 0x83;
  *target++ = 0xC4;
  *target++ = 0x0C; //asm("add $0x09,%esp"); // 1 byte je v zasobniku (miesto "pop esp")

  return micro_size + 6;
}


/**
 * Tato funkcia vytvori spustatelnu funkciu pre dany blok. Je realizatorom
 * dynamickeho prekladaca.
 */
void dyn_translate(BASIC_BLOCK *block, unsigned char *program) {
  register int a_size = 1, micro_size=0, tmp = 0;
  unsigned char *p;
  unsigned char *target;
  
  /* zaciname */
  p = program + block->address;
  target = (unsigned char *)block->code;
  
  // "push %ebp"
  *target++ = 0x55;
  
  do {
    switch (*p++) {
      case 1: // READ i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_read_i, M_READ_I);
        break;
      case 2: // READ *i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_read_ii, M_READ_II);
        break;
      case 3: // WRITE =i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_s, M_WRITE_S);
        break;
      case 4: // WRITE i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_i, M_WRITE_I);
        break;
      case 5: // WRITE *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_ii, M_WRITE_II);
        break;
      case 6: // LOAD =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_s, M_LOAD_S);
        break;
      case 7: // LOAD i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_i, M_LOAD_I);
        break;
      case 8: // LOAD *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_ii, M_LOAD_II);
        break;
      case 9: // STORE i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_store_i, M_STORE_I);
        break;
      case 10: // STORE *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_store_ii, M_STORE_II);
        break;
      case 11: // ADD =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_s, M_ADD_S);
        break;
      case 12: // ADD i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_i, M_ADD_I);
        break;
      case 13: // ADD *i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_ii, M_ADD_II);
        break;
      case 14: // SUB =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_sub_s, M_SUB_S);
        break;
      case 15: // SUB i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_sub_i, M_SUB_I);
        break;
      case 16: // SUB *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_sub_ii, M_SUB_II);
        break;
      case 17: // MUL =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_mul_s, M_MUL_S);
        break;
      case 18: // MUL i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_mul_i, M_MUL_I);
        break;
      case 19: // MUL *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_mul_ii, M_MUL_II);
        break;
      case 20: // DIV =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_div_s, M_DIV_S);
        break;
      case 21: // DIV i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_div_i, M_DIV_I);
        break;
      case 22: // DIV *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_div_ii, M_DIV_II);
        break;
/*      case 23: // JMP i 
        tmp = dyn_gen_param(target, *p++, (unsigned char*)&micro_jmp_i, M_JMP_I);
        micro_size = 0; target += tmp; a_size += tmp;
        break;
      case 24: // JGTZ i
        tmp = dyn_gen_param(target, *p++, (unsigned char*)&micro_jgtz_i, M_JGTZ_I);
        micro_size = 0; target += tmp; a_size += tmp;
        break;
      case 25: // JZ i 
        tmp = dyn_gen_param(target, *p++, (unsigned char*)&micro_jz_i, M_JZ_I);
        micro_size = 0; target += tmp; a_size += tmp;
        break;*/
      default: // other/unknown instruction
        micro_size = 0;
        p--;
        break;
    }
    target += micro_size;
    a_size += micro_size;
  } while (micro_size && (a_size < (CACHE_CODE_SIZE-2)) && ((p-program) < ram_size));

  if (a_size > 1) {
    // "pop %ebp","ret"
    *target++ = 0x5D; *target++ = 0xC3;
    block->size = p - program - block->address;
    //printf("\t\tBlock translated with size: %d...\n", block->size);
  } else 
    block->size = 0;
}


