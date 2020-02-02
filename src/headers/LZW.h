#ifndef  LZW_H
#define LZW_H

#include <list>
#include <math.h>
#include "BSDMHeader.h"
#include "Color.h"
#pragma pack(1)
std::list<int> lzw_compression(uint8_t* rawBSDMData, int BSDMDataSize);

std::string lzw_decompress(std::list<int> compressed);

uint8_t MinNumBits(std::list<int> rsrc);

uint8_t* ConvertStringtoBDSMrawData(std::string decompressed, int BSDMDataSize);


std::list<int> ReadCompressedData(FILE* bsdmFile, const BSDMHEADER& header, BSDM_PALETTE& palette);
#endif // ! LZW_H
