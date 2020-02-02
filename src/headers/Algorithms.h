#ifndef AlGORITHMS_H
#define ALGORITHMS_H
#include "FileStructs.h"
#include "Color.h"
#include "ColorAlgorithms.h"
#include "Convert.h"
#pragma pack(1)

void ditheringColor(uint8_t* src, uint8_t* dst, BSDM_PALETTE* colorTable, const inputFormatDataStruct & info);
void grayscaleIn5bits(const uint8_t* src, uint8_t* dst, const inputFormatDataStruct & info);
void grayscaleIn24bits(const uint8_t* src, uint8_t* dst, const inputFormatDataStruct & info);
void transcodePixels5bits(const uint8_t* src, uint8_t* dst, const inputFormatDataStruct & info);
void transcodePixels5bitsGrayscale(const uint8_t* src, uint8_t* dst, const inputFormatDataStruct & info);
uint8_t * transcodePixels5bitsTo24 (const uint8_t* src, const inputFormatDataStruct & info);

#endif // ! AlGORITHMS_H
