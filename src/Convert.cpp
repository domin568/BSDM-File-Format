#include "Convert.h"
#include "DefaultPalettes.h"

void writeBMPHeader (FILE * fBMP, const BSDMHEADER& header)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	memset(&fileHeader, 0, sizeof(fileHeader));
	memset(&infoHeader, 0, sizeof(infoHeader));

	fileHeader.bfType = 0x4d42;
	fileHeader.bfSize = 0; // not important
	fileHeader.bfOffBits = 0x36;

	fwrite(&fileHeader, sizeof(fileHeader), 1, fBMP);

	infoHeader.biSize = 0x28;
	infoHeader.biWidth = header.width;
	infoHeader.biHeight = -header.height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 0x18;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = header.height * ((header.width + 3) & ~3U) * (infoHeader.biBitCount / 8);
	infoHeader.biXPelsPerMeter = 1;
	infoHeader.biYPelsPerMeter = 1;

	fwrite(&infoHeader, sizeof(infoHeader), 1, fBMP);
}
void writeBSDMHeader (FILE * fBSDM, inputFormatDataStruct & info, BSDMHEADER & bsdmHeader, bool isDedicatedPaletteUsed, bool mode)
{
	memcpy(&bsdmHeader, "BSDM", 4);
	bsdmHeader.width = info.width;
	bsdmHeader.height = info.height;
	if (isDedicatedPaletteUsed)
	{
		bsdmHeader.bitsPerPixel = 24;
	}
	else
	{
		bsdmHeader.bitsPerPixel = 5;
	}
	bsdmHeader.mode = mode;
	bsdmHeader.isCustomPalette = isDedicatedPaletteUsed;
	bsdmHeader.sizeOfHeader = sizeof(BSDMHEADER);

	fwrite(&bsdmHeader, 1, sizeof(BSDMHEADER), fBSDM);
}
uint8_t * decompressLZW (FILE * fIN, const BSDMHEADER& header)
{
	std::list<int> compressed = ReadCompressedData(fIN, header);
	std::string decompressed = lzw_decompress(compressed);
	return ConvertStringtoBDSMrawData(decompressed, header.width * header.height);
}
void writeBMPDataCustomPalette (FILE * fIN, FILE* fOUT, const BSDMHEADER& header, const BSDM_PALETTE& palette)
{
	uint32_t pitch = (header.width * 3 + 3) & ~3U;

	uint32_t size = header.width * header.height;
	uint8_t* colorsData = new uint8_t[size];

	colorsData = decompressLZW(fIN, header);

	uint32_t pitchDiff = pitch - header.width * 3;

	for (int i = 0; i < size; i++)
	{
		uint8_t nColor = colorsData[i];
		fwrite(&palette.colors[nColor].b, 1, 1, fOUT);
		fwrite(&palette.colors[nColor].g, 1, 1, fOUT);
		fwrite(&palette.colors[nColor].r, 1, 1, fOUT);
		if ((i % header.width) + 1 == header.width && pitchDiff)
		{
			uint8_t c = 0xCD; // padding
			fwrite(&c, pitchDiff, 1, fOUT);
		}
	}
}
void writePNGCustomPalette (FILE * fIN, FILE * fOUT, const BSDMHEADER & bsdmHeader, const BSDM_PALETTE & palette)
{
	uint32_t size = bsdmHeader.width * bsdmHeader.height;

	uint8_t * colorsData = new uint8_t[size];
	uint8_t * rawBitmapData = new uint8_t [size * 3];

	colorsData = decompressLZW(fIN, bsdmHeader);

	for (int i = 0; i < size; i++)
	{
		uint8_t nColor = colorsData[i];
		rawBitmapData[i * 3] = palette.colors[nColor].r;
		rawBitmapData[i * 3 + 1] = palette.colors[nColor].b;
		rawBitmapData[i * 3 + 2] = palette.colors[nColor].g;
	}

	writePngFile(fOUT,rawBitmapData,bsdmHeader);
}
void writeBMPDataDefaultPalette (FILE* fOUT, const BSDMHEADER& bsdmHeader, uint8_t * rawBSDMBitmapData)
{
	uint32_t pitch = (bsdmHeader.width * 3 + 3) & ~3U;
	uint8_t* rawBitmapData = new uint8_t[pitch * bsdmHeader.height];

	uint32_t widthPitchDiff = (pitch - bsdmHeader.width) * 3;

	if (bsdmHeader.mode) // grayscale
	{
		for (int i = 0; i < std::abs(bsdmHeader.height); i++) // negative height
		{
			for (int j = 0; j < pitch; j += 3)
			{
				rawBitmapData[i * pitch + j] = j < bsdmHeader.width * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD;  // BW

				rawBitmapData[i * pitch + j + 1] = j < bsdmHeader.width * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD;  // BW

				rawBitmapData[i * pitch + j + 2] = j < bsdmHeader.width * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD; //  BW
			}
		}
	}
	else
	{
		for (int i = 0; i < std::abs(bsdmHeader.height); i++) // negative height
		{
			for (int j = 0; j < pitch; j += 3)
			{
				uint8_t b = j < bsdmHeader.width * 3 ? (rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x1) * 0xff : 0xCD;  // B
				rawBitmapData[i * pitch + j] = b;

				uint8_t g = j < bsdmHeader.width * 3 ? ((rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x6) >> 1) * 0x55 : 0xCD;  // G
				rawBitmapData[i * pitch + j + 1] = g;

				uint8_t r = j < bsdmHeader.width * 3 ? ((rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x18) >> 3) * 0x55 : 0xCD; // R
				rawBitmapData[i * pitch + j + 2] = r;
			}
		}
	}
	fwrite(rawBitmapData, pitch * bsdmHeader.height, 1, fOUT);
}
void saveColorNumbers(FILE* fBSDM, BSDM_PALETTE* palette, RGB_color* rawBMPBitmapData, const inputFormatDataStruct & info, BSDMHEADER& bsdmHeader)
{
	RGB_color* pixel;
	uint8_t* toCompress = new uint8_t[info.height * info.width];

	for (int i = 0; i < info.height; i++)
	{
		for (int j = 0; j < info.width; j++)
		{
			pixel = &rawBMPBitmapData[i * info.width + j];
			uint8_t idx = findClosestColorIndexFromPalette(*pixel, palette);
			toCompress[(i * info.width) + j] = idx;
		}
	}
	std::list<int> compressed = lzw_compression(toCompress, info.height * info.width);

	bsdmHeader.LZWCodeLength = MinNumBits(compressed);
	fseek(fBSDM, sizeof(BSDMHEADER) - 1, 0);
	fwrite(&bsdmHeader.LZWCodeLength, 1, 1, fBSDM);
	fseek(fBSDM, 0, SEEK_END);
	float LZW = (float)bsdmHeader.LZWCodeLength;
	int bytesNeeded = ceil(LZW / 8);

	for (auto it = compressed.begin(); it != compressed.end(); it++)
	{
		fwrite((char*) & *it, bytesNeeded, 1, fBSDM);
	}
}

void convertSaveToBSDM(FILE* fBSDM, inputFormatDataStruct & info, uint8_t* rawBMPBitmapData, bool mode, bool isDedicatedPaletteUsed, bool dithering)
{
	BSDMHEADER bsdmHeader;
	writeBSDMHeader(fBSDM,info,bsdmHeader,isDedicatedPaletteUsed,mode);

	uint8_t* rawBSDMBitmapData = new uint8_t[bsdmHeader.width * bsdmHeader.height];
	uint8_t* rawDitheringData = new uint8_t[bsdmHeader.width * bsdmHeader.height * 3];

	if (bsdmHeader.isCustomPalette)
	{
		uint32_t s = bsdmHeader.width * bsdmHeader.height * 3;
		uint8_t* rawBMPBitmapDataQuantinize = (uint8_t*)malloc(s);
		memcpy(rawBMPBitmapDataQuantinize, rawBMPBitmapData, s);

		BSDM_PALETTE* palette = medianCut((RGBBytes*)rawBMPBitmapDataQuantinize, bsdmHeader.width * bsdmHeader.height);
		if (dithering)
		{
			std::cout << "[*] Custom palette with dithering" << std::endl;
			ditheringColor(rawBMPBitmapData, rawDitheringData, palette,info);
			transcodePixels5bits(rawDitheringData, rawBSDMBitmapData, info);
			saveBSDMPalette(fBSDM, palette);
			saveColorNumbers(fBSDM, palette, (RGB_color*)rawDitheringData, info, bsdmHeader);
		}
		else
		{
			std::cout << "[*] Custom palette only\n";
			saveBSDMPalette(fBSDM, palette);
			saveColorNumbers(fBSDM, palette, (RGB_color*)rawBMPBitmapData, info, bsdmHeader);
		}
		free(rawBMPBitmapDataQuantinize);
	}
	else
	{
		if (dithering)
		{
			if (mode)
			{
				std::cout << "[*] Dithering with grayscale on default palette" << std::endl;
				grayscaleIn24bits(rawBMPBitmapData, rawDitheringData, info);
				ditheringColor(rawDitheringData, rawDitheringData,&grayPalette, info);
				transcodePixels5bitsGrayscale(rawDitheringData, rawBSDMBitmapData, info);
			}
			else
			{
				std::cout << "[*] Dithering only on default palette" << std::endl;
				ditheringColor(rawBMPBitmapData, rawDitheringData, &defaultPalette, info);
				transcodePixels5bits(rawDitheringData, rawBSDMBitmapData, info);
			}
		}
		else
		{
			if (mode)
			{
				std::cout << "[*] Grayscale only on default palette" << std::endl;
				grayscaleIn5bits(rawBMPBitmapData, rawBSDMBitmapData, info);
			}
			else
			{
				std::cout << "[*] Defaut palette only" << std::endl;
				transcodePixels5bits(rawBMPBitmapData, rawBSDMBitmapData, info);
			}

		}

		std::list<int> compressed = lzw_compression(rawBSDMBitmapData, bsdmHeader.width * bsdmHeader.height);
		bsdmHeader.LZWCodeLength = MinNumBits(compressed);
		fseek(fBSDM, sizeof(BSDMHEADER) - 1, 0);
		fwrite(&bsdmHeader.LZWCodeLength, 1, 1, fBSDM);
		fseek(fBSDM, 0, SEEK_END);
		float LZW = (float)bsdmHeader.LZWCodeLength;
		int bytesNeeded = ceil(LZW / 8);

		for (auto it = compressed.begin(); it != compressed.end(); it++)
		{
			fwrite((char*) & *it, bytesNeeded, 1, fBSDM);
		}
	}
	unsigned long len = (unsigned long)ftell(fBSDM);
	std::cout << info.dataSize << " bytes -> ";
	std::cout << len << " bytes \n";
	std::cout << bsdmHeader.width << "x" << bsdmHeader.height << std::endl;
	fclose(fBSDM);
}
void convertSaveFromBSDM(FILE* fOUT, uint8_t* rawBSDMBitmapData, const BSDMHEADER& bsdmHeader, FILE_TYPE type)
{
	if (type == FORMAT_BMP)
	{
		writeBMPHeader (fOUT,bsdmHeader);
		writeBMPDataDefaultPalette (fOUT,bsdmHeader,rawBSDMBitmapData);
	}
	else if (type == FORMAT_PNG)
	{
		inputFormatDataStruct info;
		info.width = bsdmHeader.width;
		info.height = bsdmHeader.height;

		uint8_t * data = transcodePixels5bitsTo24 (rawBSDMBitmapData, info);
		writePngFile (fOUT,data,bsdmHeader);
	}

	std::cout << "[*] Size: " << bsdmHeader.width << "x" << bsdmHeader.height << std::endl;
	std::cout << "[*] Custom palette: " << 0 << std::endl;
	std::cout << "[*] Grayscale: " << 0 << std::endl;
}
void convertSaveFromBSDMPalette(FILE* fIN, FILE* fOUT, BSDM_PALETTE& palette, const BSDMHEADER& bsdmHeader, FILE_TYPE type)
{
	if (type == FORMAT_BMP)
	{
		writeBMPHeader (fOUT,bsdmHeader);
		writeBMPDataCustomPalette(fIN, fOUT,bsdmHeader,palette);
	}
	else if (type == FORMAT_PNG)
	{
		writePNGCustomPalette(fIN,fOUT,bsdmHeader,palette);
	}
	
	std::cout << "[*] Size: " << bsdmHeader.width << "x" << bsdmHeader.height << std::endl;
	std::cout << "[*] Custom palette: " << 1 << std::endl;
	std::cout << "[*] Grayscale: " << 0 << std::endl;
}