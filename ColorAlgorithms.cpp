#include "ColorAlgorithms.h"

uint8_t findClosestColorIndexFromPalette(RGB_color needle, BSDM_PALETTE* palette)
{
	// search for smallest Euclidean distance
	uint8_t idx;
	uint32_t d;
	uint32_t min = 255 * 255 * 3 + 1;
	uint32_t dR, dG, dB;
	RGB_color curr;

	for (int i = 0; i < palette->numberOfColors; i++)
	{
		curr = palette->colors[i];
		dR = needle.r - curr.r;
		dG = needle.g - curr.g;
		dB = needle.b - curr.b;
		d = dR * dR + dG * dG + dB * dB;
		if (d < min)
		{
			min = d;
			idx = i;
		}
	}
	return idx;
}

RGBBytes makeTriplet(uint8_t r, uint8_t g, uint8_t b)
{
	RGBBytes triplet;
	triplet.value[0] = r;
	triplet.value[1] = g;
	triplet.value[2] = b;

	return triplet;
}

void shrinkCube(Cube* cube)
{
	uint8_t r, g, b;
	RGBBytes* data;

	data = cube->data;
	cube->min = makeTriplet(0xFF, 0xFF, 0xFF);
	cube->max = makeTriplet(0x00, 0x00, 0x00);

	for (int i = 0; i < cube->size; i++)
	{
		r = data[i].value[0];
		g = data[i].value[1];
		b = data[i].value[2];

		if (r < cube->min.value[0]) cube->min.value[0] = r;
		if (g < cube->min.value[1]) cube->min.value[1] = g;
		if (b < cube->min.value[2]) cube->min.value[2] = b;

		if (r > cube->max.value[0]) cube->max.value[0] = r;
		if (g > cube->max.value[1]) cube->max.value[1] = g;
		if (b > cube->max.value[2]) cube->max.value[2] = b;
	}
}

RGBBytes cubeAvg(Cube* cube)
{
	return makeTriplet(
		(cube->max.value[0] + cube->min.value[0]) / 2,
		(cube->max.value[1] + cube->min.value[1]) / 2,
		(cube->max.value[2] + cube->min.value[2]) / 2
	);
}

void calcBiggestDimension(Cube* cube)
{
	uint32_t diff = 0;
	uint32_t current;

	for (int i = 0; i < 3; i++)
	{
		current = cube->max.value[i] - cube->min.value[i];
		if (current > diff)
		{
			dim = i;
			diff = current;
		}
	}
}

int cmpTriplet(const void* a, const void* b)
{
	RGBBytes* t1, * t2;

	t1 = (RGBBytes*)a;
	t2 = (RGBBytes*)b;

	return t1->value[dim] - t2->value[dim];
}

BSDM_PALETTE* medianCut(RGBBytes* data, uint32_t size)
{
	BSDM_PALETTE* pall = (BSDM_PALETTE*)malloc(sizeof(BSDM_PALETTE));
	int pSize; /* generated palette size */
	RGBBytes* palette;
	Cube* cubes;

	pSize = pow(2, 5);
	pall->numberOfColors = pSize;
	cubes = (Cube*)malloc(sizeof(Cube) * pSize);
	palette = (RGBBytes*)malloc(sizeof(RGBBytes) * pSize);

	cubes[0].data = data; // first cube initialize
	cubes[0].size = size;
	shrinkCube(cubes);

	int parentIndex = 0;
	int iLevel = 1;
	int offset;
	int median;
	Cube* parentCube;
	while (iLevel <= 5)
	{
		parentCube = &cubes[parentIndex];

		calcBiggestDimension(parentCube);
		qsort(parentCube->data, parentCube->size, sizeof(RGBBytes), cmpTriplet);

		median = parentCube->data[parentCube->size / 2].value[dim];

		offset = pSize / pow(2, iLevel);

		// split
		cubes[parentIndex + offset] = *parentCube;
		cubes[parentIndex].max.value[dim] = median;
		cubes[parentIndex + offset].min.value[dim] = median + 1;

		// new cube size
		uint32_t newSize = 0;
		while (parentCube->data[newSize].value[dim] < median)
		{
			newSize++;
		}
		// newSize is now the index of the first element above the
		// median, thus it is also the count of elements below the median 
		cubes[parentIndex].size = newSize;
		cubes[parentIndex + offset].data += newSize;
		cubes[parentIndex + offset].size -= newSize;

		// shrink
		shrinkCube(&cubes[parentIndex]);
		shrinkCube(&cubes[parentIndex + offset]);

		// check if iLevel must be increased by analysing if the next
		// offset is within palette size boundary. If not, change level
		// and reset parent to 0. If it is, set next element as parent. 
		if (parentIndex + (offset * 2) < pSize)
		{
			parentIndex = parentIndex + (offset * 2);
		}
		else
		{
			parentIndex = 0;
			iLevel++;
		}
	}

	/* find final cube averages */
	for (int i = 0; i < pSize; i++)
	{
		palette[i] = cubeAvg(&cubes[i]);
	}

	pall->colors = (RGB_color*)palette;
	free(cubes);
	return pall;
}

