#include "FileStructs.h"
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
bool readBSDMHeader(FILE* bsdmFile, BSDMHEADER& header)
{
	if (!(readHelper(bsdmFile, &header.magic) &&
		readHelper(bsdmFile, &header.width) &&
		readHelper(bsdmFile, &header.height) &&
		readHelper(bsdmFile, &header.bitsPerPixel) &&
		readHelper(bsdmFile, &header.mode) &&
		readHelper(bsdmFile, &header.isCustomPalette) &&
		readHelper(bsdmFile, &header.sizeOfHeader) &&
		readHelper(bsdmFile, &header.LZWCodeLength)
		))
	{
		std::cout << "[!] Failed to parse fileHeader" << std::endl;
		return 1;
	}
	if (header.magic != 0x4D445342)
	{
		return 1;
	}
	return 0;
}