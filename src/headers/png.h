#include <png.h>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include "FileStructs.h"

uint8_t * readPngFile(FILE * fp, png_structp & png_ptr, png_infop & info_ptr);
void writePngFile(FILE * fOUT, uint8_t * data, const BSDMHEADER & info);
void processFile();

