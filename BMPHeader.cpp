#include "BMPHeader.h"
#include <fstream>
#include <iostream>
int readBMPHeaders(FILE* bmpFile, BITMAPFILEHEADER& fileHeader, BITMAPINFOHEADER& infoHeader)
{
	if (!(readHelper(bmpFile, &fileHeader.bfType) &&
		readHelper(bmpFile, &fileHeader.bfSize) &&
		readHelper(bmpFile, &fileHeader.reserved) &&
		readHelper(bmpFile, &fileHeader.bfOffBits)))
	{
		std::cout << "[!] Failed to parse fileHeader" << std::endl;
		return 1;
	}

	if (!(readHelper(bmpFile, &infoHeader.biSize) &&
		readHelper(bmpFile, &infoHeader.biWidth) &&
		readHelper(bmpFile, &infoHeader.biHeight) &&
		readHelper(bmpFile, &infoHeader.biPlanes) &&
		readHelper(bmpFile, &infoHeader.biBitCount) &&
		readHelper(bmpFile, &infoHeader.biCompression) &&
		readHelper(bmpFile, &infoHeader.biSizeImage) &&
		readHelper(bmpFile, &infoHeader.biXPelsPerMeter) &&
		readHelper(bmpFile, &infoHeader.biYPelsPerMeter) &&
		readHelper(bmpFile, &infoHeader.biClrUsed) &&
		readHelper(bmpFile, &infoHeader.biClrImportant)
		))
	{
		std::cout << "[!] Failed to parse infoHeader" << std::endl;
		return 2;
	}
	infoHeader.biHeight = std::abs(infoHeader.biHeight); // protection against negative height 
	return 0;
}