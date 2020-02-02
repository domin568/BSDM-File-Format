#ifndef  BMP_H
#define BMP_H
#include <fstream>
#include <FileStructs.h>
#pragma pack(1)

uint8_t* readRawBMPData(FILE* bmpFile, const BITMAPFILEHEADER& fileHeader, const BITMAPINFOHEADER& infoHeader);
#endif // ! BMP_h
