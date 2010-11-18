/**
 * dynarec.c
 *
 * (c) Copyright 2010, P. Jakubco <pjakubco@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "dynarec.h"
#include "cache.h"
#include "main.hpp"
#include "ram.h"

static int rsize[] = {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

static gen_code_struct icodes[] = {
  { "\x81\x05\0\0\0\0\0\0\0\0\xC7\x05\0\0\0\0\0\0\0\0\x5D\xC3", 22},
  { "\xA1\0\0\0\0\x03\x05\0\0\0\0\x0F\xBE\0\xA3\0\0\0\0\xFF\x05\0\0\0\0", 25},
  { "\xA1\0\0\0\0\x03\x05\0\0\0\0\x0F\xBE\0\x8B\x15\0\0\0\0\x89\x04\x95\0\0\0\0\xFF\x05\0\0\0\0", 33},
  { "\xA1\0\0\0\0\x83\xF8\0\x7C\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x11\x05\0\0\0\0\xC7\0\0\0\0\0\xFF\x05\0\0\0\0", 39},
  { "\xA1\0\0\0\0\x83\xF8\0\x7C\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x13\x8B\x15\0\0\0\0\x05\0\0\0\0\x89\x10\xFF\x05\0\0\0\0", 41},
  { "\xA1\0\0\0\0\x83\xF8\0\x7C\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x1A\x8B\x15\0\0\0\0\x8B\x14\x95\0\0\0\0\x05\0\0\0\0\x89\x10\xFF\x05\0\0\0\0", 48},
  { "\xC7\x05\0\0\0\0\0\0\0\0", 10},
  { "\xA1\0\0\0\0\xA3\0\0\0\0", 10},
  { "\xA1\0\0\0\0\x8B\x04\x85\0\0\0\0\xA3\0\0\0\0", 17},
  { "\xA1\0\0\0\0\xA3\0\0\0\0", 10},
  { "\xA1\0\0\0\0\x8B\x15\0\0\0\0\x89\x04\x95\0\0\0\0", 18},
  { "\x81\x05\0\0\0\0\0\0\0\0", 10},
  { "\x8B\x15\0\0\0\0\x01\x15\0\0\0\0", 12},
  { "\xA1\0\0\0\0\x8B\x04\x85\0\0\0\0\x01\x05\0\0\0\0", 18},
  { "\x81\x2D\0\0\0\0\0\0\0\0", 10},
  { "\x8B\x15\0\0\0\0\x29\x15\0\0\0\0", 12},
  { "\xA1\0\0\0\0\x8B\x04\x85\0\0\0\0\x29\x05\0\0\0\0", 18},
  { "\x69\x05\0\0\0\0\0\0\0\0\xA3\0\0\0\0", 15},
  { "\xA1\0\0\0\0\xF7\x2D\0\0\0\0\xA3\0\0\0\0", 16},
  { "\xA1\0\0\0\0\x8B\x04\x85\0\0\0\0\xF7\x2D\0\0\0\0\xA3\0\0\0\0", 23},
  { "\xBB\0\0\0\0\x81\xFB\0\0\0\0\x75\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x0D\xA1\0\0\0\0\x99\xF7\xFB\xA3\0\0\0\0", 38},
  { "\x8B\x1D\0\0\0\0\x81\xFB\0\0\0\0\x75\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x0D\xA1\0\0\0\0\x99\xF7\xFB\xA3\0\0\0\0", 39},
  { "\x8B\x1D\0\0\0\0\x8B\x1C\x9D\0\0\0\0\x81\xFB\0\0\0\0\x75\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x0D\xA1\0\0\0\0\x99\xF7\xFB\xA3\0\0\0\0", 46},
  { "\xC7\x05\0\0\0\0\0\0\0\0\xB8\0\0\0\0\xFF\xE0", 17},
  { "\x81\x05\0\0\0\0\0\0\0\0\x81\x3D\0\0\0\0\0\0\0\0\x7E\x11\xC7\x05\0\0\0\0\0\0\0\0\xB8\0\0\0\0\xFF\xE0", 39},
  { "\x81\x05\0\0\0\0\0\0\0\0\x81\x3D\0\0\0\0\0\0\0\0\x75\x11\xC7\x05\0\0\0\0\0\0\0\0\xB8\0\0\0\0\xFF\xE0", 39}
};

/**
 * This function creates runnable function for given block. It is the realisator
 * of the dynamic translator. The code is translated using predefined templates.
 *
 * @param block   - basic block containing the address of target code
 * @param program - original non-translated RAM program
 */
void dyn_translate(BASIC_BLOCK *block, const char *program, int ram_size) {
  register unsigned int target_size = 3, isize=0;
  unsigned char *ptr, *target, xcode;

  unsigned int tmp,i,addr,tmp2,tmp3;
  unsigned short icount = 0,tmp4;
  
  unsigned int ptr_max = ram_size + (unsigned int)program;
  unsigned int cache_size = cache_code_size-12; // the number of bytes of the procedure epilogue
  
  /* BEGINNING */
  ptr = (unsigned char*)((int)program + (int)block->address);
  target = (unsigned char *)block->code;
  
  *target++ = 0x55;   // "push %ebp"
  *target++ = 0x89;   // "mov %esp, %ebp"
  *target++ = 0xE5;
  
  xcode = *ptr++;
  isize = icodes[xcode].size;

  // buggy can be: ptr <= ptr_max
  while (isize && ((target_size+isize) < cache_size) && ((unsigned int)ptr <= ptr_max)) {
    memcpy(target, icodes[xcode].code, isize);
    icount += rsize[xcode]; // might be buggy

    switch (xcode) {
      case 0: // HALT
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.pc);
        *(unsigned int *)(target+6) = icount;
        *(unsigned int *)(target+12) = (unsigned int)(&ram_env.state);
        *(unsigned int *)(target+16) = (int)RAM_HALT;
        icount = 0;
        break;
      case 1: // READ i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_input);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.input);
        *(unsigned int *)(target+15) = (unsigned int)&ram_env.r[*ptr++];
        *(unsigned int *)(target+21) = (unsigned int)&ram_env.p_input;
        break;
      case 2: // READ *i 
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_input);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.input);
        *(unsigned int *)(target+16) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+23) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+29) = (unsigned int)(&ram_env.p_input);
        break;
      case 3: // WRITE =i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+23) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+29) = (unsigned int)*ptr++;
        *(unsigned int *)(target+35) = (unsigned int)&ram_env.p_output;
        break;
      case 4: // WRITE i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+24) = (unsigned int)&ram_env.r[*ptr++];
        *(unsigned int *)(target+29) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+37) = (unsigned int)&(ram_env.p_output);
        break;
      case 5: // WRITE *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+24) = (unsigned int)&ram_env.r[*ptr++];
        *(unsigned int *)(target+31) = (unsigned int)&ram_env.r[0];
        *(unsigned int *)(target+36) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+44) = (unsigned int)&(ram_env.p_output);
        break;
      case 6: // LOAD =i
        *(unsigned int *)(target+2) = (unsigned int)&ram_env.r[0];
        *(unsigned int *)(target+6) = (unsigned int)*ptr++;
        break;
      case 7: // LOAD i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+6) = (unsigned int)(&ram_env.r[0]);
        break;
      case 8: // LOAD *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+13) = (unsigned int)(&ram_env.r[0]);
        break;
      case 9: // STORE i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)(&ram_env.r[*ptr++]);
        break;
      case 10: // STORE *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        break;
      case 11: // ADD =i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*ptr++;
        break;
      case 12: // ADD i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        break;
      case 13: // ADD *i 
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        break;
      case 14: // SUB =i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*ptr++;
        break;
      case 15: // SUB i 
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        break;
      case 16: // SUB *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        break;
      case 17: // MUL =i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*ptr++;
        *(unsigned int *)(target+11) = (unsigned int)(&ram_env.r[0]);
        break;
      case 18: // MUL i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+12) = (unsigned int)(&ram_env.r[0]);
        break;
      case 19: // MUL *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+19) = (unsigned int)(&ram_env.r[0]);
        break;
      case 20: // DIV =i
        *(unsigned int *)(target+1) = (unsigned int)*ptr++;
        *(unsigned int *)(target+15) = (unsigned int)(&ram_env.state);
        *(unsigned int *)(target+19) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+26) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+34) = (unsigned int)(&ram_env.r[0]);
        break;
      case 21: // DIV i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+16) = (unsigned int)(&ram_env.state);
        *(unsigned int *)(target+20) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+27) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+35) = (unsigned int)(&ram_env.r[0]);
        break;
      case 22: // DIV *i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*ptr++]);
        *(unsigned int *)(target+9) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+23) = (unsigned int)(&ram_env.state);
        *(unsigned int *)(target+27) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+34) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+42) = (unsigned int)(&ram_env.r[0]);
        break;
      case 23: // JMP i
        // if the jump lies inside this cache block, 
        // then it is possible to do it. Considering forward references, too.
        tmp = *ptr++;        
        icount -= rsize[xcode];
        if (tmp >= block->address) {
          *(unsigned int *)(target+2) = (unsigned int)(&ram_env.pc);
          *(unsigned int *)(target+6) = tmp;
          icount = 0;
          // find address
          if (tmp <= ((int)ptr-(int)program)) {
            // from beginning
            addr = (unsigned int)block->code;
            i = block->address;
          } else {
            // from current position
            addr = (unsigned int)target + icodes[xcode].size - 3;
            i = (int)ptr-(int)program;
          }          
          tmp3 = icodes[xcode].size;
          tmp2 = target_size+tmp3;
          for (; (i < tmp) && (tmp2 < cache_size) && tmp3 && (i <= ptr_max); i+=rsize[program[i]]) {
            tmp3 = icodes[program[i]].size;
            addr += tmp3;
            tmp2 += tmp3;
          }
          
          // check
          if (i != tmp) {
            // address find failed
            ptr -= rsize[xcode];
            isize = 0;
            break;
          }
          // finally got it...
          *(unsigned int *)(target+11) = addr + 3;
        } else {
          ptr -= rsize[xcode];
          isize = 0;
        }
        break;
      case 24: // JGTZ i
        // if the jump lies inside this cache block, and its not forward
        // reference, then it is possible to do it
        tmp = *ptr++;        
        icount -= rsize[xcode];
        if (tmp >= block->address) {
          // at first - add to ram_env.pc previous instructions
          *(unsigned int *)(target+2) = (unsigned int)(&ram_env.pc);
          *(unsigned int *)(target+6) = icount;

          // do the jump
          *(unsigned int *)(target+12) = (unsigned int)(&ram_env.r[0]);
          *(unsigned int *)(target+24) = (unsigned int)(&ram_env.pc);
          *(unsigned int *)(target+28) = tmp;

          icount = 0;
          // find address
          if (tmp <= ((int)ptr-(int)program)) {
            // from beginning
            addr = (unsigned int)block->code;
            i = block->address;
          } else {
            // from current position
            addr = (unsigned int)target + icodes[xcode].size - 3;
            i = (int)ptr-(int)program;
          }          
          tmp3 = icodes[xcode].size;
          tmp2 = target_size+tmp3;
          for (; (i < tmp) && (tmp2 < cache_size) && tmp3 && (i <= ptr_max); i+=rsize[program[i]]) {
            tmp3 = icodes[program[i]].size;
            addr += tmp3;
            tmp2 += tmp3;
          }
          
          // check
          if (i != tmp) {
            // address find failed
            ptr -= rsize[xcode];
            isize = 0;
            break;
          }
          *(unsigned int *)(target+33) = addr + 3;          
        } else {
          ptr -= rsize[xcode];
          isize = 0;
        }
        break;
      case 25: // JZ i
        // if the jump lies inside this cache block, and its not forward
        // reference, then it is possible to do it
        tmp = *ptr++;
        icount -= rsize[xcode];
        
        if (tmp >= block->address) {
          // at first - add to ram_env.pc previous instructions
          *(unsigned int *)(target+2) = (unsigned int)(&ram_env.pc);
          *(unsigned int *)(target+6) = icount;

          // do the jump
          *(unsigned int *)(target+12) = (unsigned int)(&ram_env.r[0]);
          *(unsigned int *)(target+24) = (unsigned int)(&ram_env.pc);
          *(unsigned int *)(target+28) = tmp;
          
          icount = 0;
          // find address
          if (tmp <= ((int)ptr-(int)program)) {
            // from beginning
            addr = (unsigned int)block->code;
            i = block->address;
          } else {
            // from current position
            addr = (unsigned int)target + icodes[xcode].size - 3;
            i = (int)ptr-(int)program;
          }          
          tmp3 = icodes[xcode].size;
          tmp2 = target_size+tmp3;
          for (; (i < tmp) && (tmp2 < cache_size) && tmp3 && (i <= ptr_max); i+=rsize[program[i]]) {
            tmp3 = icodes[program[i]].size;
            addr += tmp3;
            tmp2 += tmp3;
          }
          
          // check
          if (i != tmp) {
            // address find failed
            ptr -= rsize[xcode];
            isize = 0;
            break;
          }
          *(unsigned int *)(target+33) = addr + 3;
        } else {
          ptr -= rsize[xcode];
          isize = 0;
        }
        break;        
      default: // other/unknown instruction
        ptr--;
        isize = 0;
        break;
    }

    target += isize;
    target_size += isize;

    xcode = *ptr++;
    isize = isize ? icodes[xcode].size : 0;
  };
  ptr--;
  
  // renew ram_env.pc
  *target++ = 0x81;
  *target++ = 0x05;
  *(unsigned int *)(target) = (unsigned int)(&ram_env.pc);
  target += 4;
  *(unsigned int *)(target) = icount;
  target += 4;
  target_size += 10;
  
  // "pop %ebp","ret"
  *target++ = 0x5D; *target++ = 0xC3;
  target_size += 2;
  block->size = (int)ptr - (int)program - (int)block->address;
  if (cmd_options & CMD_SUMMARY)
    printf("\t\tBlock translated with size: %d\n", block->size);
  if (cmd_options & CMD_SAVE_CODE) {
    char filename[30];
    if (code_filename != NULL) {
      strcpy((char*)&filename, code_filename);
      sprintf((char*)&filename+strlen(code_filename), "-%x.out", block->address);
    } else
      sprintf((char*)&filename, "code-%x.out", block->address);
    FILE *fout = fopen(filename ,"wb");
    fwrite(block->code, target_size, 1, fout);
    fclose(fout);
  }
}
