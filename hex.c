/**
 * hex.c
 *
 * (c) Copyright 2010, P. Jakubèo
 *
 * This file manages the input and output of the HEX file
 *
 * UNTESTED
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Funkcia nacita HEX subor a prevedie ho na binarny tvar
 *
 * @param filename - meno suboru
 * @return adresa s nacitanym suborom
 */
void *hex_load(const char* filename) {
  FILE *fin;
  int size;
  unsigned char *buffer;
  int status;
  
  if ((fin = fopen(filename, "r")) == NULL) {
    printf("Error: can't open hex file name '%s'.\n", filename);
    return NULL;
  }
  
  /* get approximate file size (resulting HEX binary should be less than this) */
  fseek(fin, 0, SEEK_END);
	size = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	
  buffer = (unsigned char *)calloc(1, size);
  if (buffer == NULL) {
    fclose(fin);
    printf("Error: can't allocate memory for the program.\n");
    return NULL;
  }
  
  status = hex_parse(fin, buffer);
  fclose(fin);
  if (status < 0) {
    free(buffer);
    printf("Error: can't parse the hex file.\n");
    return NULL;
  }
  return buffer;
}

/**
 * This function parses the HEX file and convert it to binary file into the
 * buffer.
 *
 * @param fin - opened input file
 * @param buffer - destination buffer where to store the program
 * @return -1 if the parsing failed, 0 if it is OK.
 */
int hex_parse(FILE *fin, unsigned char *buffer) {
  int i =0,j =0,k=0,l=0,y=0;
  char c,d,e,f;
  int byteCount, address, dataType;
  char twoBytes[3];
  int lastStartSet = 0;
  int lastImageStart = 0;
  short int data;

  while(!feof(fin)) {
    // ignore spaces
    while (((i = fgetc(fin)) != EOF) && ((char)i != '\t') && ((char)i != ' '))
      ;
    // ignore comments
    if ((char)i == ';') {
      while(((i = fgetc(fin)) != EOF) && ((char)i != '\n') && ((char)i != '\r'))
        ;
      if (i == EOF)
        return -1;
      continue;
    }
    c = (char)i;
    
    // a line MUST begin with ':'
    if (c != ':')
      return -1;

    // data bytes count
    fscanf(fin, "%2X",&byteCount); //TODO: check...
    if (feof(fin))
      return -1;

    if (byteCount == 0) {
      // ignore line
      while((i = fgetc(fin)) != EOF)
        if (((char)i == '\n') || ((char)i == '\r'))
          break;
      if (i == EOF)
        return -1;
      continue;
    }

    // address - 4 bytes
    fscanf(fin, "%4X", &address); //TODO: check...
    if (feof(fin))
      return -1;
    
    if (!lastStartSet) {
      lastImageStart = address;
      lastStartSet = 1;
    }
                
    // data type
    fscanf(fin, "%2X", &dataType); //TODO: check...
    if (feof(fin))
      return -1;

    if (dataType == 1) { 
      return 0;
    }

    if (dataType != 0) { 
      // doesnt support other data types
      return -1;
    }
                
    // data loading...
    for (y = 0; y < byteCount; y++) {
      fscanf(fin, "%2X", &data); //TODO: check...
      if (feof(fin))
        return -1;

      buffer[address++] = data;
    }

    // checksum - dont care..
    while((i = fgetc(fin)) != EOF) 
      if ((char)i == '\n' || i == 0x0D)
        break;
  }
  return 0;
}
