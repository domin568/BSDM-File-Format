#ifndef FILE_STRUCTS_H
#define FILE_STRUCTS_H
#include "Files.h"
#pragma pack(1)

struct BITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned int bfSize;
	unsigned int reserved;
	unsigned int bfOffBits;

};

struct BITMAPINFOHEADER
{
	unsigned int biSize;
	unsigned int biWidth;
	int biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	unsigned int biXPelsPerMeter;
	unsigned int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;

};

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

enum FILE_TYPE 
{
	FORMAT_BMP = 0,
	FORMAT_PNG = 1,
	FORMAT_JPG = 2
};
struct inputFormatDataStruct
{
	uint32_t width;
	uint32_t height;
	uint32_t dataSize;
};

int readBMPHeaders(FILE* bmpFile, BITMAPFILEHEADER& fileHeader, BITMAPINFOHEADER& infoHeader);
bool readBSDMHeader(FILE* bsdmFile, BSDMHEADER& header);
#endif // !BBMPHEADER_H
#pragma once
