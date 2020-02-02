#include "LZW.h"
#include <map>
#include <string>

std::list<int> lzw_compression(uint8_t* rawBSDMData, int BSDMDataSize)
{

	std::map <std::string, int> lzwDic;
	int dicSize = 32;  // For 5 bits

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
	std::list<int> encodedVal;

	for (auto it = toCompress.begin(); it != toCompress.end(); ++it)
	{
		curr = *it;
		pc = prev + curr;
		if (lzwDic.count(pc))
			prev = pc;
		else
		{
			encodedVal.push_back(lzwDic[prev]);
			lzwDic[pc] = dicSize++;
			prev = std::string(1, curr);
		}
	}

	if (!prev.empty())
	{
		encodedVal.push_back(lzwDic[prev]);
	}

	return encodedVal;
}

std::string lzw_decompress(std::list<int> compressed)
{
	std::map<int, std::string> lzwDic;
	int dicSize = 32;

	for (int i = 0; i < 32; i++)
	{
		lzwDic[i] = std::string(1, i);
	}

	std::string prev(1, *compressed.begin());
	std::string decompressed = prev;
	std::string in;
	int curr;

	for (auto it = ++(compressed.begin()); it != compressed.end(); ++it)
	{
		curr = *it;
		if (lzwDic.count(curr))
		{
			in = lzwDic[curr];
		}
		else if (curr == dicSize)
		{
			in = prev + prev[0];
		}
		else
		{
			throw "Bad compresed";
		}

		decompressed += in;
		lzwDic[dicSize++] = prev + in[0];
		prev = in;
	}
	return decompressed;
}

uint8_t MinNumBits(std::list<int> rsrc)
{
	uint8_t minBits = 0;
	int currentBits = 0;

	for (auto it = rsrc.begin(); it != rsrc.end(); ++it)
	{
		*it >>= 8;
		while (*it > 0)
		{
			*it >>= 1;
			++currentBits;
		}
		if (currentBits > minBits)
		{
			minBits = currentBits;
		}
		currentBits = 0;
	}
	return minBits + 8;
}

uint8_t* ConvertStringtoBDSMrawData(std::string decompressed, int BSDMDataSize)
{
	uint8_t* rawBSDMData = new uint8_t[BSDMDataSize];
	for (int i = 0; i < BSDMDataSize; i++)
	{
		rawBSDMData[i] = (uint8_t)decompressed[i];
	}

	return rawBSDMData;
}

std::list<int> ReadCompressedData(FILE* bsdmFile, const BSDMHEADER& header)
{
	fseek(bsdmFile, 0, SEEK_END);
	int dataSize = (int)ftell(bsdmFile) - 20;
	fseek(bsdmFile, header.sizeOfHeader, 0);

	if (header.isCustomPalette)
	{
		uint8_t nColors;
		fread(&nColors, 1, 1, bsdmFile);

		uint32_t size = sizeof(RGB_color) * nColors;

		dataSize -= size;
		fseek(bsdmFile, header.sizeOfHeader + size + 1, 0);
	}

	float LZW = header.LZWCodeLength;
	int bytesNeeded = ceil(LZW / 8);

	std::list<int> compressed(dataSize / bytesNeeded, 0);

	std::list<int>::iterator it = compressed.begin();

	for (it; it != compressed.end(); it++)
	{
		fread((char*) & *it, bytesNeeded, 1, bsdmFile);
	}

	return compressed;
}