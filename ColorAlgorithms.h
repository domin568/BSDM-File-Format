#ifndef  COLORALGORITHMS_H
#define COLORALGORITHMS_H
#include "Color.h"
#pragma pack(1)

typedef struct
{
	uint8_t value[3];
} MCTriplet;

#define NUM_DIM 3

typedef struct
{
	MCTriplet min;
	MCTriplet max;
	uint32_t size;
	MCTriplet* data;
} MCCube;
static uint8_t dim = 0; /* current cube biggest dimension */

uint8_t findClosestColorIndexFromPalette(RGB_color needle, BSDM_PALETTE* palette);
MCTriplet MCTripletMake(uint8_t r, uint8_t g, uint8_t b);
void MCShrinkCube(MCCube* cube);
MCTriplet MCCubeAverage(MCCube* cube);
void MCCalculateBiggestDimension(MCCube* cube);
int MCCompareTriplet(const void* a, const void* b);
BSDM_PALETTE* MCQuantizeData(MCTriplet* data, uint32_t size, uint8_t level);
#endif // ! COLORALGORITHMS_H
