#include "Algorithms.h"

void ditheringColor(uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader, BSDM_PALETTE* colorTable)
{
	RGB_color currentPixel;
	RGB_color actualColor;

	RGB_color* srcColor = (RGB_color*)src;
	RGB_color* dstColor = (RGB_color*)dst;

	float** errR = new float* [infoHeader.biWidth + 2];
	float** errG = new float* [infoHeader.biWidth + 2];
	float** errB = new float* [infoHeader.biWidth + 2];

	// initialize
	for (int i = 0; i < infoHeader.biWidth + 2; ++i)
	{
		errR[i] = new float[infoHeader.biHeight + 2]{ 0 };
		errG[i] = new float[infoHeader.biHeight + 2]{ 0 };
		errB[i] = new float[infoHeader.biHeight + 2]{ 0 };
	}

	int shift = 1; // offset not to get out of range
	float currErrR = 0, currErrG = 0, currErrB = 0;

	for (int y = 0; y < infoHeader.biHeight; y++)
	{
		for (int x = 0; x < infoHeader.biWidth; x++)
		{
			currentPixel = srcColor[y * infoHeader.biWidth + x];

			// error to pixel, shitty overflow checks
			if (currentPixel.r + errR[x + shift][y] > 255)
			{
				currentPixel.r = 255;
			}
			else if (currentPixel.r + errR[x + shift][y] < 0)
			{
				currentPixel.r = 0;
			}
			else
			{
				currentPixel.r += errR[x + shift][y];
			}
			if (currentPixel.g + errG[x + shift][y] > 255)
			{
				currentPixel.g = 255;
			}
			else if (currentPixel.g + errG[x + shift][y] < 0)
			{
				currentPixel.g = 0;
			}
			else
			{
				currentPixel.g += errG[x + shift][y];
			}
			if (currentPixel.b + errB[x + shift][y] > 255)
			{
				currentPixel.b = 255;
			}
			else if (currentPixel.b + errB[x + shift][y] < 0)
			{
				currentPixel.b = 0;
			}
			else
			{
				currentPixel.b += errB[x + shift][y];
			}

			actualColor = colorTable->colors[findClosestColorIndexFromPalette(currentPixel, colorTable)]; // find closest color

			currErrR = currentPixel.r - actualColor.r;
			currErrG = currentPixel.g - actualColor.g;
			currErrB = currentPixel.b - actualColor.b;

			dstColor[y * infoHeader.biWidth + x] = actualColor;

			// magic happening
			errR[x + 1 + shift][y] += (currErrR * 7.0 / 16.0);
			errR[x + 1 + shift][y + 1] += (currErrR * 1.0 / 16.0);
			errR[x + shift][y + 1] += (currErrR * 5.0 / 16.0);
			errR[x - 1 + shift][y + 1] += (currErrR * 3.0 / 16.0);

			errG[x + 1 + shift][y] += (currErrG * 7.0 / 16.0);
			errG[x + 1 + shift][y + 1] += (currErrG * 1.0 / 16.0);
			errG[x + shift][y + 1] += (currErrG * 5.0 / 16.0);
			errG[x - 1 + shift][y + 1] += (currErrG * 3.0 / 16.0);

			errB[x + 1 + shift][y] += (currErrB * 7.0 / 16.0);
			errB[x + 1 + shift][y + 1] += (currErrB * 1.0 / 16.0);
			errB[x + shift][y + 1] += (currErrB * 5.0 / 16.0);
			errB[x - 1 + shift][y + 1] += (currErrB * 3.0 / 16.0);
		}
	}
}

void grayscaleIn5bits(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader)
{
	uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
	for (int i = 0; i < dataSizeWithoutPitches; i += 3)
	{
		uint8_t BW;
		uint8_t r = src[i];
		uint8_t g = src[i + 1];
		uint8_t b = src[i + 2];
		BW = 0.299 * r + 0.587 * g + 0.114 * b;
		dst[i / 3] = (BW >> 3);
	}
}
void grayscaleIn24bits(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader)
{
	uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
	for (int i = 0; i < dataSizeWithoutPitches; i += 3)
	{
		uint8_t BW;
		uint8_t r = src[i];
		uint8_t g = src[i + 1];
		uint8_t b = src[i + 2];
		BW = 0.299 * r + 0.587 * g + 0.114 * b;
		dst[i] = BW;
		dst[i + 1] = BW;
		dst[i + 2] = BW;
	}
}
void transcodePixels5bits(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader)
{
	uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
	// assuming BMP is 24 bpp
	// blue is least important color so using one bit less for this color
	for (int i = 0; i < dataSizeWithoutPitches; i += 3)
	{
		uint8_t r = src[i];
		uint8_t g = src[i + 1];
		uint8_t b = src[i + 2];

		r = r >> 6;
		g = g >> 6;
		b = b >> 7;
		dst[i / 3] = r << 3;
		dst[i / 3] = dst[i / 3] | (g << 1);
		dst[i / 3] = dst[i / 3] | b;
	}
}
void transcodePixels5bitsGrayscale(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader)
{
	uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
	for (int i = 0; i < dataSizeWithoutPitches; i += 3)
	{
		uint8_t r = src[i];
		dst[i / 3] = r >> 3;
	}
}