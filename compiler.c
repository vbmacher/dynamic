/*
 * compiler.c
 *
 * (c) Copyright 2010, P. Jakubco
 *
 * Lex analyzer, LL(1) parser and compiler of RAM programs.
 */

#include <stdio.h>
#include "compiler.h"

static FILE *fin;
static FILE *fout;
static int status;

static int TC[512]; /* table of constants */
static int ixtc=0;  /* index to free position in the constants table */
static char *TID[317];     /* table of identifiers (labels) */

static int lexval;  /* value for syntax analyzer */
static int sym;     /* input symbol - for syntax analyzer */
static int row =1;  /* actual row for lexical analyzer */

static void get(void);
void Row();
void Instruction();
void Label();
void Start();

static void error(char *s, SET K){
  printf("ERROR(%d): %s\n",row,s); /* error message */
  status = COMPILER_ERROR;
  while(!(sym & K))
    get(); /* jump over non-key symbols */
}

// this should be called before any branching
static void check(char *s, SET K){
  if(!(sym & K))
    error(s,K);
}

// compute hash from a text
int hash_index(char *text)
{
  int i,h=0;
  for (i=0; text[i] != '\0'; i++)
    h = (h * 36 + text[i]) % 317;
  return h;
}

// save identifier to the table of identifiers
int save_id(char *id)
{
  int ix;
  
  ix = hash_index(id);
  while (TID[ix] != NULL) {
	  if (!strcmp(id,TID[ix]))
      return ix;
	  ix = (ix + 17) % 317; /* 17 is a constant step, empirically k=sqrt(TIDmax) */
  }
  TID[ix] = (char*)malloc(strlen(id) + 1);
  strcpy(TID[ix], id);
  return ix;
}

// Lexical analyzer
static void get(void) {
  int c,i;
  int val;
  char ident[128]; /* identifier name */

  while(1) {
    c = fgetc(fin);
    switch(c) {
      case ';': // comment?
        do { c = fgetc(fin); }
        while ((c != '\n') && (c != EOF));
        ungetc(c,fin);
        break;
      case ' ' : break;
      case '\t': break;
      case '\n': row++; break;
      case EOF : sym = END; return;
      case '*' : sym = ASTERISK; return;
      case ':' : sym = COLON; return;
      case '=' : sym = EQUAL; return;
      default:
        if (isdigit(c)) { // NUMBER??
          i = 0;
          ident[i++] = c;
          do {
            c = fgetc(fin);
            ident[i++]=c;
          } while(isdigit(c));
        
				  ungetc(c,fin); /* !!! return the char to the input */
				  ident[i] = '\0';
				  val = atoi(ident);
				  sym = NUMBER;
          lexval = ixtc++;
          TC[lexval]=val;
          return;
        } else if(isalpha(c)) { // RESERVED WORD? or LABEL?
          i = 0;
          do {
            ident[i++] = toupper(c);
            c = fgetc(fin);
          } while(isalnum(c));
          ident[i] = '\0';
          ungetc(c,fin); /* return the char back to the input */

          // test for reserved words
          if (!strcmp(ident, "HALT")) sym = HALT;
          else if (!strcmp(ident, "READ")) sym = READ;
          else if (!strcmp(ident, "LOAD")) sym = LOAD;
          else if (!strcmp(ident, "WRITE")) sym = WRITE;
          else if (!strcmp(ident, "STORE")) sym = STORE;
          else if (!strcmp(ident, "ADD")) sym = ADD;
          else if (!strcmp(ident, "SUB")) sym = SUB;
          else if (!strcmp(ident, "MUL")) sym = MUL;
          else if (!strcmp(ident, "DIV")) sym = DIV;
          else if (!strcmp(ident, "JMP")) sym = JMP;
          else if (!strcmp(ident, "JGTZ")) sym = JGTZ;
          else if (!strcmp(ident, "JZ")) sym = JZ;
          else {
            sym = LABELTEXT;
            lexval = save_id(ident);
          }
          return;
        } else {
          printf("ERROR: Unknown symbol ('%c' = 0x%x)\n",c,c);
          status = COMPILER_ERROR;
        }
        break;
    }
  }
}

/************** Syntax analysis **********************************************/

// Start -> Row [Start];
void Start(SET K) {
  Row(F_Start|K);
  if (sym & F_Start)
    Start(F_Row|K);
}

// Row -> [Label] Instruction
void Row(SET K) {
  if (sym & F_Label)
    Label(F_Instruction|K);
  Instruction(K);
}

// Label -> LABELTEXT COLON
void Label(SET K) {
  if (sym == LABELTEXT)
    get();
  else
    error("Label text was expected!", COLON|K);
  
  if (sym == COLON)
    get();
  else
    error("Colon (':') was expected!",K);
}

void Instruction(SET K) {
  check("Expected somewhat else!",F_Instruction|FO_Instruction|NUMBER|K);
  switch(sym) {
    case HALT:
      get(); break;
    case READ: case STORE:
      get();
      if (sym == ASTERISK)
        get();
      if (sym == NUMBER)
        get();
      else
        error("Number was expected!",K);
      break;
    case WRITE: case LOAD: case ADD: case SUB:
    case MUL: case DIV:
      get();
      if (sym & (ASTERISK|EQUAL))
        get();
      if (sym == NUMBER)
        get();
      else
        error("Number was expected!",K);
      break;
    case JMP: case JGTZ: case JZ:
      if (sym == LABELTEXT)
        get();
      else
        error("Label text was expected!",K);
      break;
    default:
      error("Unknown instruction!",F_Instruction|FO_Instruction|NUMBER|K);
  }
}

// public function
int compile(const char *input, const char *output) {
  if ((fin = fopen(input,"rt")) == NULL) {
    printf("ERROR: Input file '%s' cannot be opened!\n", input);
    return;
  }
  if ((fout = fopen(output,"wt")) == NULL) {
    printf("ERROR: Output file '%s' cannot be opened!\n", output);
    return;
  }
  status = COMPILER_OK;
  get();
  Start(F_Start|END);
  
  return status;
}
