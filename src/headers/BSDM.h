#ifndef  BSDM_H
#define BSDM_H
#include <fstream>
#include "FileStructs.h"
#pragma pack(1)
uint8_t* readRawBSDMData(FILE* bsdmFile, const BSDMHEADER& header);
#endif // ! BSDM_H
#pragma once
