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
static int status;

int TC[512]; /* table of constants */
int ixtc=0;  /* index to free position in the constants table */

int lexval;  /* value for syntax analyzer */
int sym;     /* input symbol - for syntax analyzer */

void get(void);
void Row();
void Instruction();
void Label();
void Start();


void error(char *s, SET K){
  printf("ERROR: %s\n",s); /* error message */
  status = COMPILER_ERROR;
  while(!(sym & K))
    get(); /* jump over non-key symbols */
}

// this should be called before any branching (if, switch, for, while)
void check(char *s, SET K){
  if(!(sym & K))
    error(s,K);
}

// Lexical analyzer
void get(void) {
  int c,i;
  int val;
  char ident[128]; /* identifier name */

  while(1) {
    c = fgetc(fin);
    switch(c) {
      case ';': // comment?
        do { c = fgetc(fin); } while (c != '\n');
        break;
      case ' ' : break;
      case '\t': break;
      case '\n': break;
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
void Start() {
  Row();
  if (sym & F_Start)
    Start();
}

// Row -> [Label] Instruction
void Row() {
  if (sym & F_Label)
    Label();
  Instruction();
}

// Label -> LABELTEXT COLON
void Label() {
  if (sym == LABELTEXT)
    get();
  else {
    printf("ERROR: Label text was expected!\n");
    status = COMPILER_ERROR;
  }
  
  if (sym == COLON)
    get();
  else {
    printf("ERROR: Colon (':') was expected!\n");
    status = COMPILER_ERROR;
  }
}

/*
Instruction -> HALT                            | 
               READ  [ASTERISK]         Number |
               WRITE [ASTERISK | EQUAL] Number |
               LOAD  [ASTERISK | EQUAL] Number |
               STORE [ASTERISK]         Number |
               ADD   [ASTERISK | EQUAL] Number |
               SUB   [ASTERISK | EQUAL] Number |
               MUL   [ASTERISK | EQUAL] Number |
               DIV   [ASTERISK | EQUAL] Number |
               JMP  LABELTEXT                  |
               JGTZ LABELTEXT                  |
               JZ   LABELTEXT
*/
void Instruction() {
  if (sym == HALT) {
    get();
  } else if (sym & (READ|STORE)) {
    get();
    
    if (sym == ASTERISK)
      get();
    
    if (sym == NUMBER)
      get();
    else {
      printf("ERROR: Number was expected!\n");
      status = COMPILER_ERROR;
    }
  } else if (sym & (WRITE|LOAD|ADD|SUB|MUL|DIV)) {
    get();
    
    if (sym & (ASTERISK|EQUAL))
      get();
    
    if (sym == NUMBER)
      get();
    else {
      printf("ERROR: Number was expected!\n");
      status = COMPILER_ERROR;
    }
  } else if (sym & (JMP|JGTZ|JZ)) {
    if (sym == LABELTEXT)
      get();
    else {
      printf("ERROR: Label text was expected!\n");
      status = COMPILER_ERROR;
    }
  } else {
    printf("ERROR: Unknown instruction!\n");
    status = COMPILER_ERROR;
  }
}

// public function
int Parse(char *filename) {
  if ((fin = fopen(filename,"rt")) == NULL) {
    printf("ERROR: Input file '%s' cannot be opened!\n", filename);
    return;
  }
  status = COMPILER_OK;
  get();
  
  return status;
}
