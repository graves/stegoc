#ifndef STEGOC_H
#define STEGOC_H

#include <stdlib.h>
#include <stdio.h>
#include <png.h>


void read_png_file(char *filename);
void write_png_file(char *filename);
void encode_lsb(FILE* fileToEncode, unsigned long size, int channel);
void decode_lsb(FILE* outputFile, int channel);
void tobinstr(int value, char* output);
int ipow(int base, int exp);

#endif
