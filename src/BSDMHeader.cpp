#include "BSDMHeader.h"

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