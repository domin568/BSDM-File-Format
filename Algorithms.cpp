#include "Algorithms.h"

void ditheringColor(uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader, BSDM_PALETTE* colorTable)
{
	RGB_color currentPixel;
	RGB_color choosenColor;
	RGB_color* srcColor = (RGB_color*)src;
	RGB_color* dstColor = (RGB_color*)dst;

	// Storage for errors in color conversion
	float** errorsR = new float* [infoHeader.biWidth + 2];
	float** errorsG = new float* [infoHeader.biWidth + 2];
	float** errorsB = new float* [infoHeader.biWidth + 2];
	for (int i = 0; i < infoHeader.biWidth + 2; ++i) {
		errorsR[i] = new float[infoHeader.biHeight + 2]{ 0 };
		errorsG[i] = new float[infoHeader.biHeight + 2]{ 0 };
		errorsB[i] = new float[infoHeader.biHeight + 2]{ 0 };
	}

	int shift = 1; // To not exceed table range
	float currentErrorR = 0, currentErrorG = 0, currentErrorB = 0;

	for (int y = 0; y < infoHeader.biHeight; y++)
		for (int x = 0; x < infoHeader.biWidth; x++) {
			//currentPixel = bmp.getPixel(x, y);
			currentPixel = srcColor[y * infoHeader.biWidth + x];

			// Passing current error to current Pixel with overflow protetcion
			if (currentPixel.r + errorsR[x + shift][y] > 255)
				currentPixel.r = 255;
			else if (currentPixel.r + errorsR[x + shift][y] < 0)
				currentPixel.r = 0;
			else
				currentPixel.r += errorsR[x + shift][y];

			if (currentPixel.g + errorsG[x + shift][y] > 255)
				currentPixel.g = 255;
			else if (currentPixel.g + errorsG[x + shift][y] < 0)
				currentPixel.g = 0;
			else
				currentPixel.g += errorsG[x + shift][y];

			if (currentPixel.b + errorsB[x + shift][y] > 255)
				currentPixel.b = 255;
			else if (currentPixel.b + errorsB[x + shift][y] < 0)
				currentPixel.b = 0;
			else
				currentPixel.b += errorsB[x + shift][y];

			// Matching color from give table
			//choosenColor =
			//    colorTable[findClosestColorIndexFromTable(currentPixel, colorTable)];
			choosenColor = colorTable->colors[findClosestColorIndexFromPalette(currentPixel, colorTable)];

			// Calculating error from current pixel
			currentErrorR = currentPixel.r - choosenColor.r;
			currentErrorG = currentPixel.g - choosenColor.g;
			currentErrorB = currentPixel.b - choosenColor.b;

			dstColor[y * infoHeader.biWidth + x] = choosenColor;
			//bmp.setPixel(x, y, choosenColor);

			// Pasing errors from current pixels to rest as determined in
			// Floyd–Steinber alg
			errorsR[x + 1 + shift][y] += (currentErrorR * 7.0 / 16.0);
			errorsR[x + 1 + shift][y + 1] += (currentErrorR * 1.0 / 16.0);
			errorsR[x + shift][y + 1] += (currentErrorR * 5.0 / 16.0);
			errorsR[x - 1 + shift][y + 1] += (currentErrorR * 3.0 / 16.0);

			errorsG[x + 1 + shift][y] += (currentErrorG * 7.0 / 16.0);
			errorsG[x + 1 + shift][y + 1] += (currentErrorG * 1.0 / 16.0);
			errorsG[x + shift][y + 1] += (currentErrorG * 5.0 / 16.0);
			errorsG[x - 1 + shift][y + 1] += (currentErrorG * 3.0 / 16.0);

			errorsB[x + 1 + shift][y] += (currentErrorB * 7.0 / 16.0);
			errorsB[x + 1 + shift][y + 1] += (currentErrorB * 1.0 / 16.0);
			errorsB[x + shift][y + 1] += (currentErrorB * 5.0 / 16.0);
			errorsB[x - 1 + shift][y + 1] += (currentErrorB * 3.0 / 16.0);
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
	// direction == 0 --> BMP to BSDM
	// direction == 1 --> BSDM to BMP

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