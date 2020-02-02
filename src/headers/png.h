#include <png.h>
#include <cstdlib>
#include <string.h>
#include <iostream>

uint8_t * readPngFile(FILE * fp, png_structp & png_ptr ,png_infop & info_ptr);
void writePngFile(FILE * fp);
void processFile();

