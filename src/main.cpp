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
#include "headers/png.h"
#pragma pack(1)

#define ERROR 1
#define SUCCESS 0

bool customPalette = false;
bool dithering = false;
bool grayscale = false;
bool toBMP = true; // default conversion from BSDM to BMP
bool toPNG = false;

void parseFlags (int argc, const char ** argv)
{
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
		if (!strcmp(argv[i], "--to-bmp"))
		{
			toBMP = true;
		}
		if (!strcmp(argv[i], "--to-png"))
		{
			toPNG = true;
		}
	}	
}
bool readMagic (FILE * in, const char ** argv, char * fileMagic)
{
	if (fread(fileMagic, 1, 4, in) != 4)
	{
		return ERROR;
	}
	fseek(in, 0, 0); // set to begining 
	return SUCCESS;
}
bool BMPtoBSDM (FILE * in, FILE * out)
{
	BITMAPFILEHEADER BMPfileHeaderIN;
	BITMAPINFOHEADER BMPinfoHeaderIN;

	if (readBMPHeaders(in, BMPfileHeaderIN, BMPinfoHeaderIN))
	{
		std::cout << "[!] Could not read BMP headers !" << std::endl;
		return ERROR;
	}
	uint8_t* rawBMPBitmapData = readRawBMPData(in, BMPfileHeaderIN, BMPinfoHeaderIN);

	inputFormatDataStruct bitmapINFO;
	bitmapINFO.width = BMPinfoHeaderIN.biWidth;
	bitmapINFO.height = BMPinfoHeaderIN.biHeight;
	bitmapINFO.dataSize = BMPfileHeaderIN.bfSize;
	convertSaveToBSDM(out, bitmapINFO ,rawBMPBitmapData, grayscale, customPalette, dithering);
	return SUCCESS;
}
bool PNGtoBSDM (FILE * in, FILE * out)
{
	png_structp png_ptr;
	png_infop info_ptr;
	fseek(in, 0L, SEEK_END);
	uint32_t size = ftell(in);
	fseek(in, 0L, 0);
	uint8_t * rawBitmapData = readPngFile (in,png_ptr,info_ptr);

  	png_byte color_type = png_get_color_type(png_ptr, info_ptr);

  	if (color_type != PNG_COLOR_TYPE_RGB)
   	{
   		std::cout << "[!] RGB only supported in PNG, that's probably RGBA or grayscale" << std::endl;
   		return ERROR;
   	}
  	
	inputFormatDataStruct bitmapINFO;
	bitmapINFO.width = png_get_image_width(png_ptr, info_ptr);
	bitmapINFO.height = png_get_image_height(png_ptr, info_ptr);
	bitmapINFO.dataSize = size;

	convertSaveToBSDM(out, bitmapINFO, rawBitmapData, grayscale, customPalette, dithering);
	return SUCCESS;
}
uint8_t * decompressLZWData (FILE * in, BSDMHEADER & bsdmHeader)
{
	std::list<int> compressed = ReadCompressedData(in, bsdmHeader);
	std::string decompressed = lzw_decompress(compressed);
	uint8_t* rawBSDMBitmapData = new uint8_t[bsdmHeader.width * bsdmHeader.height];
	rawBSDMBitmapData = ConvertStringtoBDSMrawData(decompressed, bsdmHeader.width * bsdmHeader.height);
	return rawBSDMBitmapData;
}
bool BSDMtoOtherFormat (FILE * in, FILE * out, FILE_TYPE format)
{
	BSDMHEADER BSDMheaderIN;

	if (readBSDMHeader(in, BSDMheaderIN))
	{
		std::cout << "[!] Could not read BSDM header !" << std::endl;
		return ERROR;
	}
	if (BSDMheaderIN.isCustomPalette)
	{
		BSDM_PALETTE palette;
		readBSDMPalette(in, palette);
		convertSaveFromBSDMPalette(in, out, palette, BSDMheaderIN,format);
	}
	else
	{
		uint8_t * rawBSDMBitmapData = decompressLZWData(in,BSDMheaderIN);
		convertSaveFromBSDM(out, rawBSDMBitmapData, BSDMheaderIN,format);
	}
	return SUCCESS;
}

bool performConversions (FILE * in, FILE * out, const char * fileMagic)
{
	if (!strncmp(fileMagic, "BM", 2)) // BMP to BSDM
	{
		uint32_t status = BMPtoBSDM(in,out);
		return status;
	}
	else if (!strncmp(fileMagic,"\x89\x50\x4e\x47",4)) // PNG file to BSDM
	{
		uint32_t status = PNGtoBSDM (in,out);
		return status;
	}
	else if (!strncmp(fileMagic, "BSDM", 4)) // BSDM to BMP or PNG, 24 bit
	{
		uint32_t status = SUCCESS; 
		if (toBMP)
		{
			status = BSDMtoOtherFormat(in,out,FORMAT_BMP);  // to BMP
		}
		else if (toPNG)
		{
			status = BSDMtoOtherFormat(in,out,FORMAT_PNG);  // to PNG
		}
		return status;
	}
	else
	{
		std::cout << "[!] Unknown file format !" << std::endl;
		return ERROR;
	}
}

int main(int argc, const char* argv[])
{
	char fileMagic[4];
	FILE * in = fopen(argv[1], "rb");
	FILE * out = fopen(argv[2], "wb");

	if (argc < 3)
	{
		std::cout << "[!] Usage: ./bsdmConvert <in> <out> <--to-bmp>/<--to-png> [--custom-palette] [--dithering] [--grayscale]" << std::endl;
		return 1;
	}

	parseFlags (argc, argv);

	if (in == NULL)
	{
		std::cout << "[!] Cannot open input file !" << std::endl;
		return ERROR;
	}
	if (out == NULL)
	{
		std::cout << "[!] Cannot open output file !" << std::endl;
		return ERROR;
	}

	uint32_t status = readMagic(in,argv,fileMagic);

	if (status == ERROR)
	{
		return 2;
	}

	status = performConversions (in,out,fileMagic);
	if (status == ERROR)
	{
		return 3;
	}

	fclose(in);
	fclose(out);

	return 0;
}