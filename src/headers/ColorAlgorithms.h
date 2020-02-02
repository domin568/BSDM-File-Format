#ifndef  COLORALGORITHMS_H
#define COLORALGORITHMS_H
#include "Color.h"
#include <math.h>
#pragma pack(1)


typedef struct
{
	uint8_t value[3];
} RGBBytes;

typedef struct
{
	RGBBytes min;
	RGBBytes max;
	uint32_t size;
	RGBBytes* data;
} Cube;
static uint8_t dim = 0; /* current cube biggest dimension */

uint8_t findClosestColorIndexFromPalette(RGB_color needle, BSDM_PALETTE* palette);
RGBBytes makeTriplet(uint8_t r, uint8_t g, uint8_t b);
void shrinkCube(Cube* cube);
RGBBytes cubeAvg(Cube* cube);
void calcBiggestDimension(Cube* cube);
int cmpTriplet(const void* a, const void* b);
BSDM_PALETTE* medianCut(RGBBytes* data, uint32_t size);
#endif // ! COLORALGORITHMS_H
