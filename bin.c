/**
 * bin.c
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 * This module handles the loading of textual binary representation of RAM
 * program.
 *
 */
 
#include <stdio.h>

int ram_size = -1;  /* velkost RAM binarneho programu */


/**
 * Tato funkcia nacita subor s RAM programom do pamate.
 *
 * @param filename - nazov suboru
 * @return adresa nacitaneho bufra
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
      printf("Warning: Ignoring some input (since position %d)\n", p);
      break;
    }
    fscanf(fin, "%u ", &t);
    buffer[p++] = (unsigned char)(t & 0xFF);
  }
  buffer[p] = 0;
  return p;
}

/**
 * Tato funkcia zobrazi binarnu reprezentaciu programu RAM v mnemo-kode.
 * Predpoklada spravnu syntax a samozrejme ze je program nacitany.
 *
 * @param buffer - pamat kde je ulozeny RAM program
 */
void bin_print(unsigned char *buffer) {
  int p = 0;
  int c;
  
  if (ram_size < 0) {
    printf("Error: RAM program was not loaded properly.");
    return;
  }

  do {
    c = buffer[p++];
    switch (c) {
      case 0: printf("\tHALT\n"); break;
      case 1: printf("\tREAD %d\n", (unsigned char)buffer[p++]); break;
      case 2: printf("\tREAD *%d\n", (unsigned char)buffer[p++]); break;
      case 3: printf("\tWRITE =%d\n", (unsigned char)buffer[p++]); break;
      case 4: printf("\tWRITE %d\n", (unsigned char)buffer[p++]); break;
      case 5: printf("\tWRITE *%d\n", (unsigned char)buffer[p++]); break;
      case 6: printf("\tLOAD =%d\n", (unsigned char)buffer[p++]); break;
      case 7: printf("\tLOAD %d\n", (unsigned char)buffer[p++]); break;
      case 8: printf("\tLOAD *%d\n", (unsigned char)buffer[p++]); break;
      case 9: printf("\tSTORE %d\n", (unsigned char)buffer[p++]); break;
      case 10: printf("\tSTORE *%d\n", (unsigned char)buffer[p++]); break;
      case 11: printf("\tADD =%d\n", (unsigned char)buffer[p++]); break;
      case 12: printf("\tADD %d\n", (unsigned char)buffer[p++]); break;
      case 13: printf("\tADD *%d\n", (unsigned char)buffer[p++]); break;
      case 14: printf("\tSUB =%d\n", (unsigned char)buffer[p++]); break;
      case 15: printf("\tSUB %d\n", (unsigned char)buffer[p++]); break;
      case 16: printf("\tSUB *%d\n", (unsigned char)buffer[p++]); break;
      case 17: printf("\tMUL =%d\n", (unsigned char)buffer[p++]); break;
      case 18: printf("\tMUL %d\n", (unsigned char)buffer[p++]); break;
      case 19: printf("\tMUL *%d\n", (unsigned char)buffer[p++]); break;
      case 20: printf("\tDIV =%d\n", (unsigned char)buffer[p++]); break;
      case 21: printf("\tDIV %d\n", (unsigned char)buffer[p++]); break;
      case 22: printf("\tDIV *%d\n", (unsigned char)buffer[p++]); break;
      case 23: printf("\tJMP %d\n", (unsigned char)buffer[p++]); break;
      case 24: printf("\tJGTZ %d\n", (unsigned char)buffer[p++]); break;
      case 25: printf("\tJZ %d\n", (unsigned char)buffer[p++]); break;
      default:
        printf("\t>> UNKNOWN INSTRUCTION <<\n"); break;
    }
  } while (p < ram_size);
}
