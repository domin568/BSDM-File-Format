#ifndef BSDMHEADER_H
#define BSDMHEADER_H
#include "Files.h"
#include <fstream>
#include <iostream>
#pragma pack(1)
struct BSDMHEADER
{
	uint32_t magic;
	uint32_t width;
	int32_t height;
	uint8_t bitsPerPixel;
	uint8_t mode;            // color, grayscale       
	uint8_t isCustomPalette; // true,false
	uint32_t sizeOfHeader;   // 0x14
	uint8_t LZWCodeLength;
};

bool readBSDMHeader(FILE* bsdmFile, BSDMHEADER& header);

#endif // !BSDMHEADER_H