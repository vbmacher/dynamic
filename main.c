/**
 * main.c
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 * How to build
 * ^^^^^^^^^^^^
 *   - do not use code align
 *   - all microcode functions must have only 1 return instruction
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <getopt.h>

#define INPUT_CHARS 100

#define ERROR_MISSING_ARGS 1
#define ERROR_LOAD 2
#define OK 0

#define CACHE_BLOCKS 10
#define CACHE_CODE_SIZE 1024

typedef struct bBAS_BLOCK {
  int address;          // povodna adresa
  unsigned char *code;  // ?? 1K pre jeden blok
  int size;             // velkost povodnych instrukcii
} __attribute__((packed)) BASIC_BLOCK;

#define RAM_OK 0
#define RAM_UNKNOWN_INSTRUCTION 1
#define RAM_ADDRESS_FALLOUT 2
#define RAM_HALT 3
#define RAM_OUTPUT_FULL 4
#define RAM_DIVISION_BY_ZERO 5

#define RAM_OUTPUT_SIZE 100

unsigned int pc;            /* program counter */
unsigned int r[100];        /* registers */
char *input;       /* input tape */
char output[RAM_OUTPUT_SIZE];  /* output tape, max. 100 chars */
int p_input;       /* pointer to the next cell within the input tape */
int p_output;      /* pointer to the next cell within the output tape */
int state;        /* state of the RAM machine, used only in dyntran */

typedef struct {
  unsigned char* code;
  int size;
} gen_code_struct;


int ram_size = -1;  /* velkost RAM binarneho programu */

static BASIC_BLOCK *cache; /* Prekladova cache, to je zaklad. */
static int freeBlock = 0; /* index dalsieho volneho bloku*/

static unsigned char *program; // loaded program, corresponds with operating memory

gen_code_struct gen_codes[] = {
  { NULL, 0},
  { "\xA1\0\0\0\0\x03\x05\0\0\0\0\x0F\xBE\0\xA3\0\0\0\0\xFF\x05\0\0\0\0", 25},
  { "\xA1\0\0\0\0\x03\x05\0\0\0\0\x0F\xBE\0\x8B\x15\0\0\0\0\x89\x04\x95\0\0" \
    "\0\0\xFF\x05\0\0\0\0", 33},
  { "\xA1\0\0\0\0\x83\xF8\0\x7C\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x11\x05\0\0" \
    "\0\0\xC7\0\0\0\0\0\xFF\x05\0\0\0\0", 39},
  { "\xA1\0\0\0\0\x83\xF8\0\x7C\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x13\x8B\x15" \
    "\0\0\0\0\x05\0\0\0\0\x89\x10\xFF\x05\0\0\0\0", 41},
  { "\xA1\0\0\0\0\x83\xF8\0\x7C\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x1A\x8B\x15\0" \
    "\0\0\0\x8B\x14\x95\0\0\0\0\x05\0\0\0\0\x89\x10\xFF\x05\0\0\0\0", 48},
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
  { "\xBB\0\0\0\0\x81\xFB\0\0\0\0\x75\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB\x0D" \
    "\xA1\0\0\0\0\x99\xF7\xFB\xA3\0\0\0\0", 38},
  { "\x8B\x1D\0\0\0\0\x81\xFB\0\0\0\0\x75\x0C\xC7\x05\0\0\0\0\0\0\0\0\xEB" \
    "\x0D\xA1\0\0\0\0\x99\xF7\xFB\xA3\0\0\0\0", 39},
  { "\x8B\x1D\0\0\0\0\x8B\x1C\x9D\0\0\0\0\x81\xFB\0\0\0\0\x75\x0C\xC7\x05\0\0" \
    "\0\0\0\0\0\0\xEB\x0D\xA1\0\0\0\0\x99\xF7\xFB\xA3\0\0\0\0", 46},
  { NULL, 0},
  { NULL, 0},
  { NULL, 0}
};

void cache_flush();

/***************** BINARY IMAGE MANIPULATION *******************************/

/**
 * This function loads RAM program into memory.
 *
 * @param filename - file name of the RAM program
 * @return address of the loaded buffer
 */
void *bin_load(const char *filename) {
  FILE *fin;
  int size;
  void *buffer;
  
  if ((fin = fopen(filename, "r")) == NULL) {
    printf("Error: can't open file name '%s'.\n", filename);
    return NULL;
  }
  
  /* get approximate file size (resulting binary should be less than this) */
  fseek(fin, 0, SEEK_END);
	size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	
  buffer = calloc(1, size+1);
  if (buffer == NULL) {
    fclose(fin);
    printf("Error: can't allocate memory for the program.\n");
    return NULL;
  }
  
  ram_size = bin_parse(fin, buffer, size+1);
  fclose(fin);
  if (ram_size < 0) {
    free(buffer);
    printf("Error: can't parse the file.\n");
    return NULL;
  }
  return buffer;
}

/**
 * This function parses the file and convert it to binary file into the
 * buffer. Each RAM instruction is 1 byte long.
 *
 * @param fin - opened input file
 * @param buffer - destination buffer where to store the program
 * @param max_size - maximal loadable size
 * @return -1 if the parsing failed, positive value (binary size) if it is OK.
 */
int bin_parse(FILE *fin, unsigned char *buffer, const int max_size) {
  int p = 0;
  unsigned int t;
  while (!feof(fin)) {
    if (p == max_size-1) {
      printf("Warning: Ignoring some input\n");
      break;
    }
    fscanf(fin, "%u ", &t);
    buffer[p++] = (unsigned char)(t & 0xFF);
  }
  buffer[p] = 0;
  return p;
}

/****************** CACHE MANIPULATION ****************************/

/**
 * Funkcia inicializuje prekladovu cache - vytvori pamat pre N blokov.
 */
void cache_init() {
  cache = (BASIC_BLOCK *)malloc(CACHE_BLOCKS * sizeof(BASIC_BLOCK));
  cache_flush();
  freeBlock = 0;
}

/**
 * Tato funkcia uvolni cache.
 */
void cache_destroy() {
  free(cache);
}

/**
 * Vyprazdni cache. Vyprazdnuje sa vtedy, ked je plna.
 */
void cache_flush() {
  memset(cache, 0, CACHE_BLOCKS * sizeof(BASIC_BLOCK));
  freeBlock = 0;
}

/**
 * Funkcia na zaklade adresy najde dany blok.
 *
 * @param address - Adresa povodneho programu, na ktorej ma zacinat blok.
 * @return NULL ak sa blok nenasiel, inak blok.
 */
BASIC_BLOCK *cache_get_block(int address) {
  register int i;
  for (i = 0; i < freeBlock; i++) {
    if (cache[i].address == address)
      return &cache[i];
  }
  return NULL;
}

/**
 * Tato funkcia vytvori blok, ak sa da.
 *
 * @param address - Adresa povodneho programu, na ktorej zacina blok
 * @return blok ak sa to podarilo, inak NULL.
 */
BASIC_BLOCK *cache_create_block(int address) {
  if (freeBlock >= CACHE_BLOCKS)
    return NULL;
  cache[freeBlock].address = address;
  cache[freeBlock++].code = (unsigned char *)calloc(1, CACHE_CODE_SIZE);
  return &cache[freeBlock-1];
}

/*********************************** RAM MANIPULATION ***********************/

/**
 * Tato funkcia inicializuje staticky stav RAM stroja.
 */
void ram_init(int input_size) {
  pc = 0;
  memset(r, 0, 100);
  input = (char *)calloc(1, input_size);
  memset(output, 0, RAM_OUTPUT_SIZE);
  p_input = 0;
  p_output = 0;
  state = RAM_OK;
}

/**
 * Uvolni pamat.
 */
void ram_destroy() {
  free(input);
}

/**
 * Tato funkcia vypise vystupnu pasku na obrazovku.
 */
void ram_output() {
  int stop = p_output;
  int i;
  
  for (i = 0; i < stop; i++)
    printf("%d ", output[i]);
  printf("\n");
}


/**
 * Tato funkcia interpretuje jednu instrukciu. Teda ide o interpretator.
 * Predpoklada sa, ze semantika aj syntax je spravna.
 *
 * @param program - pamat s programom
 * @return chybovy kod
 */
int ram_interpret(const char *program) {
  int c,t;

  if (ram_size < 0) {
    printf("Error: RAM program was not loaded properly.");
    return 1;
  }

  if (pc >= ram_size) {
    printf("Error: Address fallout.\n");
    return 2;
  }

  c = program[pc++];
  if ((c > 0) && (pc >= ram_size)) {
    printf("Error: Address fallout.\n");
    return 2;
  }

  switch (c) {
    case 0: return RAM_HALT;
    case 23: /* JMP i */
      pc = program[pc]; break;
    case 24: /* JGTZ i */
      pc = (r[0] > 0) ? program[pc] : pc+1;
      break;
    case 25: /* JZ i */
      pc = (r[0] == 0) ? program[pc] : pc+1;
      break;
    default:
      printf("Error: unknown insruction.\n");
      return RAM_UNKNOWN_INSTRUCTION;
  }
  return RAM_OK;
}

/**************** DYNAMIC TRANSLATOR ******************************/

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
        *(unsigned int *)(target+1) = (unsigned int)(&p_input);
        *(unsigned int *)(target+7) = (unsigned int)(&input);
        *(unsigned int *)(target+15) = (unsigned int)&r[*p++];
        *(unsigned int *)(target+21) = (unsigned int)&p_input;
        break;
      case 2: // READ *i 
        *(unsigned int *)(target+1) = (unsigned int)(&p_input);
        *(unsigned int *)(target+7) = (unsigned int)(&input);
        *(unsigned int *)(target+16) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+23) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+29) = (unsigned int)(&p_input);
        break;
      case 3: // WRITE =i
        *(unsigned int *)(target+1) = (unsigned int)(&p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+23) = (unsigned int)&output;
        *(unsigned int *)(target+29) = (unsigned int)*p++;
        *(unsigned int *)(target+35) = (unsigned int)&p_output;
        break;      
      case 4: // WRITE i
        *(unsigned int *)(target+1) = (unsigned int)(&p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+24) = (unsigned int)&r[*p++];
        *(unsigned int *)(target+29) = (unsigned int)&output;
        *(unsigned int *)(target+37) = (unsigned int)&(p_output);
        break;
      case 5: // WRITE *i
        *(unsigned int *)(target+1) = (unsigned int)(&p_output);
        *(target+7) = (unsigned char)(RAM_OUTPUT_SIZE);
        *(unsigned int *)(target+12) = (unsigned int)&state;
        *(unsigned int *)(target+16) = (unsigned int)RAM_OUTPUT_FULL;
        *(unsigned int *)(target+24) = (unsigned int)&r[*p++];
        *(unsigned int *)(target+31) = (unsigned int)&r[0];
        *(unsigned int *)(target+36) = (unsigned int)&output;
        *(unsigned int *)(target+44) = (unsigned int)&(p_output);
        break;
      case 6: // LOAD =i
        *(unsigned int *)(target+2) = (unsigned int)&r[0];
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        break;
      case 7: // LOAD i
        *(unsigned int *)(target+1) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+6) = (unsigned int)(&r[0]);
        break;
      case 8: // LOAD *i
        *(unsigned int *)(target+1) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+13) = (unsigned int)(&r[0]);
        break;
      case 9: // STORE i
        *(unsigned int *)(target+1) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+6) = (unsigned int)(&r[*p++]);
        break;
      case 10: // STORE *i
        *(unsigned int *)(target+1) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+7) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+14) = (unsigned int)(&r[0]);
        break;
      case 11: // ADD =i
        *(unsigned int *)(target+2) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        break;
      case 12: // ADD i
        *(unsigned int *)(target+2) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&r[0]);
        break;
      case 13: // ADD *i 
        *(unsigned int *)(target+1) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&r[0]);
        break;
      case 14: // SUB =i
        *(unsigned int *)(target+2) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        break;
      case 15: // SUB i 
        *(unsigned int *)(target+2) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&r[0]);
        break;
      case 16: // SUB *i
        *(unsigned int *)(target+1) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&r[0]);
        break;
      case 17: // MUL =i
        *(unsigned int *)(target+2) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+6) = (unsigned int)*p++;
        *(unsigned int *)(target+11) = (unsigned int)(&r[0]);
        break;
      case 18: // MUL i
        *(unsigned int *)(target+1) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+7) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+12) = (unsigned int)(&r[0]);
        break;
      case 19: // MUL *i
        *(unsigned int *)(target+1) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+8) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+14) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+19) = (unsigned int)(&r[0]);
        break;
      case 20: // DIV =i
        *(unsigned int *)(target+1) = (unsigned int)*p++;
        *(unsigned int *)(target+15) = (unsigned int)(&state);
        *(unsigned int *)(target+19) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+26) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+34) = (unsigned int)(&r[0]);
        break;
      case 21: // DIV i
        *(unsigned int *)(target+2) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+16) = (unsigned int)(&state);
        *(unsigned int *)(target+20) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+27) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+35) = (unsigned int)(&r[0]);
        break;
      case 22: // DIV *i
        *(unsigned int *)(target+2) = (unsigned int)(&r[*p++]);
        *(unsigned int *)(target+9) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+23) = (unsigned int)(&state);
        *(unsigned int *)(target+27) = (unsigned int)RAM_DIVISION_BY_ZERO;
        *(unsigned int *)(target+34) = (unsigned int)(&r[0]);
        *(unsigned int *)(target+42) = (unsigned int)(&r[0]);
        break;
      default: // other/unknown instruction
        p--;
        break;
    }
    target += micro_size;
    a_size += micro_size;

    xcode = *p++;
    micro_size = gen_codes[xcode].size;
  };
  p--;
  
  // "pop %ebp","ret"
  *target++ = 0x5D; *target++ = 0xC3;
  block->size = p - program - block->address;
}


int main(int argc, char *argv[])
{
  int status;
  BASIC_BLOCK *tmp;
  char input_str[INPUT_CHARS];

  // chyba dalsi operand
  if (argc < 2) {
    printf("Usage: dynamic path/to/ram/program\n");
    return ERROR_MISSING_ARGS;
  }

  /* load RAM program */
  program = (unsigned char *)bin_load(argv[1]);

  if (program == NULL)
    return ERROR_LOAD;

  ram_init(INPUT_CHARS);
  cache_init();
  
  printf("Enter input tape (max. %d chars):", INPUT_CHARS);
  scanf("%s", input_str);

  // convert the input tape into binary form
  int i,l=strlen(input_str);
  for (i = 0; i < l; i++)
    input[i] = input_str[i]-'0';

  do {    
    tmp = cache_get_block(pc);
    if (tmp == NULL) {
      if ((tmp = cache_create_block(pc)) == NULL) {
        cache_flush();
        tmp = cache_create_block(pc);
      }
      dyn_translate(tmp, program);
    }
    if ((tmp != NULL) && (tmp->size > 0)) {
      (*(void (*)())tmp->code)(); // translated code run
      pc += tmp->size;
    } else
      state = ram_interpret(program);
  } while ((state == RAM_OK) && (pc < ram_size));
  
  printf("\nOutput tape:\n\t");
  ram_output();
  
  cache_destroy();
  ram_destroy();
  free(program);

  return OK;
}
