#include <iostream>
#include <fstream>
#include <math.h>
#include <map>
#include <vector>
#include <inttypes.h>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <cstring>
#include <algorithm>
#include "BMP.h"
#include "BSDM.h"
#include "Convert.h"
#include "Files.h"
#pragma pack(1)

int main(int argc, const char* argv[])
{
	bool customPalette = false;
	bool dithering = false;
	bool grayscale = false;


	if (argc < 3)
	{
		std::cout << "[!] Usage: ./bsdm  <in> <out> [--custom-palette] [--dithering] [--grayscale]" << std::endl;
	}

	for (int i = 3; i < argc; i++)
	{
		if (!strcmp(argv[i], "--custom-palette"))
		{
			customPalette = true;
		}
		if (!strcmp(argv[i], "--dithering"))
		{
			dithering = true;
		}

		if (!strcmp(argv[i], "--grayscale"))
		{
			grayscale = true;
		}

	}

	FILE* in = fopen(argv[1], "rb");
	FILE* out = fopen(argv[2], "wb");

	if (in == NULL)
	{
		std::cout << "[!] Cannot open input file !" << std::endl;
		return -2;
	}
	if (out == NULL)
	{
		std::cout << "[!] Cannot open output file !" << std::endl;
		return -3;
	}

	uint8_t* rawBSDMBitmapData;

	char magic[4];

	if (fread(magic, 1, 4, in) != 4)
	{
		return -1;
	}
	fseek(in, 0, 0); // set to begining 

	if (!strncmp(magic, "BM", 2)) // BMP to BSDM
	{
		BITMAPFILEHEADER BMPfileHeaderIN;
		BITMAPINFOHEADER BMPinfoHeaderIN;

		if (readBMPHeaders(in, BMPfileHeaderIN, BMPinfoHeaderIN))
		{
			return 1;
		}
		uint8_t* rawBMPBitmapData = readRawBMPData(in, BMPfileHeaderIN, BMPinfoHeaderIN);
		convertSaveBSDM(out, BMPfileHeaderIN, BMPinfoHeaderIN, rawBMPBitmapData, grayscale, customPalette, dithering);
	}
	else if (!strncmp(magic, "BSDM", 4)) // BSDM to BMP, 24 bit RGB Windows bitmap
	{
		BSDMHEADER BSDMheaderIN;

		if (readBSDMHeader(in, BSDMheaderIN))
		{
			return 1;
		}

		if (BSDMheaderIN.isCustomPalette)
		{
			BSDM_PALETTE palette;
			readBSDMPalette(in, palette);
			convertSaveBMPPalette(in, out, palette, BSDMheaderIN);
		}
		else
		{
			BSDM_PALETTE null;
			std::list<int> compressed = ReadCompressedData(in, BSDMheaderIN, null);
			std::string decompressed = lzw_decompress(compressed);
			uint8_t* rawBSDMBitmapData = new uint8_t[BSDMheaderIN.width * BSDMheaderIN.height];

			rawBSDMBitmapData = ConvertStringtoBDSMrawData(decompressed, BSDMheaderIN.width * BSDMheaderIN.height);
			convertSaveBMP(out, rawBSDMBitmapData, BSDMheaderIN, dithering);
		}
	}
	else
	{
		std::cout << "[!] Unknown file format !" << std::endl;
	}

	fclose(in);
	fclose(out);

	return 0;
}