#include "Files.h"

void saveBSDMPalette(FILE* fBSDM, BSDM_PALETTE* palette)
{
	uint8_t colorsPaletteNumber = palette->numberOfColors;
	fwrite(&(palette->numberOfColors), 1, 1, fBSDM); // write number of palette colors
	for (int i = 0; i < palette->numberOfColors * 3; i += 3)
	{
		fwrite(palette->colors + i / 3, 3, 1, fBSDM);
	}
}
void readBSDMPalette(FILE* fBSDM, BSDM_PALETTE& palette)
{
	uint8_t nColors;
	fread(&nColors, 1, 1, fBSDM);
	palette.numberOfColors = nColors;
	uint32_t size = sizeof(RGB_color) * nColors;
	RGB_color* colors = (RGB_color*)malloc(size);
	fread(colors, size, 1, fBSDM);
	palette.colors = colors;
}