#include "ColorAlgorithms.h"

uint8_t findClosestColorIndexFromPalette(RGB_color needle, BSDM_PALETTE* palette)
{
	// search for smallest Euclidean distance
	uint8_t index;
	uint32_t d, minimal = 255 * 255 * 3 + 1;
	uint32_t dR, dG, dB;
	RGB_color current;

	for (int i = 0; i < palette->numberOfColors; i++)
	{
		current = palette->colors[i];
		dR = needle.r - current.r;
		dG = needle.g - current.g;
		dB = needle.b - current.b;
		d = dR * dR + dG * dG + dB * dB;
		if (d < minimal)
		{
			minimal = d;
			index = i;
		}
	}
	return index;
}

MCTriplet MCTripletMake(uint8_t r, uint8_t g, uint8_t b)
{
	MCTriplet triplet;
	triplet.value[0] = r;
	triplet.value[1] = g;
	triplet.value[2] = b;

	return triplet;
}

void MCShrinkCube(MCCube* cube)
{
	uint8_t r, g, b;
	MCTriplet* data;

	data = cube->data;

	cube->min = MCTripletMake(0xFF, 0xFF, 0xFF);
	cube->max = MCTripletMake(0x00, 0x00, 0x00);

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

MCTriplet MCCubeAverage(MCCube* cube)
{
	return MCTripletMake(
		(cube->max.value[0] + cube->min.value[0]) / 2,
		(cube->max.value[1] + cube->min.value[1]) / 2,
		(cube->max.value[2] + cube->min.value[2]) / 2
	);
}

void MCCalculateBiggestDimension(MCCube* cube)
{
	uint32_t diff = 0;
	uint32_t current;

	for (int i = 0; i < NUM_DIM; i++)
	{
		current = cube->max.value[i] - cube->min.value[i];
		if (current > diff)
		{
			dim = i;
			diff = current;
		}
	}
}

int MCCompareTriplet(const void* a, const void* b)
{
	MCTriplet* t1, * t2;

	t1 = (MCTriplet*)a;
	t2 = (MCTriplet*)b;

	return t1->value[dim] - t2->value[dim];
}

BSDM_PALETTE* MCQuantizeData(MCTriplet* data, uint32_t size, uint8_t level)
{
	BSDM_PALETTE* pall = (BSDM_PALETTE*)malloc(sizeof(BSDM_PALETTE));
	int p_size; /* generated palette size */
	MCCube* cubes;
	MCTriplet* palette;

	p_size = pow(2, level);
	pall->numberOfColors = p_size;
	cubes = (MCCube*)malloc(sizeof(MCCube) * p_size);
	palette = (MCTriplet*)malloc(sizeof(MCTriplet) * p_size);

	/* first cube */
	cubes[0].data = data;
	cubes[0].size = size;
	MCShrinkCube(cubes);

	/* remaining cubes */
	int parentIndex = 0;
	int iLevel = 1; /* iteration level */
	int offset;
	int median;
	MCCube* parentCube;
	while (iLevel <= level)
	{
		parentCube = &cubes[parentIndex];

		MCCalculateBiggestDimension(parentCube);
		qsort(parentCube->data, parentCube->size, sizeof(MCTriplet), MCCompareTriplet);

		median = parentCube->data[parentCube->size / 2].value[dim];

		offset = p_size / pow(2, iLevel);

		/* split cubes */
		cubes[parentIndex + offset] = *parentCube;
		cubes[parentIndex].max.value[dim] = median;
		cubes[parentIndex + offset].min.value[dim] = median + 1;

		/* find new cube data sizes */
		uint32_t newSize = 0;
		while (parentCube->data[newSize].value[dim] < median)
		{
			newSize++;
		}
		/* newSize is now the index of the first element above the
		* median, thus it is also the count of elements below the median */
		cubes[parentIndex].size = newSize;
		cubes[parentIndex + offset].data += newSize;
		cubes[parentIndex + offset].size -= newSize;

		/* shrink new cubes */
		MCShrinkCube(&cubes[parentIndex]);
		MCShrinkCube(&cubes[parentIndex + offset]);

		/* check if iLevel must be increased by analysing if the next
		* offset is within palette size boundary. If not, change level
		* and reset parent to 0. If it is, set next element as parent. */
		if (parentIndex + (offset * 2) < p_size)
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
	for (int i = 0; i < p_size; i++)
	{
		palette[i] = MCCubeAverage(&cubes[i]);
	}

	pall->colors = (RGB_color*)palette;

	free(cubes);

	return pall;
}


