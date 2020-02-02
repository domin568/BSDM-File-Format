#ifndef CONVERT_H
#define CONVERT_H
#include <fstream>
#include "Color.h"
#include "FileStructs.h"
#include "LZW.h"
#include "Algorithms.h"
#include "png.h"
#pragma pack(1)

void writeBMPHeader (FILE * fBMP, const BSDMHEADER& header);
void writeBSDMHeader (FILE * fBSDM, inputFormatDataStruct & info, BSDMHEADER & bsdmHeader, bool isDedicatedPaletteUsed, bool mode);

uint8_t * decompressLZW (FILE * fIN, const BSDMHEADER& header);

void writePNGCustomPalette (FILE * fIN, FILE * fOUT, const BSDMHEADER & header, const BSDM_PALETTE & palette);
void writeBMPDataCustomPalette (FILE * fIN, FILE* fOUT, const BSDMHEADER& header, const BSDM_PALETTE& palette);
void writeBMPDataDefaultPalette (FILE* fOUT, const BSDMHEADER& header, BSDM_PALETTE& palette);

void convertSaveFromBSDMPalette(FILE* fIN, FILE* fOUT, BSDM_PALETTE& palette, const BSDMHEADER& bsdmHeader, FILE_TYPE type);
void saveColorNumbers(FILE* fBSDM, BSDM_PALETTE* palette, RGB_color* rawBMPBitmapData, const inputFormatDataStruct & info, BSDMHEADER& bsdm_header);

void convertSaveToBSDM(FILE* fBSDM, inputFormatDataStruct & info, uint8_t* rawBMPBitmapData, bool mode, bool isDedicatedPaletteUsed, bool dithering);
void convertSaveFromBSDM(FILE* fOUT, uint8_t* rawBSDMBitmapData, const BSDMHEADER& bsdmHeader, FILE_TYPE type);

#endif // !CONVERT_H
