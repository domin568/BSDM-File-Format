#ifndef  FILES_H
#define FILES_H
#include "Color.h"
#pragma pack(1)
template<typename T>
bool readHelper(FILE* file, T* data)
{
	bool isRead = fread(data, sizeof(*data), 1, file);

	if (isRead)
	{
		return true;
	}
	return false;
}

void saveBSDMPalette(FILE* fBSDM, BSDM_PALETTE* palette);
void readBSDMPalette(FILE* fBSDM, BSDM_PALETTE& palette);
#endif // ! FILES_H
