#ifndef STEGOC_H
#define STEGOC_H

#include <stdlib.h>
#include <stdio.h>
#include <png.h>


void read_png_file(char *filename);
void write_png_file(char *filename);
void encode_lsb_red(FILE* fileToEncode, unsigned long size);
void decode_lsb_red(FILE* outputFile);
void tobinstr(int value, char* output);
int ipow(int base, int exp);

#endif
