//  Created by Dominik Tamiołło on 19.03.2016.

#include <iostream>
#include <fstream>
#include <math.h>
#include <map>
#include <vector>
#include <inttypes.h>
#include <string>
#include <vector>
#include <list>

struct BITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned int bfSize;
	unsigned int reserved;
	unsigned int bfOffBits;

}__attribute__((packed));

struct BITMAPINFOHEADER
{
	unsigned int biSize;
	unsigned int biWidth;
	int biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	unsigned int biXPelsPerMeter;
	unsigned int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;

}__attribute__((packed));

template <int size>
struct COLORPALETTE
{
	int colors[size];

}__attribute__((packed));

struct RGB_color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	bool operator == (const RGB_color& checked)
	{
		if (checked.r == this->r && checked.g == this->g && checked.b == this->b)
		{
			return true;
		}
		return false;
	}
};

struct BSDMHEADER
{
	uint32_t magic;
	uint32_t width;
	uint32_t height;
	uint8_t bitsPerPixel;
	uint8_t mode;            // color, grayscale
	uint8_t isCustomPalette; // true,false
	uint32_t sizeOfHeader;   // 0x13
}__attribute__((packed));

struct BSDM_PALETTE_ENTRY
{
	uint8_t nrColor;
	RGB_color data;
};

struct BSDM_PALETTE
{
	uint8_t numberOfColors;
	std::vector<BSDM_PALETTE_ENTRY> colors;
};

std::list<uint8_t> lzw_compression(uint8_t* rawBSDMData, int BSDMDataSize)
{
    std::map <std::string, int> lzwDic;
    int dicSize = 32;



    for (int i = 0; i < 32; i++)
    {
        lzwDic[std::string(1, i)] = i;
    }

    std::string toCompress = "";
    for (int i = 0; i < BSDMDataSize; i++) {
        toCompress += rawBSDMData[i];
    }

    std::string prev;
    char curr;
    std::string pc;
    std::list<uint8_t> encodedVal;

    for (int i = 0; i < toCompress.length(); i++) {

        curr = toCompress[i];
        pc = prev + curr;
        if (lzwDic.count(pc))
            prev = pc;
        else {

            encodedVal.push_back(lzwDic[prev]);
            lzwDic[pc] = dicSize++;
            prev = std::string(1, curr);

        }
    }

    if (!prev.empty())
        encodedVal.push_back(lzwDic[prev]);

    return encodedVal;
}

std::string lzw_decompress(std::list<uint8_t> encoded) {

    std::map<int, std::string> lzwDic;

    int dicSize = 32;

    for (int i = 0; i < 32; i++) {
        lzwDic[i] = std::string(1, i);
    }

    std::string prev(1, *encoded.begin());
    std::string decompressed = prev;
    std::string in;
    int curr;

    for (auto it = ++(encoded.begin()); it != encoded.end(); ++it) {
        curr = *it;

        if (lzwDic.count(curr))
            in = lzwDic[curr];
        else if (curr == dicSize) {
            in = prev + prev[0];
        }
        else
            throw std::runtime_error("Bad compression of data");


        decompressed += in;
        lzwDic[dicSize++] = prev + in[0];


        prev = in;
    }

    return decompressed;
}

uint8_t *ListToArray(std::list<uint8_t> list){

uint8_t array[list.size()];

std::copy(list.begin(),list.end(),array);

return array;
}


std::list <uint8_t> ArrayToList(uint8_t *array){

std::list<uint8_t> ret(array,array+sizeof(array)/sizeof(uint8_t));

return ret;
}



uint8_t *ConvertStringtoBDSMrawData(std::string decompressed,int BSDMDataSize){

uint8_t *rawBSDMData = new uint8_t[BSDMDataSize];


for(int i=0;i<BSDMDataSize;i++){
   rawBSDMData = (uint8_t*)decompressed[i];
}

return rawBSDMData;
}





void DitheringColor(uint8_t* src, const BITMAPINFOHEADER& infoHeader) {
	uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
	uint32_t width = infoHeader.biWidth;
	RGB_color newColor;
	for (int i = 0; i < dataSizeWithoutPitches-width-3; i += 3){

		uint8_t oldR = src[i];
		uint8_t oldG = src[i + 1];
		uint8_t oldB = src[i + 2];
		RGB_color oldColor = { oldR,oldG,oldB };
		//	newColor = find_closest_palette_color(oldColor);
		uint8_t newR = newColor.r;
		uint8_t newG = newColor.g;
		uint8_t newB = newColor.b;

		src[i] = newR;
		src[i + 1] = newG;
		src[i + 2] = newB;

		int errorR = src[i] - newR;				//not sure, maybe change places with 181-183
		int errorG = src[i + 1] - newG;
		int errorB = src[i + 2] - newB;

		src[i + 3] = src[i + 3] + errorR * (7.0f / 16.0f);
		src[i + 4] = src[i + 4] + errorG * (7.0f / 16.0f);
		src[i + 5] = src[i + 5] + errorB * (7.0f / 16.0f);

		src[i + width - 3] = src[i + width - 3] + errorR * (3.0f / 16.0f);
		src[i + width - 2] = src[i + width - 2] + errorG * (3.0f / 16.0f);
		src[i + width - 1] = src[i + width - 1] + errorB * (3.0f / 16.0f);

		src[i + width] = src[i + width] + errorR * (5.0f / 16.0f);
		src[i + width + 1] = src[i + width + 1] + errorG * (5.0f / 16.0f);
		src[i + width + 2] = src[i + width + 2] + errorB * (5.0f / 16.0f);

		src[i + width + 3] = src[i + width + 3] + errorR * (1.0f / 16.0f);
		src[i + width + 4] = src[i + width + 4] + errorG * (1.0f / 16.0f);
		src[i + width + 5] = src[i + width + 5] + errorB * (1.0f / 16.0f);
	}
}

		/*pixels[y][x + 1].red   = pixels[y][x + 1].red   + ErrorR * (7.0f / 16.0f);
			pixels[y][x + 1].green = pixels[y][x + 1].green + ErrorG * (7.0f / 16.0f);
			pixels[y][x + 1].blue  = pixels[y][x + 1].blue  + ErrorB * (7.0f / 16.0f);

			pixels[y + 1][x - 1].red    = pixels[y + 1][x - 1].red   + ErrorR * (3.0f / 16.0f);
			pixels[y + 1][x - 1].green  = pixels[y + 1][x - 1].green + ErrorG * (3.0f / 16.0f);
			pixels[y + 1][x - 1].blue   = pixels[y + 1][x - 1].blue  + ErrorB * (3.0f / 16.0f);

			pixels[y + 1][x].red    = pixels[y + 1][x].red   + ErrorR * (5.0f / 16.0f);
			pixels[y + 1][x].green  = pixels[y + 1][x].green + ErrorG * (5.0f / 16.0f);
			pixels[y + 1][x].blue   = pixels[y + 1][x].blue  + ErrorB * (5.0f / 16.0f);

			pixels[y + 1][x + 1].red    = pixels[y + 1][x + 1].red   + ErrorR * (1.0f / 16.0f);
			pixels[y + 1][x + 1].green  = pixels[y + 1][x + 1].green + ErrorG * (1.0f / 16.0f);
			pixels[y + 1][x + 1].blue   = pixels[y + 1][x + 1].blue  + ErrorB * (1.0f / 16.0f);*/


	/*

	void FloydSteinbergBW(uint8_t * data, uint32_t width, uint32_t height)
	{
		float bledy [width+2][height+2];
		memset(bledy, 0, sizeof(bledy));

		int BW = 0;
		int przesuniecie = 1; //aby nie wyjść poniżej (-1) tabeli błędów
		int blad = 0;

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				kolor = data[x][y];
				BW = 0.299*kolor.r + 0.587*kolor.g + 0.114*kolor.b;
				BW += bledy[x+przesuniecie][y];
				if (BW > 127)
				{
					setPixel(x+(width/2), y, 255, 255, 255);
					blad = BW - 255;
				}
			else
			{
				setPixel(x+(width/2), y, 0, 0, 0);
				blad = BW;
			}

			bledy[x+1+przesuniecie][y  ] += (blad*7.0/16.0);
			bledy[x+1+przesuniecie][y+1] += (blad*1.0/16.0);
			bledy[x  +przesuniecie][y+1] += (blad*5.0/16.0);
			bledy[x-1+przesuniecie][y+1] += (blad*3.0/16.0);
		}

	}
	*/
	template<typename T>
	bool readHelper(FILE * file, T * data)
	{
		bool isRead = fread(data, sizeof(*data), 1, file);

		if (isRead)
		{
			return true;
		}
		return false;
	}
	int readBMPHeaders(FILE * bmpFile, BITMAPFILEHEADER & fileHeader, BITMAPINFOHEADER & infoHeader)
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
		return 0;
	}
	bool readBSDMHeader(FILE * bsdmFile, BSDMHEADER & header)
	{
		if (!(readHelper(bsdmFile, &header.magic) &&
			readHelper(bsdmFile, &header.width) &&
			readHelper(bsdmFile, &header.height) &&
			readHelper(bsdmFile, &header.bitsPerPixel) &&
			readHelper(bsdmFile, &header.mode) &&
			readHelper(bsdmFile, &header.isCustomPalette) &&
			readHelper(bsdmFile, &header.sizeOfHeader)))
		{
			std::cout << "[!] Failed to parse fileHeader" << std::endl;
			return 1;
		}
		if (header.magic != 0x4D445342)
		{
			return 1;
		}
		if (header.isCustomPalette)
		{

		}
		return 0;
	}
	uint8_t* readRawBMPData(FILE * bmpFile, const BITMAPFILEHEADER & fileHeader, const BITMAPINFOHEADER & infoHeader)
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
	uint8_t* readRawBSDMData(FILE * bsdmFile, const BSDMHEADER & header)
	{
		fseek(bsdmFile, header.sizeOfHeader, 0);
		uint32_t rawDataSize = header.width * header.height;
		uint8_t* data = new uint8_t[rawDataSize];
		fread(data, 1, rawDataSize, bsdmFile);
		return data;
	}
	void transcodePixels5bits(const uint8_t * src, uint8_t * dst, bool direction, const BITMAPINFOHEADER & infoHeader)
	{
		// direction == 0 --> BMP to BSDM
		// direction == 1 --> BSDM to BMP

		if (direction == 0)
		{
			uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
			// assuming BMP is 24 bpp
			// blue is least important color so using one bit less for this color
			for (int i = 0; i < dataSizeWithoutPitches; i += 3)
			{
				uint8_t r = src[i];
				uint8_t g = src[i + 1];
				uint8_t b = src[i + 2];

				// 1
				{
					r = r >> 6;
					g = g >> 6;
					b = b >> 7;
					dst[i / 3] = r << 3;
					dst[i / 3] = dst[i / 3] | (g << 1);
					dst[i / 3] = dst[i / 3] | b;
				}
			}

		}
		else if (direction == 1)
		{

		}
	}
	bool sortbysec(const std::pair<RGB_color, int> & a, const std::pair<RGB_color, int> & b)
	{
		return (a.second > b.second);
	}
	std::vector <BSDM_PALETTE_ENTRY> calculateDedicatedPalette(const uint8_t * rawBMPBitmapData, const BITMAPINFOHEADER & infoHeader)
	{
		uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
		std::vector <std::pair<RGB_color, int>> occurencies;
		std::std::vector <BSDM_PALETTE_ENTRY> colors;

		for (int i = 0; i < dataSizeWithoutPitches; i += 3)
		{
			bool found = false;
			RGB_color actualColor;
			actualColor.r = rawBMPBitmapData[i];
			actualColor.g = rawBMPBitmapData[i + 1];
			actualColor.b = rawBMPBitmapData[i + 2];
			for (auto& it : occurencies)
			{
				if (it.first == actualColor)
				{
					found = true;
					it.second++;
					break;
				}
			}
			if (!found)
			{
				occurencies.push_back(std::make_pair(actualColor, 1));
			}
		}
		std::sort(std::begin(occurencies), std::end(occurencies), sortbysec);
		// get max of 32 first colors
		int colorsUsed = 0;
		for (auto& it : occurencies)
		{
			colors.push_back({ colorsUsed,it.first });
			colorsUsed++;
			if (colorsUsed == 32)
			{
				break;
			}
		}
		return colors;
	}
	void saveBSDMPalette(FILE * fBSDM, std::std::vector<BSDM_PALETTE_ENTRY> & palette)
	{
		uint8_t colorsPaletteNumber = palette.size();
		fwrite(&colorsPaletteNumber, 1, 1, fBSDM); // write number of palette colors
		for (const auto& it : palette)
		{
			fwrite(&it.first, 1, 1, fBSDM);
			fwrite(&it.second, 3, 1, fBSDM);
		}
	}
	void readBSDMPalette(FILE * fBSDM, BSDM_PALETTE & palette)
	{
		uint8_t nColors;
		fread(&nColors, 1, 1, fBSDM);
		palette.numberOfColors = nColors;
		for (int i = 0; i < nColors; i++)
		{
			BSDM_PALETTE_ENTRY entry;
			fread(&entry, 4, 1, fBSDM);
			palette.colors.push_back(entry);
		}
	}
	void convertSaveBSDMPalette(FILE * fBSDM, FILE * fBMP, std::vector<BSDM_PALETTE_ENTRY> & palette, const BSDMHEADER & header)
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
		infoHeader.biSizeImage = bsdmHeader.height * ((bsdmHeader.width + 3) & ~3U) * (infoHeader.biBitCount / 8);
		infoHeader.biXPelsPerMeter = 1;
		infoHeader.biYPelsPerMeter = 1;

		fwrite(&infoHeader, sizeof(infoHeader), 1, fBMP);

		uint32_t pitch = (infoHeader.biWidth * 3 + 3) & ~3U;

		uint32_t size = header.width * header.height;
		uint8_t* colorsData = new uint8_t[header.width * header.height];
		uint8_t* bmpData = new uint8_t[infoHeader.biHeight * pitch];

		fread(colorsData, size, 1, fBSDM);

		for (int i = 0; i < size; i++)
		{
			nColor = colorsData[i];
			bmpData[i * 3] = palette[nColor].data.b;
			bmpData[i * 3 + 1] = palette[nColor].data.g;
			bmpData[i * 3 + 2] = palette[nColor].data.r;
		}

		fwrite(data, size * 3, 1, fBMP);
	}
	void saveColorNumbers(FILE * fBSDM, const std::std::vector<BSDM_PALETTE_ENTRY> & palette, uint8_t * rawBMPBitmapData, const BITMAPINFOHEADER & infoHeader)
	{
		uint32_t sizeOfData = infoHeader.biWidth * infoHeader.biHeight;
		uint32_t dataSizeWithoutPitches = sizeOfData * 3;
		uint8_t* data = new uint8_t[sizeOfData];
		for (int i = 0; i < dataSizeWithoutPitches; i += 3)
		{
			RGB_color actualColor;
			actualColor.r = rawBMPBitmapData[i];
			actualColor.g = rawBMPBitmapData[i + 1];
			actualColor.b = rawBMPBitmapData[i + 2];
			for (const auto& it : palette)
			{
				if (actualColor == it.first)
				{
					data[i / 3] = it.second;
				}
			}
		}
		fwrite(data, sizeOfData, 1, fBSDM);
	}
	void convertSaveBSDM(FILE * fBSDM, const BITMAPFILEHEADER & fileHeader, const BITMAPINFOHEADER & infoHeader, uint8_t * rawBMPBitmapData, bool mode, bool isDedicatedPaletteUsed, bool dithering)
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

		uint8_t* rawBSDMBitmapData = new uint8_t[infoHeader.biSizeImage / 3];

		if (bsdm_header.isCustomPalette)
		{
			std::vector <BSDM_PALETTE_ENTRY> colors_palette = calculateDedicatedPalette(rawBMPBitmapData, infoHeader);
			saveBSDMPalette(fBSDM, colors_palette);
			saveColorNumbers(fBSDM, colors_palette, rawBMPBitmapData, infoHeader);
		}
		else
		{
			transcodePixels5bits(rawBMPBitmapData, rawBSDMBitmapData, 0, infoHeader);
			std::list<uint8_t> rawBSDMBit = lzw_compression(rawBSDMBitmapData,bsdm_header.width*bsdm_header.height);

        	rawBSDMBitmapData = ListToArray(rawBSDMBit);
			fwrite(rawBSDMBitmapData, 1, infoHeader.biSizeImage / 3, fBSDM);
		}

		fclose(fBSDM);

	}
	void convertSaveBMP(FILE * fBMP, uint8_t * rawBSDMBitmapData, const BSDMHEADER & bsdmHeader)
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

		for (int i = 0; i < std::abs(infoHeader.biHeight); i++) // negative height
		{
			for (int j = 0; j < pitch; j += 3)
			{
				rawBitmapData[i * pitch + j] = j < infoHeader.biWidth * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x1) << 7  : 0xCD;  // B

				rawBitmapData[i * pitch + j + 1] = j < infoHeader.biWidth * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x6) << 5  : 0xCD;  // G

				rawBitmapData[i * pitch + j + 2] = j < infoHeader.biWidth * 3 ? \
					(rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x18) << 3  : 0xCD; // R
			}
		}
		fwrite(rawBitmapData, infoHeader.biSizeImage, 1, fBMP);
	}


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
			// TODO PALETTE
			BSDMHEADER BSDMheaderIN;
			if (readBSDMHeader(in, BSDMheaderIN))
			{
				return 1;
			}
			if (BSDMheaderIN.isCustomPalette)
			{
				BSDM_PALETTE palette;
				readBSDMPalette(in, palette);
				convertSaveBMPPalette(out, in, palette, BSDMheaderIN);
			}
			else
			{
				uint8_t* rawBSDMBitmapData = readRawBSDMData(in, BSDMheaderIN);
				std::list<uint8_t> rawBSDMBit = ArrayToList(rawBSDMBitmapData);

        		std::string decompressed = lzw_decompress(rawBSDMBit);

        		rawBSDMBitmapData = ConvertStringtoBDSMrawData(decompressed,BSDMheaderIN.width*BSDMheaderIN.height);
				convertSaveBMP(out, rawBSDMBitmapData, BSDMheaderIN);
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
