#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>

#include "stegoc.h"


int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;

int main(int argc, char *argv[])
{

  if(strcmp(argv[1], "--encode") == 0) {

    if(argc != 5) {
      printf("%s %s %s %s\n", "Usage:", argv[0], "--encode",
             "<INPUT.PNG> <OUTPUT.PNG> <FILE>");
      exit(1);
    }

    FILE* fileToEncode = NULL;
    fileToEncode = fopen(argv[4],"rb");

    if(!fileToEncode)
      exit(1);

    fseek(fileToEncode, 0L, SEEK_END);
    unsigned long size = ftell(fileToEncode);
    rewind(fileToEncode);

    read_png_file(argv[2]);
    encode_lsb_red(fileToEncode, size);
    write_png_file(argv[3]);

    return 0;
  }

  if(strcmp(argv[1], "--decode") == 0) {

    if(argc != 4) {
      printf("%s %s %s %s\n", "Usage:", argv[0], "--decode",
             "<INPUT.PNG> <OUTPUT_FILE>");
      exit(1);
    }

    FILE* outputFile = NULL;
    outputFile = fopen(argv[3],"ab*");

    if(!outputFile)
      exit(1);

    read_png_file(argv[2]);
    decode_lsb_red(outputFile);

    return 0;
  }
}

void read_png_file(char *filename) {
  FILE *fp = fopen(filename, "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
      NULL, NULL);
  if(!png) abort();

  png_infop info = png_create_info_struct(png);
  if(!info) abort();

  if(setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_read_info(png, info);

  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }

  png_read_image(png, row_pointers);

  fclose(fp);
}

void write_png_file(char *filename) {
  int y;

  FILE *fp = fopen(filename, "wb");
  if(!fp) abort();

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  // Output is 8bit depth, RGBA format.
  png_set_IHDR(
    png,
    info,
    width, height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

  for(int y = 0; y < height; y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);

  fclose(fp);
}

void encode_lsb_red(FILE* fileToEncode, unsigned long size)
{
  unsigned char buffer = 0;

  for(int y = 0; y < height; y++) {

    png_bytep row = row_pointers[y];

    for(int x = 0; x < width; x++) {

      png_bytep px = &(row[x * 4]);

      if(y == 0 && x < (sizeof(long) * 8)){
        if((size & ipow(2, x))) {
          px[0] |= 1;
        }
        else {
          px[0] &= 0xFE;
        }
      }


      if(y != 0) {
        if(x % 8 == 0) {
          buffer = fgetc(fileToEncode);
        }

        if(!feof(fileToEncode)) {
          if(buffer & ipow(2, x % 8)) {
            px[0] |= 1;
          }
          else {
            px[0] &= 0xFE;
          }
        }
      }
    }
  }
}

void decode_lsb_red(FILE* outputFile)
{
  unsigned int size = 0;
  int counter = 0;
  unsigned char buffer = 0;

  for(int y = 0; y < height; y++) {

    png_bytep row = row_pointers[y];

    for(int x = 0; x < width; x++) {

      png_bytep px = &(row[x * 4]);

      if(y == 0 && x < (sizeof(long) * 8)){
        size |= ((px[0] & 1 ) << x);
      }

      if(y > 0) {
        if(counter < size * 8 * 8) {

          buffer |= ((px[0] & 1) << counter % 8);
          counter++;

          if(counter % 8 == 0){
            fwrite(&buffer, 1, 1, outputFile);
            buffer = 0;
          }
        }
      }
    }
  }
}

void tobinstr(int value, char* output)
{
  int i;
  output[8] = '\0';

  for (i = 7; i >= 0; --i, value >>= 1) {
    output[i] = (value & 1) + '0';
  }
}

int ipow(int base, int exp)
{
  int result = 1;

  while(exp) {

    if (exp & 1) {
      result *= base;
    }

    exp >>= 1;
    base *= base;
  }

  return result;
}
