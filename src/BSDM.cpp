#include "BSDM.h"

uint8_t* readRawBSDMData(FILE* bsdmFile, const BSDMHEADER& header)
{
	fseek(bsdmFile, header.sizeOfHeader, 0);
	uint32_t rawDataSize = header.width * header.height;
	uint8_t* data = new uint8_t[rawDataSize];
	fread(data, 1, rawDataSize, bsdmFile);
	return data;
}