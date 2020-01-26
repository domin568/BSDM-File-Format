#include "BMP.h"

uint8_t* readRawBMPData(FILE* bmpFile, const BITMAPFILEHEADER& fileHeader, const BITMAPINFOHEADER& infoHeader)
{
	int pitch = (infoHeader.biWidth * 3 + 3) & ~3U;

	fseek(bmpFile, fileHeader.bfOffBits, 0);
	uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;

	uint8_t* data = new uint8_t[dataSizeWithoutPitches];

	uint8_t* line = new uint8_t[pitch];
	uint8_t tmp;
	int offset = dataSizeWithoutPitches - infoHeader.biWidth * 3;
	for (int i = 0; i < infoHeader.biHeight; i++)
	{
		fread(line, 1, pitch, bmpFile);
		for (int j = 0; j < infoHeader.biWidth * 3; j += 3)
		{
			//BGR to RGB
			tmp = line[j];
			line[j] = line[j + 2];
			line[j + 2] = tmp;
		}

		memcpy(data + offset, line, infoHeader.biWidth * 3);
		offset -= infoHeader.biWidth * 3;
	}
	return data;
}
