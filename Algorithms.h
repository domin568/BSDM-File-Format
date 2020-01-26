#ifndef  AlGORITHMS_H
#define ALGORITHMS_H
#include "BMPHeader.h";
#include "Color.h";
#include "ColorAlgorithms.h"
#pragma pack(1)

void ditheringColor(uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader, BSDM_PALETTE* colorTable);
void grayscaleIn5bits(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader);
void grayscaleIn24bits(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader);
void transcodePixels5bits(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader);
void transcodePixels5bitsGrayscale(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader);

#endif // ! AlGORITHMS_H
