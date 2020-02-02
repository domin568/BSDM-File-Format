#ifndef BBMPHEADER_H
#define BMPHEADER_H
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

int readBMPHeaders(FILE* bmpFile, BITMAPFILEHEADER& fileHeader, BITMAPINFOHEADER& infoHeader);
#endif // !BBMPHEADER_H
#pragma once
