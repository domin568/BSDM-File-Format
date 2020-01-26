#include "Convert.h"
#include "DefaultPalettes.h"
void convertSaveBMPPalette(FILE* fBSDM, FILE* fBMP, BSDM_PALETTE& palette, const BSDMHEADER& header)
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

	uint32_t pitch = (infoHeader.biWidth * 3 + 3) & ~3U;

	uint32_t bmpRawDataSize = pitch * header.height;

	uint32_t size = header.width * header.height;
	uint8_t* colorsData = new uint8_t[size];



	std::list<int> compressed = ReadCompressedData(fBSDM, header, palette);

	std::string decompressed = lzw_decompress(compressed);

	colorsData = ConvertStringtoBDSMrawData(decompressed, header.width * header.height);


	uint32_t pitchDiff = pitch - infoHeader.biWidth * 3;
	uint32_t colorsOfffset = 0;
	std::cout << pitch << std::endl;
	for (int i = 0; i < size; i++)
	{
		uint8_t nColor = colorsData[i];
		fwrite(&palette.colors[nColor].b, 1, 1, fBMP);
		fwrite(&palette.colors[nColor].g, 1, 1, fBMP);
		fwrite(&palette.colors[nColor].r, 1, 1, fBMP);
		if ((i % infoHeader.biWidth) + 1 == infoHeader.biWidth && pitchDiff)
		{
			uint8_t c = 0xCD;
			fwrite(&c, pitchDiff, 1, fBMP);
		}
	}
}


void saveColorNumbers(FILE* fBSDM, BSDM_PALETTE* palette, RGB_color* rawBMPBitmapData, const BITMAPINFOHEADER& infoHeader, BSDMHEADER& bsdm_header)
{
	RGB_color* pixel;

	uint8_t* toCompress = new uint8_t[infoHeader.biHeight * infoHeader.biWidth];

	for (int i = 0; i < infoHeader.biHeight; i++)
	{
		for (int j = 0; j < infoHeader.biWidth; j++)
		{
			pixel = &rawBMPBitmapData[i * infoHeader.biWidth + j];
			uint8_t idx = findClosestColorIndexFromPalette(*pixel, palette);
			toCompress[(i * infoHeader.biWidth) + j] = idx;
		}
	}

	std::list<int> compressed = lzw_compression(toCompress, infoHeader.biHeight * infoHeader.biWidth);



	bsdm_header.LZWCodeLength = MinNumBits(compressed);

	fseek(fBSDM, sizeof(BSDMHEADER) - 1, 0);

	fwrite(&bsdm_header.LZWCodeLength, 1, 1, fBSDM);

	fseek(fBSDM, 0, SEEK_END);




	float LZW = (float)bsdm_header.LZWCodeLength;

	int bytesNeeded = ceil(LZW / 8);


	for (auto it = compressed.begin(); it != compressed.end(); it++) {

		fwrite((char*) & *it, bytesNeeded, 1, fBSDM);


	}
}

void convertSaveBSDM(FILE* fBSDM, const BITMAPFILEHEADER& fileHeader, const BITMAPINFOHEADER& infoHeader, uint8_t* rawBMPBitmapData, bool mode, bool isDedicatedPaletteUsed, bool dithering)
{
	BSDMHEADER bsdm_header;
	memcpy(&bsdm_header, "BSDM", 4);
	bsdm_header.width = infoHeader.biWidth;
	bsdm_header.height = infoHeader.biHeight;
	if (isDedicatedPaletteUsed)
	{
		bsdm_header.bitsPerPixel = 24;
	}
	else
	{
		bsdm_header.bitsPerPixel = 5;
	}
	bsdm_header.mode = mode;
	bsdm_header.isCustomPalette = isDedicatedPaletteUsed;
	bsdm_header.sizeOfHeader = sizeof(BSDMHEADER);

	fwrite(&bsdm_header, 1, sizeof(BSDMHEADER), fBSDM);

	uint8_t* rawBSDMBitmapData = new uint8_t[bsdm_header.width * bsdm_header.height];
	uint8_t* rawDitheringData = new uint8_t[bsdm_header.width * bsdm_header.height * 3];

	if (bsdm_header.isCustomPalette)
	{
		uint32_t s = bsdm_header.width * bsdm_header.height * 3;
		uint8_t* rawBMPBitmapDataQuantinize = (uint8_t*)malloc(s);
		memcpy(rawBMPBitmapDataQuantinize, rawBMPBitmapData, s);

		BSDM_PALETTE* palette = MCQuantizeData((MCTriplet*)rawBMPBitmapDataQuantinize, bsdm_header.width * bsdm_header.height, 5);
		if (dithering)
		{
			//WIN
			std::cout << "Paleta custom with dithering" << std::endl;
			ditheringColor(rawBMPBitmapData, rawDitheringData, infoHeader, palette);
			transcodePixels5bits(rawDitheringData, rawBSDMBitmapData, infoHeader);
			saveBSDMPalette(fBSDM, palette);
			saveColorNumbers(fBSDM, palette, (RGB_color*)rawDitheringData, infoHeader, bsdm_header);
		}
		else
		{
			// wIN
			std::cout << "Paleta custom only\n";
			saveBSDMPalette(fBSDM, palette);
			saveColorNumbers(fBSDM, palette, (RGB_color*)rawBMPBitmapData, infoHeader, bsdm_header);
		}
		free(rawBMPBitmapDataQuantinize);
	}
	else
	{
		if (dithering)
		{
			if (mode)
			{
				std::cout << "Dithering with grayscale on default palette" << std::endl;
				grayscaleIn24bits(rawBMPBitmapData, rawDitheringData, infoHeader);
				ditheringColor(rawDitheringData, rawDitheringData, infoHeader, &grayPalette);
				transcodePixels5bitsGrayscale(rawDitheringData, rawBSDMBitmapData, infoHeader);
			}
			else
			{
				//WIN
				std::cout << "Dithering only on default palette" << std::endl;
				ditheringColor(rawBMPBitmapData, rawDitheringData, infoHeader, &defaultPalette);
				transcodePixels5bits(rawDitheringData, rawBSDMBitmapData, infoHeader);
			}
		}
		else
		{
			if (mode)
			{
				// WIN
				std::cout << "Grayscale only on default palette" << std::endl;
				grayscaleIn5bits(rawBMPBitmapData, rawBSDMBitmapData, infoHeader);
			}
			else
			{
				std::cout << "5 bits color on default palette" << std::endl;
				transcodePixels5bits(rawBMPBitmapData, rawBSDMBitmapData, infoHeader);
			}

		}



		std::list<int> compressed = lzw_compression(rawBSDMBitmapData, bsdm_header.width * bsdm_header.height);



		bsdm_header.LZWCodeLength = MinNumBits(compressed);

		fseek(fBSDM, sizeof(BSDMHEADER) - 1, 0);

		fwrite(&bsdm_header.LZWCodeLength, 1, 1, fBSDM);

		fseek(fBSDM, 0, SEEK_END);

		float LZW = (float)bsdm_header.LZWCodeLength;

		int bytesNeeded = ceil(LZW / 8);


		for (auto it = compressed.begin(); it != compressed.end(); it++) {

			fwrite((char*) & *it, bytesNeeded, 1, fBSDM);


		}
		unsigned long len = (unsigned long)ftell(fBSDM);
		std::cout << fileHeader.bfSize << " bytes -> ";
		std::cout << len << " bytes \n";
		std::cout << bsdm_header.width << "x" << bsdm_header.height;
	}

	fclose(fBSDM);

}

void convertSaveBMP(FILE* fBMP, uint8_t* rawBSDMBitmapData, const BSDMHEADER& bsdmHeader, bool dithering)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	memset(&fileHeader, 0, sizeof(fileHeader));
	memset(&infoHeader, 0, sizeof(infoHeader));

	fileHeader.bfType = 0x4d42;
	fileHeader.bfSize = 0; // not important
	fileHeader.bfOffBits = 0x36;

	fwrite(&fileHeader, sizeof(fileHeader), 1, fBMP);
	bool mode = bsdmHeader.mode;
	infoHeader.biSize = 0x28;
	infoHeader.biWidth = bsdmHeader.width;
	infoHeader.biHeight = -bsdmHeader.height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 0x18;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = bsdmHeader.height * ((bsdmHeader.width + 3) & ~3U) * (infoHeader.biBitCount / 8);
	infoHeader.biXPelsPerMeter = 1;
	infoHeader.biYPelsPerMeter = 1;

	fwrite(&infoHeader, sizeof(infoHeader), 1, fBMP);

	uint32_t rawDataSize = bsdmHeader.width * bsdmHeader.height;
	uint8_t* rawBitmapData = new uint8_t[infoHeader.biSizeImage];

	uint32_t pitch = (infoHeader.biWidth * 3 + 3) & ~3U;
	uint32_t widthPitchDiff = (pitch - bsdmHeader.width) * 3;
	std::cout << "Size: " << infoHeader.biWidth << "x" << bsdmHeader.height << "\n";
	std::cout << "Custom palette: " << (int)bsdmHeader.isCustomPalette << "\n";
	std::cout << "Grayscale: " << (int)bsdmHeader.mode << "\n";


	if (mode)
	{
		for (int i = 0; i < std::abs(infoHeader.biHeight); i++) // negative height
		{
			for (int j = 0; j < pitch; j += 3)
			{
				rawBitmapData[i * pitch + j] = j < infoHeader.biWidth * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD;  // BW

				rawBitmapData[i * pitch + j + 1] = j < infoHeader.biWidth * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD;  // BW

				rawBitmapData[i * pitch + j + 2] = j < infoHeader.biWidth * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD;
			}
		}
	}
	else
	{
		for (int i = 0; i < std::abs(infoHeader.biHeight); i++) // negative height
		{
			for (int j = 0; j < pitch; j += 3)
			{

				uint8_t b = j < infoHeader.biWidth * 3 ? (rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x1) * 0xff : 0xCD;  // B
				rawBitmapData[i * pitch + j] = b;

				uint8_t g = j < infoHeader.biWidth * 3 ? ((rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x6) >> 1) * 0x55 : 0xCD;  // G
				rawBitmapData[i * pitch + j + 1] = g;

				uint8_t r = j < infoHeader.biWidth * 3 ? ((rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x18) >> 3) * 0x55 : 0xCD; // R
				rawBitmapData[i * pitch + j + 2] = r;
			}
		}
	}
	fwrite(rawBitmapData, infoHeader.biSizeImage, 1, fBMP);
}
