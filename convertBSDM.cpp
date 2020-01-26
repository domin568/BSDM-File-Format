#include <iostream>
#include <fstream>
#include <math.h>
#include <map>
#include <vector>
#include <inttypes.h>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <cstring>
#include <algorithm>

#pragma pack(1)

struct BITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned int reserved;
    unsigned int bfOffBits;

};


struct BITMAPINFOHEADER
{
    unsigned int biSize;
    unsigned int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;

};

struct RGB_color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    friend bool operator == (const RGB_color& checked, const RGB_color& thisclass)
    {
        if (checked.r == thisclass.r && checked.g == thisclass.g && checked.b == thisclass.b)
        {
            return true;
        }
        return false;
    }
    friend std::ostream& operator << (std::ostream& out, RGB_color& color)
    {
        out << std::hex << "r : " << (int)color.r << " " << "g : " << (int)color.g << " " << "b : " << (int)color.b;
        return out;
    }
};

struct BSDMHEADER
{
    uint32_t magic;
    uint32_t width;
    int32_t height;
    uint8_t bitsPerPixel;
    uint8_t mode;            // color, grayscale       
    uint8_t isCustomPalette; // true,false
    uint32_t sizeOfHeader;   // 0x14
    uint8_t LZWCodeLength;
};

struct BSDM_PALETTE
{
    uint8_t numberOfColors;
    RGB_color* colors;
};

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

std::list<int> ReadCompressedData(FILE* bsdmFile, const BSDMHEADER& header, BSDM_PALETTE& palette) 
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

RGB_color defaultcolors[32] =
{
    {0,0,0},
    {0,0,255},
    {0,85,0},
    {0,85,255},
    {0,170,0},
    {0,170,255},
    {0,255,0},
    {0,255,255},
    {85,0,0},
    {85,0,255},
    {85,85,0},
    {85,85,255},
    {85,170,0},
    {85,170,255},
    {85,255,0},
    {85,255,255},
    {170,0,0},
    {170,0,255},
    {170,85,0},
    {170,85,255},
    {170,170,0},
    {170,170,255},
    {170,255,0},
    {170,255,255},
    {255,0,0},
    {255,0,255},
    {255,85,0},
    {255,85,255},
    {255,170,0},
    {255,170,255},
    {255,255,0},
    {255,255,255}
};

BSDM_PALETTE defaultPalette
{
    32, defaultcolors
};

RGB_color graycolors[32] =
{
    {0,0,0},
    {8,8,8},
    {16,16,16},
    {24,24,24},
    {32,32,32},
    {41,41,41},
    {49,49,49},
    {57,57,57},
    {65,65,65},
    {74,74,74},
    {82,82,82},
    {90,90,90},
    {98,98,98},
    {106,106,106},
    {115,115,115},
    {123,123,123},
    {131,131,131},
    {139,139,139},
    {148,148,148},
    {156,156,156},
    {164,164,164},
    {172,172,172},
    {180,180,180},
    {189,189,189},
    {197,197,197},
    {205,205,205},
    {213,213,213},
    {222,222,222},
    {230,230,230},
    {238,238,238},
    {246,246,246},
    {255,255,255}
};

BSDM_PALETTE grayPalette
{
    32,graycolors
};

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
        errR[i] = new float[infoHeader.biHeight + 2] {0};
        errG[i] = new float[infoHeader.biHeight + 2] {0};
        errB[i] = new float[infoHeader.biHeight + 2] {0};
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
int readBMPHeaders(FILE* bmpFile, BITMAPFILEHEADER& fileHeader, BITMAPINFOHEADER& infoHeader)
{
    if (!(readHelper(bmpFile, &fileHeader.bfType) &&
        readHelper(bmpFile, &fileHeader.bfSize) &&
        readHelper(bmpFile, &fileHeader.reserved) &&
        readHelper(bmpFile, &fileHeader.bfOffBits)))
    {
        std::cout << "[!] Failed to parse fileHeader" << std::endl;
        return 1;
    }

    if (!(readHelper(bmpFile, &infoHeader.biSize) &&
        readHelper(bmpFile, &infoHeader.biWidth) &&
        readHelper(bmpFile, &infoHeader.biHeight) &&
        readHelper(bmpFile, &infoHeader.biPlanes) &&
        readHelper(bmpFile, &infoHeader.biBitCount) &&
        readHelper(bmpFile, &infoHeader.biCompression) &&
        readHelper(bmpFile, &infoHeader.biSizeImage) &&
        readHelper(bmpFile, &infoHeader.biXPelsPerMeter) &&
        readHelper(bmpFile, &infoHeader.biYPelsPerMeter) &&
        readHelper(bmpFile, &infoHeader.biClrUsed) &&
        readHelper(bmpFile, &infoHeader.biClrImportant)
        ))
    {
        std::cout << "[!] Failed to parse infoHeader" << std::endl;
        return 2;
    }
    infoHeader.biHeight = std::abs(infoHeader.biHeight); // protection against negative height 
    return 0;
}
bool readBSDMHeader(FILE* bsdmFile, BSDMHEADER& header)
{
    if (!(readHelper(bsdmFile, &header.magic) &&
        readHelper(bsdmFile, &header.width) &&
        readHelper(bsdmFile, &header.height) &&
        readHelper(bsdmFile, &header.bitsPerPixel) &&
        readHelper(bsdmFile, &header.mode) &&
        readHelper(bsdmFile, &header.isCustomPalette) &&
        readHelper(bsdmFile, &header.sizeOfHeader) &&
        readHelper(bsdmFile, &header.LZWCodeLength)
        ))
    {
        std::cout << "[!] Failed to parse fileHeader" << std::endl;
        return 1;
    }
    if (header.magic != 0x4D445342)
    {
        return 1;
    }
    return 0;
}
uint8_t* readRawBMPData(FILE* bmpFile, const BITMAPFILEHEADER& fileHeader, const BITMAPINFOHEADER& infoHeader)
{
    int pitch = (infoHeader.biWidth * 3 + 3) & ~3U;

    fseek(bmpFile, fileHeader.bfOffBits, 0);
    uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;

    uint8_t* data = new uint8_t[dataSizeWithoutPitches];

    uint8_t* line = new uint8_t[pitch];
    uint8_t tmp;
    int offset = dataSizeWithoutPitches - infoHeader.biWidth * 3;
    for (int i = 0; i < infoHeader.biHeight; i++)
    {
        fread(line, 1, pitch, bmpFile);
        for (int j = 0; j < infoHeader.biWidth * 3; j += 3)
        {
            //BGR to RGB
            tmp = line[j];
            line[j] = line[j + 2];
            line[j + 2] = tmp;
        }

        memcpy(data + offset, line, infoHeader.biWidth * 3);
        offset -= infoHeader.biWidth * 3;
    }
    return data;
}
uint8_t* readRawBSDMData(FILE* bsdmFile, const BSDMHEADER& header)
{
    fseek(bsdmFile, header.sizeOfHeader, 0);
    uint32_t rawDataSize = header.width * header.height;
    uint8_t* data = new uint8_t[rawDataSize];
    fread(data, 1, rawDataSize, bsdmFile);
    return data;
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

RGBBytes makeTriplet(uint8_t r, uint8_t g, uint8_t b)
{
    RGBBytes triplet;
    triplet.value[0] = r;
    triplet.value[1] = g;
    triplet.value[2] = b;

    return triplet;
}

void shrinkCube(Cube * cube)
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

RGBBytes cubeAvg(Cube * cube)
{
    return makeTriplet(
        (cube->max.value[0] + cube->min.value[0]) / 2,
        (cube->max.value[1] + cube->min.value[1]) / 2,
        (cube->max.value[2] + cube->min.value[2]) / 2
    );
}

void calcBiggestDimension(Cube * cube)
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

int cmpTriplet(const void * a, const void * b)
{
    RGBBytes* t1, * t2;

    t1 = (RGBBytes *) a;
    t2 = (RGBBytes *) b;

    return t1->value[dim] - t2->value[dim];
}

BSDM_PALETTE* medianCut(RGBBytes * data, uint32_t size)
{
    BSDM_PALETTE* pall = (BSDM_PALETTE*) malloc(sizeof(BSDM_PALETTE));
    int pSize; /* generated palette size */
    RGBBytes* palette;
    Cube* cubes;
    
    pSize = pow(2, 5);
    pall->numberOfColors = pSize;
    cubes = (Cube*) malloc(sizeof (Cube) * pSize);
    palette = (RGBBytes*) malloc(sizeof(RGBBytes) * pSize);

    cubes[0].data = data; // first cube initialize
    cubes[0].size = size;
    shrinkCube(cubes);

    int parentIndex = 0;
    int iLevel = 1; 
    int offset;
    int median;
    Cube * parentCube;
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

    pall->colors = (RGB_color*) palette;
    free(cubes);
    return pall;
}

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

void convertSaveBMPPalette(FILE* fBSDM, FILE* fBMP, BSDM_PALETTE& palette, const BSDMHEADER& header)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    memset(&fileHeader, 0, sizeof(fileHeader));
    memset(&infoHeader, 0, sizeof(infoHeader));

    fileHeader.bfType = 0x4d42;
    fileHeader.bfSize = 0; // not important
    fileHeader.bfOffBits = 0x36;

    fwrite(&fileHeader, sizeof(fileHeader), 1, fBMP);

    infoHeader.biSize = 0x28;
    infoHeader.biWidth = header.width;
    infoHeader.biHeight = -header.height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 0x18;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = header.height * ((header.width + 3) & ~3U) * (infoHeader.biBitCount / 8);
    infoHeader.biXPelsPerMeter = 1;
    infoHeader.biYPelsPerMeter = 1;

    fwrite(&infoHeader, sizeof(infoHeader), 1, fBMP);

    uint32_t pitch = (infoHeader.biWidth * 3 + 3) & ~3U;

    uint32_t bmpRawDataSize = pitch * header.height;

    uint32_t size = header.width * header.height;
    uint8_t* colorsData = new uint8_t[size];

    std::list<int> compressed = ReadCompressedData(fBSDM, header, palette);
    std::string decompressed = lzw_decompress(compressed);
    colorsData = ConvertStringtoBDSMrawData(decompressed, header.width * header.height);

    uint32_t pitchDiff = pitch - infoHeader.biWidth * 3;
    uint32_t colorsOfffset = 0;

    for (int i = 0; i < size; i++)
    {
        uint8_t nColor = colorsData[i];
        fwrite(&palette.colors[nColor].b, 1, 1, fBMP);
        fwrite(&palette.colors[nColor].g, 1, 1, fBMP);
        fwrite(&palette.colors[nColor].r, 1, 1, fBMP);
        if ((i % infoHeader.biWidth) + 1 == infoHeader.biWidth && pitchDiff)
        {
            uint8_t c = 0xCD; // padding
            fwrite(&c, pitchDiff, 1, fBMP);
        }
    }
    std::cout << "Size: " << header.width << "x" << header.height << "\n";
    std::cout << "Custom palette: " << 1 << "\n";
    std::cout << "Grayscale: " << 0 << "\n";
}

void saveColorNumbers(FILE* fBSDM, BSDM_PALETTE* palette, RGB_color* rawBMPBitmapData, const BITMAPINFOHEADER& infoHeader, BSDMHEADER& bsdm_header)
{
    RGB_color* pixel;
    uint8_t* toCompress = new uint8_t[infoHeader.biHeight * infoHeader.biWidth];

    for (int i = 0; i < infoHeader.biHeight; i++)
    {
        for (int j = 0; j < infoHeader.biWidth; j++)
        {
            pixel = &rawBMPBitmapData[i * infoHeader.biWidth + j];
            uint8_t idx = findClosestColorIndexFromPalette(*pixel, palette);
            toCompress[(i * infoHeader.biWidth) + j] = idx;
        }
    }
    std::list<int> compressed = lzw_compression(toCompress, infoHeader.biHeight * infoHeader.biWidth);

    bsdm_header.LZWCodeLength = MinNumBits(compressed);
    fseek(fBSDM, sizeof(BSDMHEADER) - 1, 0);
    fwrite(&bsdm_header.LZWCodeLength, 1, 1, fBSDM);
    fseek(fBSDM, 0, SEEK_END);
    float LZW = (float)bsdm_header.LZWCodeLength;
    int bytesNeeded = ceil(LZW / 8);

    for (auto it = compressed.begin(); it != compressed.end(); it++) 
    {
        fwrite((char*) & *it, bytesNeeded, 1, fBSDM);
    }
}
uint8_t* fiveBitToRGB(uint8_t* src, const BITMAPINFOHEADER& infoHeader)
{
    RGB_color* data = new RGB_color[infoHeader.biWidth * infoHeader.biHeight];
    for (int i = 0; i < infoHeader.biHeight * infoHeader.biWidth; i++)
    {
        data[i].r = src[i] * (255 / 31);
        data[i].g = src[i] * (255 / 31);
        data[i].b = src[i] * (255 / 31);
    }
    return (uint8_t*)data;
}
void convertSaveBSDM(FILE* fBSDM, const BITMAPFILEHEADER& fileHeader, const BITMAPINFOHEADER& infoHeader, uint8_t* rawBMPBitmapData, bool mode, bool isDedicatedPaletteUsed, bool dithering)
{
    BSDMHEADER bsdm_header;
    memcpy(&bsdm_header, "BSDM", 4);
    bsdm_header.width = infoHeader.biWidth;
    bsdm_header.height = infoHeader.biHeight;
    if (isDedicatedPaletteUsed)
    {
        bsdm_header.bitsPerPixel = 24;
    }
    else
    {
        bsdm_header.bitsPerPixel = 5;
    }
    bsdm_header.mode = mode;
    bsdm_header.isCustomPalette = isDedicatedPaletteUsed;
    bsdm_header.sizeOfHeader = sizeof(BSDMHEADER);

    fwrite(&bsdm_header, 1, sizeof(BSDMHEADER), fBSDM);

    uint8_t* rawBSDMBitmapData = new uint8_t[bsdm_header.width * bsdm_header.height];
    uint8_t* rawDitheringData = new uint8_t[bsdm_header.width * bsdm_header.height * 3];

    if (bsdm_header.isCustomPalette)
    {
        uint32_t s = bsdm_header.width * bsdm_header.height * 3;
        uint8_t* rawBMPBitmapDataQuantinize = (uint8_t*)malloc(s);
        memcpy(rawBMPBitmapDataQuantinize, rawBMPBitmapData, s);

        BSDM_PALETTE* palette = medianCut((RGBBytes*)rawBMPBitmapDataQuantinize, bsdm_header.width * bsdm_header.height);
        if (dithering)
        {
            std::cout << "Custom palette with dithering" << std::endl;
            ditheringColor(rawBMPBitmapData, rawDitheringData, infoHeader, palette);
            transcodePixels5bits(rawDitheringData, rawBSDMBitmapData, infoHeader);
            saveBSDMPalette(fBSDM, palette);
            saveColorNumbers(fBSDM, palette, (RGB_color*)rawDitheringData, infoHeader, bsdm_header);
        }
        else
        {
            std::cout << "Custom palette only\n";
            saveBSDMPalette(fBSDM, palette);
            saveColorNumbers(fBSDM, palette, (RGB_color*)rawBMPBitmapData, infoHeader, bsdm_header);
        }
        free(rawBMPBitmapDataQuantinize);
    }
    else
    {
        if (dithering)
        {
            if (mode)
            {
                std::cout << "Dithering with grayscale on default palette" << std::endl;
                grayscaleIn24bits(rawBMPBitmapData, rawDitheringData, infoHeader);
                ditheringColor(rawDitheringData, rawDitheringData, infoHeader, &grayPalette);
                transcodePixels5bitsGrayscale(rawDitheringData, rawBSDMBitmapData, infoHeader);
            }
            else
            {
                std::cout << "Dithering only on default palette" << std::endl;
                ditheringColor(rawBMPBitmapData, rawDitheringData, infoHeader, &defaultPalette);
                transcodePixels5bits(rawDitheringData, rawBSDMBitmapData, infoHeader);
            }
        }
        else
        {
            if (mode)
            {
                std::cout << "Grayscale only on default palette" << std::endl;
                grayscaleIn5bits(rawBMPBitmapData, rawBSDMBitmapData, infoHeader);
            }
            else
            {
                std::cout << "Defaut palette only" << std::endl;
                transcodePixels5bits(rawBMPBitmapData, rawBSDMBitmapData, infoHeader);
            }

        }

        std::list<int> compressed = lzw_compression(rawBSDMBitmapData, bsdm_header.width * bsdm_header.height);
        bsdm_header.LZWCodeLength = MinNumBits(compressed);
        fseek(fBSDM, sizeof(BSDMHEADER) - 1, 0);
        fwrite(&bsdm_header.LZWCodeLength, 1, 1, fBSDM);
        fseek(fBSDM, 0, SEEK_END);
        float LZW = (float)bsdm_header.LZWCodeLength;
        int bytesNeeded = ceil(LZW / 8);

        for (auto it = compressed.begin(); it != compressed.end(); it++) 
        {
            fwrite((char*) & *it, bytesNeeded, 1, fBSDM);
        }
    }
    unsigned long len = (unsigned long)ftell(fBSDM);
    std::cout << fileHeader.bfSize << " bytes -> ";
    std::cout << len << " bytes \n";
    std::cout << bsdm_header.width << "x" << bsdm_header.height << std::endl;
    fclose(fBSDM);
}
void convertSaveBMP(FILE* fBMP, uint8_t* rawBSDMBitmapData, const BSDMHEADER& bsdmHeader, bool dithering)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    memset(&fileHeader, 0, sizeof(fileHeader));
    memset(&infoHeader, 0, sizeof(infoHeader));

    fileHeader.bfType = 0x4d42;
    fileHeader.bfSize = 0; // not important
    fileHeader.bfOffBits = 0x36;

    fwrite(&fileHeader, sizeof(fileHeader), 1, fBMP);
    bool mode = bsdmHeader.mode;
    infoHeader.biSize = 0x28;
    infoHeader.biWidth = bsdmHeader.width;
    infoHeader.biHeight = -bsdmHeader.height; // !
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 0x18;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = bsdmHeader.height * ((bsdmHeader.width + 3) & ~3U) * (infoHeader.biBitCount / 8);
    infoHeader.biXPelsPerMeter = 1;
    infoHeader.biYPelsPerMeter = 1;

    fwrite(&infoHeader, sizeof(infoHeader), 1, fBMP);

    uint32_t rawDataSize = bsdmHeader.width * bsdmHeader.height;
    uint8_t* rawBitmapData = new uint8_t[infoHeader.biSizeImage];

    uint32_t pitch = (infoHeader.biWidth * 3 + 3) & ~3U;
    uint32_t widthPitchDiff = (pitch - bsdmHeader.width) * 3;
    std::cout << "Size: " << infoHeader.biWidth << "x" << bsdmHeader.height << "\n";
    std::cout << "Custom palette: " << (int)bsdmHeader.isCustomPalette << "\n";
    std::cout << "Grayscale: " << (int)bsdmHeader.mode << "\n";

    if (mode)
    {
        for (int i = 0; i < std::abs(infoHeader.biHeight); i++) // negative height
        {
            for (int j = 0; j < pitch; j += 3)
            {
                rawBitmapData[i * pitch + j] = j < infoHeader.biWidth * 3 ? \
                    (rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD;  // BW

                rawBitmapData[i * pitch + j + 1] = j < infoHeader.biWidth * 3 ? \
                    (rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD;  // BW

                rawBitmapData[i * pitch + j + 2] = j < infoHeader.biWidth * 3 ? \
                    (rawBSDMBitmapData[i * bsdmHeader.width + j / 3] * 255 / 31) : 0xCD; //  BW
            }
        }
    }
    else
    {
        for (int i = 0; i < std::abs(infoHeader.biHeight); i++) // negative height
        {
            for (int j = 0; j < pitch; j += 3)
            {

                uint8_t b = j < infoHeader.biWidth * 3 ? (rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x1) * 0xff : 0xCD;  // B
                rawBitmapData[i * pitch + j] = b;

                uint8_t g = j < infoHeader.biWidth * 3 ? ((rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x6) >> 1) * 0x55 : 0xCD;  // G
                rawBitmapData[i * pitch + j + 1] = g;

                uint8_t r = j < infoHeader.biWidth * 3 ? ((rawBSDMBitmapData[i * bsdmHeader.width + j / 3] & 0x18) >> 3) * 0x55 : 0xCD; // R
                rawBitmapData[i * pitch + j + 2] = r;
            }
        }
    }
    fwrite(rawBitmapData, infoHeader.biSizeImage, 1, fBMP);
}


int main(int argc, const char* argv[])
{
    bool customPalette = false;
    bool dithering = false;
    bool grayscale = false;


    if (argc < 3)
    {
        std::cout << "[!] Usage: ./bsdm  <in> <out> [--custom-palette] [--dithering] [--grayscale]" << std::endl;
    }

    for (int i = 3; i < argc; i++)
    {
        if (!strcmp(argv[i], "--custom-palette"))
        {
            customPalette = true;
        }
        if (!strcmp(argv[i], "--dithering"))
        {
            dithering = true;
        }

        if (!strcmp(argv[i], "--grayscale"))
        {
            grayscale = true;
        }

    }

    FILE* in = fopen(argv[1], "rb");
    FILE* out = fopen(argv[2], "wb");

    if (in == NULL)
    {
        std::cout << "[!] Cannot open input file !" << std::endl;
        return -2;
    }
    if (out == NULL)
    {
        std::cout << "[!] Cannot open output file !" << std::endl;
        return -3;
    }

    uint8_t* rawBSDMBitmapData;

    char magic[4];

    if (fread(magic, 1, 4, in) != 4)
    {
        return -1;
    }
    fseek(in, 0, 0); // set to begining 

    if (!strncmp(magic, "BM", 2)) // BMP to BSDM
    {
        BITMAPFILEHEADER BMPfileHeaderIN;
        BITMAPINFOHEADER BMPinfoHeaderIN;

        if (readBMPHeaders(in, BMPfileHeaderIN, BMPinfoHeaderIN))
        {
            return 1;
        }
        uint8_t* rawBMPBitmapData = readRawBMPData(in, BMPfileHeaderIN, BMPinfoHeaderIN);
        convertSaveBSDM(out, BMPfileHeaderIN, BMPinfoHeaderIN, rawBMPBitmapData, grayscale, customPalette, dithering);
    }
    else if (!strncmp(magic, "BSDM", 4)) // BSDM to BMP, 24 bit RGB Windows bitmap
    {
        BSDMHEADER BSDMheaderIN;

        if (readBSDMHeader(in, BSDMheaderIN))
        {
            return 1;
        }

        if (BSDMheaderIN.isCustomPalette)
        {
            BSDM_PALETTE palette;
            readBSDMPalette(in, palette);
            convertSaveBMPPalette(in, out, palette, BSDMheaderIN);
        }
        else
        {
            BSDM_PALETTE null;
            std::list<int> compressed = ReadCompressedData(in, BSDMheaderIN, null);
            std::string decompressed = lzw_decompress(compressed);
            uint8_t* rawBSDMBitmapData = new uint8_t[BSDMheaderIN.width * BSDMheaderIN.height];

            rawBSDMBitmapData = ConvertStringtoBDSMrawData(decompressed, BSDMheaderIN.width * BSDMheaderIN.height);
            convertSaveBMP(out, rawBSDMBitmapData, BSDMheaderIN, dithering);
        }
    }
    else
    {
        std::cout << "[!] Unknown file format !" << std::endl;
    }

    fclose(in);
    fclose(out);

    return 0;
}
