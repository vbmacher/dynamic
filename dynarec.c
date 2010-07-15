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
#include "main.h"

char *names[] = {"HALT","READ i","READ *i","WRITE =i","WRITE i","WRITE *i",
                 "LOAD =i","LOAD i","LOAD *i","STORE i","STORE *i","ADD =i",
                 "ADD i","ADD *i","SUB =i","SUB i","SUB *i","MUL =i","MUL i",
                 "MUL *i","DIV =i","DIV i","DIV *i","JMP i","JGTZ i","JZ i",NULL};

gen_code_struct gen_codes[] = {
  { NULL, 0},
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
  { NULL, 0},
  { NULL, 0},
  { NULL, 0}
};

/**
 * Tato funkcia "prilepi" kod-sablonu na koniec vysledneho kodu. Sablonovy
 * kod "oblepi" instrukciami, ktore ulozia do zasobnika parameter, ako sa uklada
 * pri normalnom volani funkcie. Dalej "odreze" prvu instrukciu (tj. "push ebp")
 * a poslednych @leave bytov (zvycajne 2 byty - "pop ebp"+"ret")
 *
 * @param target - cielovy kod
 * @param param  - parameter, ktory sa ma ulozit do zasobnika
 * @param f1     - kod-sablona jednej instrukcie
 * @param size   - velkost kodu-sablony
 * @param leave  - velkost bytov, ktore sa maju "odrezat" z konca
 * @return Skutocna velkost vygenerovaneho kodu v bytoch
 */
inline int dyn_gen_param(unsigned char *target, unsigned char param,
    unsigned char *f1, int size, int leave) {
  register unsigned int micro_size;

  if (cmd_options & CMD_SUMMARY)
    printf("\t\t\tGenerating: %x -> %x, size=%d, param=%d\n", f1, target, size,param);

  // vlozim parameter do zasobnika (int, 4 byty)
  *target++ = 0x6A;
  *target++ = param; //asm("push $0x06");

  // posuniem zasobnik (akoze "call" + "push ebp") 
  *target++ = 0x83; // asm("sub $0x8,%esp");
  *target++ = 0xEC;
  *target++ = 0x08;

  micro_size = size - 1; // vynechavam "push %ebp"

  if (cmd_options & CMD_SUMMARY)
    printf("\t\t\tCopying: %x -> %x, size=%d...\n", f1+1, target, micro_size);
  memcpy(target, f1+1, micro_size);
  target += (micro_size-leave); // vynechavam "pop %ebp" a "ret"

  // vratim spat zasobnik (akoze "pop ebp" + "ret")
  *target++ = 0x83;
  *target++ = 0xC4;
  
  //asm("add $0xXX,%esp");
  // ak (leave == 1) tak predpokladam ze kod-sablona nema na konci "pop ebp" (4 byty),
  // ale instrukciu "leave", ktora zo zasobnika odstrani tie 4 byty (a plus nejake
  // dalsie, lebo ked gcc pouzije instrukciu "leave" znamena to ze pouzival lokalne
  // premenne, ktore su tiez ulozene v zasobniku).
  *target++ = (unsigned char)(8 + ((leave == 2) ? 4 : 0)); 

  if (cmd_options & CMD_SUMMARY)
    printf("\t\t\tFinal code size: %d\n", micro_size+8-leave);

  return micro_size + 8 - leave;
}


/**
 * Tato funkcia vytvori spustatelnu funkciu pre dany blok. Je realizatorom
 * dynamickeho prekladaca. Kod preklada priamo bez predprogramovanych sablon.
 *
 * @param block   - zakladny blok obsahujuci adresu cieloveho kodu
 * @param program - adresa povodneho neprelozeneho programu
 */
void dyn_translate(BASIC_BLOCK *block, unsigned char *program) {
  register int a_size = 3, micro_size=0;
  unsigned char *p, xcode;
  unsigned char *target;
  
  unsigned int overrun = ram_size + (unsigned int)program;
  unsigned int cache_size = CACHE_CODE_SIZE-2;
  
  /* zaciname */
  p = program + block->address;
  target = (unsigned char *)block->code;
  
  *target++ = 0x55;   // "push %ebp"
  *target++ = 0x89;   // "mov %esp, %ebp"
  *target++ = 0xE5;
  
  xcode = *p++;
  micro_size = gen_codes[xcode].size;

  while (micro_size && (a_size < cache_size) && ((unsigned int)p < overrun)) {
    memcpy(target, gen_codes[xcode].code, micro_size);

    switch (xcode) {
      case 1: // READ i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_input);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.input);
        *(unsigned int *)(target+15) = (unsigned int)&ram_env.r[*p++];
        *(unsigned int *)(target+21) = (unsigned int)&ram_env.p_input;
        break;
      case 2: // READ *i 
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_input);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.input);
        *(unsigned int *)(target+16) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+23) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+29) = (unsigned int)(&ram_env.p_input);
        break;
      case 3: // WRITE =i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+23) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+29) = (unsigned int)*p++;
        *(unsigned int *)(target+35) = (unsigned int)&ram_env.p_output;
        break;      
      case 4: // WRITE i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+24) = (unsigned int)&ram_env.r[*p++];
        *(unsigned int *)(target+29) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+37) = (unsigned int)&(ram_env.p_output);
        break;
      case 5: // WRITE *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&ram_env.state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+24) = (unsigned int)&ram_env.r[*p++];
        *(unsigned int *)(target+31) = (unsigned int)&ram_env.r[0];
        *(unsigned int *)(target+36) = (unsigned int)&ram_env.output;
        *(unsigned int *)(target+44) = (unsigned int)&(ram_env.p_output);
        break;
      case 6: // LOAD =i
        *(unsigned int *)(target+2) = (unsigned int)&ram_env.r[0];
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        break;
      case 7: // LOAD i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+6) = (unsigned int)(&ram_env.r[0]);
        break;
      case 8: // LOAD *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+13) = (unsigned int)(&ram_env.r[0]);
        break;
      case 9: // STORE i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)(&ram_env.r[*p++]);
        break;
      case 10: // STORE *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        break;
      case 11: // ADD =i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        break;
      case 12: // ADD i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        break;
      case 13: // ADD *i 
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        break;
      case 14: // SUB =i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        break;
      case 15: // SUB i 
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        break;
      case 16: // SUB *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        break;
      case 17: // MUL =i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        *(unsigned int *)(target+11) = (unsigned int)(&ram_env.r[0]);
        break;
      case 18: // MUL i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+7) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+12) = (unsigned int)(&ram_env.r[0]);
        break;
      case 19: // MUL *i
        *(unsigned int *)(target+1) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+19) = (unsigned int)(&ram_env.r[0]);
        break;
      case 20: // DIV =i
        *(unsigned int *)(target+1) = (unsigned int)*p++;
        *(unsigned int *)(target+15) = (unsigned int)(&ram_env.state);
        *(unsigned int *)(target+19) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+26) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+34) = (unsigned int)(&ram_env.r[0]);
        break;
      case 21: // DIV i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+16) = (unsigned int)(&ram_env.state);
        *(unsigned int *)(target+20) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+27) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+35) = (unsigned int)(&ram_env.r[0]);
        break;
      case 22: // DIV *i
        *(unsigned int *)(target+2) = (unsigned int)(&ram_env.r[*p++]);
        *(unsigned int *)(target+9) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+23) = (unsigned int)(&ram_env.state);
        *(unsigned int *)(target+27) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+34) = (unsigned int)(&ram_env.r[0]);
        *(unsigned int *)(target+42) = (unsigned int)(&ram_env.r[0]);
        break;
      default: // other/unknown instruction
        p--;
        break;
    }
    target += micro_size;
    a_size += micro_size;

    xcode = *p++;
//    if (cmd_options & CMD_SUMMARY)
  //    printf("\t\t%s\n", names[xcode]);
    micro_size = gen_codes[xcode].size;
  };
  p--;
  
  // "pop %ebp","ret"
  *target++ = 0x5D; *target++ = 0xC3;
  block->size = p - program - block->address;
    if (cmd_options & CMD_SUMMARY)
      printf("\t\tBlock translated with size: %d\n", block->size);
 /*   if (cmd_options & CMD_SAVE_CODE) {
      char filename[30];
      if (code_filename != NULL) {
          strcpy((char*)&filename, code_filename);
          sprintf((char*)&filename+strlen(code_filename), "-%x.out", block->address);
      } else
          sprintf((char*)&filename, "code-%x.out", block->address);
      FILE *fout = fopen(filename ,"wb");
      fwrite(block->code, a_size+2, 1, fout);
      fclose(fout);
    }
*/
}

/**
 * Tato funkcia vytvori spustatelnu funkciu pre dany blok. Je realizatorom
 * dynamickeho prekladaca. Pouziva kod-sablony.
 *
 * @param block   - zakladny blok obsahujuci adresu cieloveho kodu
 * @param program - adresa povodneho neprelozeneho programu
 */
void dyn_template(BASIC_BLOCK *block, unsigned char *program) {
  register int a_size = 3, micro_size=0;
  unsigned char *p;
  unsigned char *target;
  
  /* zaciname */
  p = program + block->address;
  target = (unsigned char *)block->code;
  
  *target++ = 0x55;   // "push %ebp"
  *target++ = 0x89;   // "mov %esp, %ebp"
  *target++ = 0xE5;
  
  do {
    if (cmd_options & CMD_SUMMARY)
      printf("\t\t%s\n", names[*p]);
    switch (*p++) {
      case 1: // READ i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_read_i, M_READ_I,2);
        break;
      case 2: // READ *i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_read_ii, M_READ_II,2);
        break;
      case 3: // WRITE =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_s, M_WRITE_S,2);
        break;      
      case 4: // WRITE i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_i, M_WRITE_I,2);
        break;
      case 5: // WRITE *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_write_ii, M_WRITE_II,2);
        break;
      case 6: // LOAD =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_s, M_LOAD_S,2);
        break;
      case 7: // LOAD i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_i, M_LOAD_I,2);
        break;
      case 8: // LOAD *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_load_ii, M_LOAD_II,2);
        break;
      case 9: // STORE i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_store_i, M_STORE_I,2);
        break;
      case 10: // STORE *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_store_ii, M_STORE_II,2);
        break;
      case 11: // ADD =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_s, M_ADD_S,2);
        break;
      case 12: // ADD i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_i, M_ADD_I,2);
        break;
      case 13: // ADD *i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_add_ii, M_ADD_II,2);
        break;
      case 14: // SUB =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_sub_s, M_SUB_S,2);
        break;
      case 15: // SUB i 
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_sub_i, M_SUB_I,2);
        break;
      case 16: // SUB *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_sub_ii, M_SUB_II,2);
        break;
      case 17: // MUL =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_mul_s, M_MUL_S,2);
        break;
      case 18: // MUL i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_mul_i, M_MUL_I,2);
        break;
      case 19: // MUL *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_mul_ii, M_MUL_II,2);
        break;
      case 20: // DIV =i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_div_s, M_DIV_S,2);
        break;
      case 21: // DIV i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_div_i, M_DIV_I,1);
        break;
      case 22: // DIV *i
        micro_size = dyn_gen_param(target, *p++, (unsigned char*)&micro_div_ii, M_DIV_II,1);
        break;
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
    *target++ = 0x5D; *target++ = 0xC3;
    block->size = p - program - block->address;
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
      fwrite(block->code, a_size+2, 1, fout);
      fclose(fout);
    }
  } else 
    block->size = 0;
}

