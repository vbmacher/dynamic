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
  register int a_size = 3, micro_size=0, tmp = 0;
  unsigned char *p;
  unsigned char *target;
  
  /* zaciname */
  p = program + block->address;
  target = (unsigned char *)block->code;
  
  *target++ = 0x55;   // "push %ebp"
  *target++ = 0x89;   // "mov %esp, %ebp"
  *target++ = 0xE5;
  
  do {
    switch (*p++) {
      case 1: // READ i
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_input);
        *(target+5) = 0x03;
        *(target+6) = 0x05;
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.input);
        *(unsigned int *)(target+11) = 0xA300BE0F;
        *(unsigned int *)(target+15) = (unsigned int)&ram_env.r[*p++];
        *(target+19) = 0xFF;
        *(target+20) = 0x05;
        *(unsigned int *)(target+21) = (unsigned int)&(ram_env.p_input);        
        micro_size = 25;
    //    micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_read_i, M_READ_I);
        break;
      case 2: // READ *i 
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_input);
        *(target+5) = 0x03;
        *(target+6) = 0x05;
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.input);
        *(unsigned int *)(target+11) = 0x8B00BE0F;
        *(target+15) = 0x15;
        *(unsigned int *)(target+16) = (unsigned int)(&ram_env.r[*p++]);
        *(target+20) = 0x89;
        *(target+21) = 0x04;
        *(target+22) = 0x95;
        *(unsigned int *)(target+23) = (unsigned int)(&ram_env.r[0]);
        *(target+27) = 0xFF;
        *(target+28) = 0x05;
        *(unsigned int *)(target+29) = (unsigned int)(&ram_env.p_input);
        micro_size = 33;
    //    micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_read_ii, M_READ_II);
        break;
      case 3: // WRITE =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_s, M_WRITE_S);
        break;      
      case 4: // WRITE i
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+5) = 0x83;
        *(target+6) = 0xF8;
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+8) = 0x05C70C7C;
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+20) = 0x158B13EB;
        *(unsigned int *)(target+24) = (unsigned int)&ram_env.r[*p++];
        *(target+28) = 0x05;
        *(unsigned int *)(target+29) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+33) = 0x05FF1089;
        *(unsigned int *)(target+37) = (unsigned int)&(ram_env.p_output);
        micro_size = 41;
  //      micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_i, M_WRITE_I);
        break;
      case 5: // WRITE *i
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+5) = 0x83;
        *(target+6) = 0xF8;
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+8) = 0x05C70C7C;
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+20) = 0x158B1AEB;
        *(unsigned int *)(target+24) = (unsigned int)&ram_env.r[*p++];
        *(target+28) = 0x8B;
        *(target+29) = 0x14;
        *(target+30) = 0x95;
        *(unsigned int *)(target+31) = (unsigned int)&ram_env.r[0];
        *(target+35) = 0x05;
        *(unsigned int *)(target+36) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+40) = 0x05FF1089;
        *(unsigned int *)(target+44) = (unsigned int)&(ram_env.p_output);
        micro_size = 48;
      //  micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_ii, M_WRITE_II);
        break;
      case 6: // LOAD =i
        *target = 0xC7;
        *(target+1) = 0x05;
        *(unsigned int *)(target+2) = (unsigned int)&ram_env.r[0];
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        micro_size = 10;
//        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_s, M_LOAD_S);
        break;
      case 7: // LOAD i
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(target+5) = 0xA3;
        *(unsigned int *)(target+6) = (unsigned int)(&ram_env.r[0]);
        micro_size = 10;
//        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_i, M_LOAD_I);
        break;
      case 8: // LOAD *i
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(target+5) = 0x8B;
        *(target+6) = 0x04;
        *(target+7) = 0x85;
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(target+12) = 0xA3;
        *(unsigned int *)(target+13) = (unsigned int)(&ram_env.r[0]);
        micro_size = 17;
   //     micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_ii, M_LOAD_II);
        break;
      case 9: // STORE i
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[0]);
        *(target+5) = 0xA3;
        *(unsigned int *)(target+6) = (unsigned int)(&ram_env.r[*p++]);
        micro_size = 10;
//        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_store_i, M_STORE_I);
        break;
      case 10: // STORE *i
        *target = 0xA1;
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[0]);
        *(target+5) = 0x8B;
        *(target+6) = 0x15;
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.r[*p++]);
        *(target+11) = 0x89;
        *(target+12) = 0x04;
        *(target+13) = 0x95;
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        micro_size = 18;
//        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_store_ii, M_STORE_II);
        break;
      case 11: // ADD =i
        *target = 0x81;
        *(target+1) = 0x05;
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        micro_size = 10;
//        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_s, M_ADD_S);
        break;
      case 12: // ADD i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_i, M_ADD_I);
        break;
      case 13: // ADD *i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_ii, M_ADD_II);
        break;
      case 14: // SUB =i
        *target = 0x81;
        *(target+1) = 0x2D;
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        micro_size = 10;
//        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_sub_s, M_SUB_S);
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

  if (a_size > 3) {
    // "pop %ebp","ret"
    char filename[20];
    *target++ = 0x5D; *target++ = 0xC3;
    block->size = p - program - block->address;
    //printf("\t\tBlock translated with size: %d...\n", block->size);
    sprintf(&filename, "code-%x.out", block->address);
    FILE *fout = fopen(filename ,"wb");
    fwrite(block->code, a_size+2, 1, fout);
    fclose(fout);
  } else 
    block->size = 0;
}


