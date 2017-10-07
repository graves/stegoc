#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include <getopt.h>
#include <sys/time.h>


#include "stegoc.h"


int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers;

int main(int argc, char *argv[])
{
  int eflag = 0;
  int dflag = 0;
  int sflag = 0;
  char *inputFileName = NULL;
  char *fileToEncodeName = NULL;
  char *outputFileName = NULL;
  char *htmlFileName = NULL;
  char *chanarg;
  char *channelName;
  int channel;
  int c;
  FILE* outputFile = NULL;
  FILE* fileToEncode = NULL;

  opterr = 0;

  while ((c = getopt (argc, argv, "e:d:s:o:c:x:h:")) != -1) {
    switch (c)
    {
      case 'e':
        eflag = 1;
        inputFileName = optarg;
        break;
      case 'd':
        dflag = 1;
        inputFileName = optarg;
        break;
      case 's':
        sflag = 1;
        inputFileName = optarg;
        break;
      case 'o':
        outputFileName = optarg;
        break;
      case 'c':
        chanarg = optarg;
        break;
      case 'x':
        fileToEncodeName = optarg;
        break;
      case 'h':
        htmlFileName = optarg;
        break;
      case '?':
        return 1;
      default:
        abort ();
    }
  }

  if (eflag || dflag) {
    if (strcmp(chanarg, "1") == 0) 
    {
      channel = 0;
      channelName = "RED";
    } 
    else if (strcmp(chanarg, "2") == 0)
    {
      channel = 1;
      channelName = "GREEN";
    }
    else if (strcmp(chanarg, "3") == 0)
    {
      channel = 2;
      channelName = "BLUE";
    }
    else if (strcmp(chanarg, "4") == 0)
    {
      channel = 3;
      channelName = "ALPHA";
    }
    else /* default: */
    {
      printf("%s %s\n", "Error:", "Channel must be between 1 and 4");
      printf("%s\n------------\n", "Channel map:");
      printf("  %6s %s\n", "RED:", "1");
      printf("  %6s %s\n", "GREEN:", "2");
      printf("  %6s %s\n", "BLUE:", "3");
      printf("  %6s %s\n", "ALPHA:", "4");
      exit(1);
    }
  }

  if(eflag) {

    if(argc != 9) {
      printf("%s %s %s\n", "Usage:", argv[0],
          "-e <INPUT.PNG> -o <OUTPUT.PNG> -x <FILE> -c <CHANNEL>");
      exit(1);
    }

    printf("Encoding %s into %s on %s channel and outputting to %s\n", 
        fileToEncodeName, inputFileName, channelName, outputFileName);

    fileToEncode = fopen(fileToEncodeName, "rb");
    if( !fileToEncode ) perror(fileToEncodeName),exit(1);

    fseek(fileToEncode, 0L, SEEK_END);
    unsigned long size = ftell(fileToEncode);
    rewind(fileToEncode);

    read_png_file(inputFileName);
    encode_lsb(fileToEncode, size, channel);
    write_png_file(outputFileName);

    fclose(fileToEncode);

    printf("Success!\n");

    return 0;
  }

  if(dflag) {

    if(argc != 7) {
      printf("%s %s %s\n", "Usage:", argv[0],
          "-d <INPUT.PNG> -o <OUTPUT_FILE> -c <CHANNEL>");
      exit(1);
    }

    printf("Decoding %s channel of %s and outputting to %s\n", 
        channelName, inputFileName, outputFileName);

    outputFile = fopen(outputFileName, "ab*");
    if( !outputFile ) perror(outputFileName),exit(1);

    read_png_file(inputFileName);
    decode_lsb(outputFile, channel);

    fclose(outputFile);

    printf("Success!\n");

    return 0;
  }

  if(sflag) {

    if(argc != 7) {
      printf("%s %s %s\n", "Usage:", argv[0],
          "-s <INPUT.PNG> -h <FILE.HTML> -o <OUTPUT_FILE>");
      exit(1);
    }

    printf("Stuffing %s in %s tEXT chunks and outputting in %s\n",
        htmlFileName, inputFileName, outputFileName);

    FILE* htmlFile = fopen ( htmlFileName , "rb" );
    if( !htmlFile ) perror(htmlFileName),exit(1);

    int r = randint();

    char *randchars = randstring(r);
    
    read_png_file(inputFileName);
    stuff_html(outputFileName, htmlFile, randchars);

    fclose(htmlFile);

    printf("Success!\n");

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

void write_png_file(char *filename) 
{
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

void stuff_html(char *outputFileName, FILE* htmlFile, char* randchars) 
{
   
  char *htmlstrbegin = htmlstrbegin = "--> ";

  fseek( htmlFile , 0L , SEEK_END);
  size_t lSize = ftell( htmlFile );
  rewind( htmlFile );

  char* htmlbuf = calloc( 1, lSize+1 );
  if( !htmlbuf ) fclose(htmlFile),fputs("Failed allocating memory for html",
      stderr),exit(1);

  if( 1!=fread( htmlbuf , lSize, 1 , htmlFile) )
    fclose(htmlFile),free(htmlbuf),fputs("Failed reading html file",
        stderr),exit(1);

  char *dest = malloc(strlen(htmlstrbegin) + strlen(htmlbuf));
  strcpy(dest, htmlstrbegin);
  
  dest = strcat(dest, htmlbuf);

  png_text text_ptr[3];

  char *key0 = "Title";
  char *text0 = "<html> <-- ";
  text_ptr[0].key = key0;
  text_ptr[0].text = text0;
  text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
  text_ptr[0].itxt_length = 0;
  text_ptr[0].lang = NULL;
  text_ptr[0].lang_key = NULL;

  char *key1 = "Author";
  char *text1 = randchars;
  text_ptr[1].key = key1;
  text_ptr[1].text = text1;
  text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
  text_ptr[1].itxt_length = 0;
  text_ptr[1].lang = NULL;
  text_ptr[1].lang_key = NULL;

  char *key2 = "Description";
  char *text2 = dest;
  text_ptr[2].key = key2;
  text_ptr[2].text = text2;
  text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
  text_ptr[2].itxt_length = 0;
  text_ptr[2].lang = NULL;
  text_ptr[2].lang_key = NULL;

  FILE *fp = fopen(outputFileName, "wb");
  if(!fp) abort();

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_set_text(png, info, text_ptr, 3);

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

void encode_lsb(FILE* fileToEncode, unsigned long size, int channel)
{
  unsigned char buffer = 0;

  for(int y = 0; y < height; y++) {

    png_bytep row = row_pointers[y];

    for(int x = 0; x < width; x++) {

      png_bytep px = &(row[x * 4]);

      if(y == 0 && x < (sizeof(long) * 8)){
        if((size & ipow(2, x))) {
          px[channel] |= 1;
        }
        else {
          px[channel] &= 0xFE;
        }
      }

      if(y != 0) {
        if(x % 8 == 0) {
          buffer = fgetc(fileToEncode);
        }

        if(!feof(fileToEncode)) {
          if(buffer & ipow(2, x % 8)) {
            px[channel] |= 1;
          }
          else {
            px[channel] &= 0xFE;
          }
        }
      }
    }
  }
}

void decode_lsb(FILE* outputFile, int channel)
{
  unsigned int size = 0;
  int counter = 0;
  unsigned char buffer = 0;

  for(int y = 0; y < height; y++) {

    png_bytep row = row_pointers[y];

    for(int x = 0; x < width; x++) {

      png_bytep px = &(row[x * 4]);

      if(y == 0 && x < (sizeof(long) * 8)){
        size |= ((px[channel] & 1 ) << x);
      }

      if(y > 0) {
        if(counter < size * 8 * 8) {

          buffer |= ((px[channel] & 1) << counter % 8);
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

char *randstring(int size)
{
  const char charset[] = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  char *str;
  str = malloc(sizeof(char) * (size + 1));

  if (size) {
    --size;

    for (int n = 0; n < size; n++) {
      int key = rand() % (int) (sizeof charset - 1);
      str[n] = charset[key];
    }

    str[size] = '\0';
  }

  return str;
}

int randint(void)
{
  struct timeval t1;

  gettimeofday(&t1, NULL);
  srand(t1.tv_usec * t1.tv_sec);

  int a = 1, b = 12;

  int val = a + (b-a) * (double)rand() / (double)RAND_MAX + 0.5;
  int smallval = val % 2048;

  return smallval;
}
