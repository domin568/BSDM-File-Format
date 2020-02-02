#ifndef CONVERT_H
#define CONVERT_H
#include <fstream>
#include "Color.h"
#include "BSDMHeader.h"
#include "BMPHeader.h"
#include "LZW.h"
#include "Algorithms.h"
#pragma pack(1)

void convertSaveBMPPalette(FILE* fBSDM, FILE* fBMP, BSDM_PALETTE& palette, const BSDMHEADER& header);
void saveColorNumbers(FILE* fBSDM, BSDM_PALETTE* palette, RGB_color* rawBMPBitmapData, const BITMAPINFOHEADER& infoHeader, BSDMHEADER& bsdm_header);
void convertSaveBSDM(FILE* fBSDM, const BITMAPFILEHEADER& fileHeader, const BITMAPINFOHEADER& infoHeader, uint8_t* rawBMPBitmapData, bool mode, bool isDedicatedPaletteUsed, bool dithering);
void convertSaveBMP(FILE* fBMP, uint8_t* rawBSDMBitmapData, const BSDMHEADER& bsdmHeader, bool dithering);

#endif // !CONVERT_H
