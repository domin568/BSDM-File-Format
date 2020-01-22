//  Created by Dominik Tamiołło on 19.03.2016.

#include <iostream>
#include <fstream>
#include <math.h>
#include <map>
#include <vector>
#include <inttypes.h>
#include <string>
#include <vector>
#include <list>

struct BITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned int reserved;
    unsigned int bfOffBits;
    
}__attribute__((packed));

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
    
}__attribute__((packed));

struct RGB_color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    friend bool operator == (const RGB_color & checked, const RGB_color & thisclass)
    {
        if (checked.r == thisclass.r && checked.g == thisclass.g && checked.b == thisclass.b)
        {
            return true;
        }
        return false;
    }
    friend std::ostream& operator<<(std::ostream& out, RGB_color & color) //wypisuje tablicę (z numerami pól), pozostawia puste dla wolnych pól
    {
        out << std::hex << "r : " << (int)color.r <<  " " << "g : " << (int)color.g << " " << "b : " << (int)color.b;
        return out;
    } 
};

struct BSDMHEADER 
{
    uint32_t magic;
    uint32_t width;
    uint32_t height;
    uint8_t bitsPerPixel;
    uint8_t mode;            // color, grayscale       
    uint8_t isCustomPalette; // true,false
    uint32_t sizeOfHeader;   // 0x13
}__attribute__((packed));

struct BSDM_PALETTE
{
    uint8_t numberOfColors;
    RGB_color * colors; 
};

std::list<uint8_t> lzw_compression(uint8_t* rawBSDMData, int BSDMDataSize)
{
    std::map <std::string, int> lzwDic;
    int dicSize = 32;

    

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
    std::list<uint8_t> encodedVal;

    for (int i = 0; i < toCompress.length(); i++) {

        curr = toCompress[i];
        pc = prev + curr;
        if (lzwDic.count(pc))
            prev = pc;
        else {

            encodedVal.push_back(lzwDic[prev]);
            lzwDic[pc] = dicSize++;
            prev = std::string(1, curr);

        }
    }

    if (!prev.empty())
        encodedVal.push_back(lzwDic[prev]);

    return encodedVal;
}

std::string lzw_decompress(std::list<uint8_t> encoded) {

    std::map<int, std::string> lzwDic;

    int dicSize = 32;

    for (int i = 0; i < 32; i++) {
        lzwDic[i] = std::string(1, i);
    }

    std::string prev(1, *encoded.begin());
    std::string decompressed = prev;
    std::string in;
    int curr;

    for (auto it = ++(encoded.begin()); it != encoded.end(); ++it) {
        curr = *it;

        if (lzwDic.count(curr))
            in = lzwDic[curr];
        else if (curr == dicSize) {
            in = prev + prev[0];
        }
        else
            throw std::runtime_error("Bad compression of data");


        decompressed += in;
        lzwDic[dicSize++] = prev + in[0];


        prev = in;
    }

    return decompressed;
}

uint8_t *ListToArray(std::list<uint8_t> list)
{

    uint8_t array[list.size()];

    std::copy(list.begin(),list.end(),array);

    return array;
}


std::list <uint8_t> ArrayToList(uint8_t *array)
{

    std::list<uint8_t> ret (array,array+sizeof(array)/sizeof(uint8_t));

    return ret;
}



uint8_t *ConvertStringtoBDSMrawData(std::string decompressed,int BSDMDataSize)
{

    uint8_t *rawBSDMData = new uint8_t[BSDMDataSize];


    for(int i=0;i<BSDMDataSize;i++)
    {
        rawBSDMData = (uint8_t*)decompressed[i];
    }

    return rawBSDMData;
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

 uint8_t findClosestColorIndexFromPalette(RGB_color needle, BSDM_PALETTE * palette)
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

void ditheringColor(uint8_t * src, uint8_t * dst, const BITMAPINFOHEADER & infoHeader, BSDM_PALETTE * colorTable)
{
  RGB_color currentPixel;
  RGB_color choosenColor;
  RGB_color * srcColor = (RGB_color *) src;
  RGB_color * dstColor = (RGB_color *) dst;
 
  // Storage for errors in color conversion
  float **errorsR = new float *[infoHeader.biWidth + 2];
  float **errorsG = new float *[infoHeader.biWidth + 2];
  float **errorsB = new float *[infoHeader.biWidth + 2];
  for (int i = 0; i < infoHeader.biWidth + 2; ++i) {
    errorsR[i] = new float[infoHeader.biHeight + 2]{0};
    errorsG[i] = new float[infoHeader.biHeight + 2]{0};
    errorsB[i] = new float[infoHeader.biHeight + 2]{0};
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

template<typename T>
bool readHelper (FILE * file , T * data)
{
    bool isRead = fread(data, sizeof(*data), 1, file);

    if (isRead)
    {
        return true;
    }
    return false;
}
int readBMPHeaders (FILE * bmpFile, BITMAPFILEHEADER & fileHeader, BITMAPINFOHEADER & infoHeader)
{
    if (!(readHelper(bmpFile,&fileHeader.bfType) &&
          readHelper(bmpFile, &fileHeader.bfSize)&&
          readHelper(bmpFile, &fileHeader.reserved) &&
          readHelper(bmpFile, &fileHeader.bfOffBits)))
    {
        std::cout << "[!] Failed to parse fileHeader" << std::endl;
        return 1;
    }
    
    if (!(readHelper(bmpFile,&infoHeader.biSize) &&
          readHelper(bmpFile, &infoHeader.biWidth)&&
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
    return 0;
}
bool readBSDMHeader (FILE * bsdmFile, BSDMHEADER & header)
{
    if (!(readHelper(bsdmFile, &header.magic) &&
          readHelper(bsdmFile, &header.width)&&
          readHelper(bsdmFile, &header.height) &&
          readHelper(bsdmFile, &header.bitsPerPixel) &&
          readHelper(bsdmFile, &header.mode) &&
          readHelper(bsdmFile, &header.isCustomPalette) &&
          readHelper(bsdmFile, &header.sizeOfHeader)))
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
uint8_t * readRawBMPData (FILE * bmpFile, const BITMAPFILEHEADER & fileHeader, const BITMAPINFOHEADER & infoHeader)
{
    int pitch = (infoHeader.biWidth*3 + 3) & ~3U;

    fseek(bmpFile,fileHeader.bfOffBits,0);
    uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight *3;

    uint8_t * data = new uint8_t [dataSizeWithoutPitches];    

    uint8_t * line = new uint8_t [pitch];
    uint8_t tmp;
    int offset = dataSizeWithoutPitches - infoHeader.biWidth * 3;
    for (int i = 0; i < infoHeader.biHeight; i++)
    {
        fread(line, 1, pitch, bmpFile);
        for (int j = 0; j < infoHeader.biWidth*3; j += 3)
        {
            //BGR to RGB
            tmp = line[j];
            line[j] = line[j+2];
            line[j+2] = tmp;
        }

        memcpy (data+offset,line,infoHeader.biWidth*3);   
        offset -= infoHeader.biWidth * 3;
    }
    return data;
}
uint8_t * readRawBSDMData (FILE * bsdmFile, const BSDMHEADER & header)
{
    fseek(bsdmFile,header.sizeOfHeader,0);
    uint32_t rawDataSize = header.width * header.height;
    uint8_t * data = new uint8_t [rawDataSize];    
    fread(data, 1, rawDataSize, bsdmFile);
    return data;
}
void grayscaleIn5bits(const uint8_t* src, uint8_t* dst, const BITMAPINFOHEADER& infoHeader) 
{
    uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight * 3;
    for (int i = 0; i < dataSizeWithoutPitches; i+=3) 
    {
        uint8_t BW;
        uint8_t r = src[i];
        uint8_t g = src[i + 1];
        uint8_t b = src[i + 2];
        BW = 0.299 * r + 0.587 * g + 0.114 * b;
        dst[i/3] = (BW>>3);
    }
}
void transcodePixels5bits (const uint8_t * src, uint8_t * dst, const BITMAPINFOHEADER & infoHeader)
{
    // direction == 0 --> BMP to BSDM
    // direction == 1 --> BSDM to BMP
    
    uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight *3;
    // assuming BMP is 24 bpp
    // blue is least important color so using one bit less for this color
    for (int i = 0; i < dataSizeWithoutPitches; i+=3)
    {
        uint8_t r = src[i];
        uint8_t g = src[i+1];
        uint8_t b = src[i+2];

        r = r >> 6;
        g = g >> 6;
        b = b >> 7;
        dst[i/3] = r << 3;
        dst[i/3] = dst[i/3] | (g << 1);
        dst[i/3] = dst[i/3] | b;
    }   
}
void transcodePixels5bitsGrayscale (const uint8_t * src, uint8_t * dst, const BITMAPINFOHEADER & infoHeader)
{    
    uint32_t dataSizeWithoutPitches = infoHeader.biWidth * infoHeader.biHeight *3;

    for (int i = 0; i < dataSizeWithoutPitches; i+=3)
    {
        uint8_t r = src[i];

        dst[i/3] = r >> 3;
    }   
}

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
    MCTriplet *data;
} MCCube;

static uint8_t dim; /* current cube biggest dimension */

MCTriplet MCTripletMake(uint8_t r, uint8_t g, uint8_t b)
{
    MCTriplet triplet;
    triplet.value[0] = r;
    triplet.value[1] = g;
    triplet.value[2] = b;

    return triplet;
}

void MCShrinkCube(MCCube *cube)
{
    uint8_t r, g, b;
    MCTriplet *data;

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

MCTriplet MCCubeAverage(MCCube *cube)
{
    return MCTripletMake(
    (cube->max.value[0] + cube->min.value[0]) / 2,
    (cube->max.value[1] + cube->min.value[1]) / 2,
    (cube->max.value[2] + cube->min.value[2]) / 2
    );
}

void MCCalculateBiggestDimension(MCCube *cube)
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

int MCCompareTriplet(const void *a, const void *b)
{
    MCTriplet *t1, *t2;

    t1 = (MCTriplet *)a;
    t2 = (MCTriplet *)b;

    return t1->value[dim] - t2->value[dim];
}

BSDM_PALETTE * MCQuantizeData(MCTriplet *data, uint32_t size, uint8_t level)
{
    BSDM_PALETTE * pall = (BSDM_PALETTE *) malloc (sizeof (BSDM_PALETTE));
    int p_size; /* generated palette size */
    MCCube *cubes;
    MCTriplet *palette;

    p_size  = pow(2, level);
    pall->numberOfColors = p_size;
    cubes   = (MCCube *)     malloc(sizeof(MCCube) * p_size);
    palette = (MCTriplet * ) malloc(sizeof(MCTriplet) * p_size);

    /* first cube */
    cubes[0].data = data;
    cubes[0].size = size;
    MCShrinkCube(cubes);

    /* remaining cubes */
    int parentIndex = 0;
    int iLevel = 1; /* iteration level */
    int offset;
    int median;
    MCCube *parentCube;
    while (iLevel <= level)
    {
        parentCube = &cubes[parentIndex];

        MCCalculateBiggestDimension(parentCube);
        qsort(parentCube->data, parentCube->size, sizeof(MCTriplet), MCCompareTriplet);

        median = parentCube->data[parentCube->size/2].value[dim];

        offset = p_size / pow(2, iLevel);

         /* split cubes */
        cubes[parentIndex+offset] = *parentCube;
        cubes[parentIndex   ].max.value[dim] = median;
        cubes[parentIndex+offset].min.value[dim] = median+1;

        /* find new cube data sizes */
        uint32_t newSize = 0;
        while (parentCube->data[newSize].value[dim] <= median)
        {
            newSize++;
        }
        /* newSize is now the index of the first element above the
        * median, thus it is also the count of elements below the median */
        cubes[parentIndex   ].size = newSize;
        cubes[parentIndex+offset].data += newSize;
        cubes[parentIndex+offset].size -= newSize;

        /* shrink new cubes */
        MCShrinkCube(&cubes[parentIndex]);
        MCShrinkCube(&cubes[parentIndex+offset]);

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

    pall->colors = (RGB_color *) palette;

    free(cubes);

    return pall;
}

void saveBSDMPalette (FILE * fBSDM, BSDM_PALETTE * palette)
{
    uint8_t colorsPaletteNumber = palette->numberOfColors;
    fwrite(&(palette->numberOfColors),1,1,fBSDM); // write number of palette colors
    for (int i = 0 ; i < palette->numberOfColors*3; i+=3)
    {
        fwrite(palette->colors+i/3,3,1,fBSDM);
    }
}

void readBSDMPalette (FILE * fBSDM, BSDM_PALETTE & palette)
{
    uint8_t nColors;
    fread(&nColors,1,1,fBSDM);
    palette.numberOfColors = nColors;
    uint32_t size = sizeof (RGB_color) * nColors;
    RGB_color * colors = (RGB_color *) malloc (size);
    fread(colors,size,1,fBSDM);
    palette.colors = colors;    
}

void convertSaveBMPPalette (FILE * fBSDM, FILE * fBMP, BSDM_PALETTE & palette, const BSDMHEADER & header)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    memset (&fileHeader,0,sizeof(fileHeader));
    memset (&infoHeader,0,sizeof(infoHeader));

    fileHeader.bfType = 0x4d42;
    fileHeader.bfSize = 0; // not important
    fileHeader.bfOffBits = 0x36;

    fwrite (&fileHeader,sizeof(fileHeader),1,fBMP);

    infoHeader.biSize = 0x28;
    infoHeader.biWidth = header.width;
    infoHeader.biHeight = -header.height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 0x18;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = header.height * ((header.width + 3) & ~3U) * (infoHeader.biBitCount / 8);
    infoHeader.biXPelsPerMeter = 1;
    infoHeader.biYPelsPerMeter = 1;

    fwrite (&infoHeader,sizeof(infoHeader),1,fBMP);

    uint32_t pitch = (infoHeader.biWidth*3 + 3) & ~3U;

    uint32_t bmpRawDataSize = pitch * header.height;

    uint32_t size = header.width * header.height;
    uint8_t * colorsData = new uint8_t [size];

    if (fread(colorsData,size,1,fBSDM) == 0)
    {
        std::cout << "[!] Cannot read colors" << std::endl;
        return;
    }
    uint32_t pitchDiff = pitch - infoHeader.biWidth*3;
    uint32_t colorsOfffset = 0;
    std::cout << pitch << std::endl;
    for (int i = 0 ; i < size ; i++)
    {
        //std::cout << "colorsOfffset " << colorsOfffset << " i / 3 = " << i/3 << "    ((i/3) % (pitch/3)) = " << ((i/3) % (pitch/3)) <<std::endl;
        uint8_t nColor = colorsData[i];
        fwrite (&palette.colors[nColor].b,1,1,fBMP);
        fwrite (&palette.colors[nColor].g,1,1,fBMP);
        fwrite (&palette.colors[nColor].r,1,1,fBMP);
        if ( (i % infoHeader.biWidth) + 1 == infoHeader.biWidth && pitchDiff)
        {
            uint8_t c = 0xCD;
            fwrite (&c,pitchDiff,1,fBMP);
        }
    }
}

void saveColorNumbers (FILE * fBSDM, BSDM_PALETTE * palette, RGB_color * rawBMPBitmapData, const BITMAPINFOHEADER & infoHeader)
{
    RGB_color * pixel;
    for (int i = 0; i < infoHeader.biHeight; i++) 
    {
        for (int j = 0; j < infoHeader.biWidth; j++) 
        {
            pixel = &rawBMPBitmapData[i*infoHeader.biWidth + j];
            uint8_t idx = findClosestColorIndexFromPalette(*pixel, palette);
            fwrite(&idx,1,1,fBSDM);
        }
    }
}
uint8_t * fiveBitToRGB (uint8_t * src, const BITMAPINFOHEADER & infoHeader)
{
    RGB_color * data = new RGB_color [infoHeader.biWidth * infoHeader.biHeight];
    for (int i = 0 ; i < infoHeader.biHeight * infoHeader.biWidth; i++)
    {
        data[i].r = src[i] * (255/31);
        data[i].g = src[i] * (255/31);
        data[i].b = src[i] * (255/31);
    }

    return (uint8_t *) data;
}
void convertSaveBSDM (FILE * fBSDM, const BITMAPFILEHEADER & fileHeader, const BITMAPINFOHEADER & infoHeader, uint8_t * rawBMPBitmapData, bool mode, bool isDedicatedPaletteUsed, bool dithering)
{
    BSDMHEADER bsdm_header;
    memcpy(&bsdm_header,"BSDM",4);
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
    bsdm_header.sizeOfHeader = sizeof (BSDMHEADER);

    fwrite (&bsdm_header,1,sizeof(BSDMHEADER),fBSDM);

    uint8_t * rawBSDMBitmapData = new uint8_t [bsdm_header.width * bsdm_header.height]; 
    uint8_t * rawDitheringData = new uint8_t [bsdm_header.width * bsdm_header.height * 3]; 

    if (bsdm_header.isCustomPalette)
    {  
        uint32_t s = bsdm_header.width * bsdm_header.height * 3;
        uint8_t * rawBMPBitmapDataQuantinize = (uint8_t *) malloc (s);
        memcpy (rawBMPBitmapDataQuantinize,rawBMPBitmapData,s);

        BSDM_PALETTE * palette = MCQuantizeData((MCTriplet *)rawBMPBitmapDataQuantinize,bsdm_header.width * bsdm_header.height,5);
        if (dithering)
        { 
            // WIN
            std::cout << "Paleta custom with dithering" << std::endl;
            ditheringColor(rawBMPBitmapData,rawBSDMBitmapData,infoHeader,palette);
            saveBSDMPalette(fBSDM,palette);
            saveColorNumbers (fBSDM,palette,(RGB_color *)rawBSDMBitmapData,infoHeader);
        }
        else 
        {
            // wIN
            std::cout << "Paleta custom only\n";
            saveBSDMPalette(fBSDM,palette);
            saveColorNumbers (fBSDM,palette,(RGB_color *)rawBMPBitmapData,infoHeader);
        }
        free (rawBMPBitmapDataQuantinize);
    }
    else 
    {  
        if (dithering)
        {
            if (mode)
            {
                std::cout << "Dithering with grayscale on default palette" << std::endl;
                grayscaleIn5bits(rawBMPBitmapData,rawBSDMBitmapData,infoHeader);
                uint8_t * data = fiveBitToRGB (rawBSDMBitmapData,infoHeader);
                ditheringColor(data,rawDitheringData,infoHeader,&grayPalette);
                transcodePixels5bitsGrayscale(rawDitheringData,rawBSDMBitmapData,infoHeader);
            }
            else
            {
                //WIN
                std::cout << "Dithering only on default palette" << std::endl;
                ditheringColor(rawBMPBitmapData,rawDitheringData,infoHeader,&defaultPalette);
                transcodePixels5bits(rawDitheringData,rawBSDMBitmapData,infoHeader);
            }
        }
        else
        {
            // WIN
            std::cout << "Grayscale only on default palette" << std::endl;
            grayscaleIn5bits(rawBMPBitmapData,rawBSDMBitmapData,infoHeader);
            
        }
        fwrite(rawBSDMBitmapData,1,infoHeader.biWidth * infoHeader.biHeight,fBSDM);
    }

    fclose(fBSDM);

}
void convertSaveBMP (FILE * fBMP, uint8_t * rawBSDMBitmapData, const BSDMHEADER & bsdmHeader, bool grayscale, bool dithering)
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    memset (&fileHeader,0,sizeof(fileHeader));
    memset (&infoHeader,0,sizeof(infoHeader));

    fileHeader.bfType = 0x4d42;
    fileHeader.bfSize = 0; // not important
    fileHeader.bfOffBits = 0x36;

    fwrite (&fileHeader,sizeof(fileHeader),1,fBMP);

    infoHeader.biSize = 0x28;
    infoHeader.biWidth = bsdmHeader.width;
    infoHeader.biHeight = -bsdmHeader.height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 0x18;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = bsdmHeader.height * ((bsdmHeader.width + 3) & ~3U) * (infoHeader.biBitCount / 8);
    infoHeader.biXPelsPerMeter = 1;
    infoHeader.biYPelsPerMeter = 1;

    fwrite (&infoHeader,sizeof(infoHeader),1,fBMP);

    uint32_t rawDataSize = bsdmHeader.width * bsdmHeader.height;
    uint8_t * rawBitmapData = new uint8_t [infoHeader.biSizeImage];

    uint32_t pitch = (infoHeader.biWidth*3 + 3) & ~3U;
    uint32_t widthPitchDiff = (pitch - bsdmHeader.width) * 3;

    if (grayscale)
    {
        for (int i = 0; i < std::abs(infoHeader.biHeight); i++) // negative height
        {
            for (int j = 0; j < pitch; j+=3)
            {
                rawBitmapData[ i * pitch + j    ] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] << 3 ) : 0xCD;  // BW

                rawBitmapData[ i * pitch + j + 1] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] << 3 )  : 0xCD;  // BW
    
                rawBitmapData[ i * pitch + j + 2] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] << 3)  : 0xCD; // BW
            }
        }
    }
    else
    {
        for (int i = 0; i < std::abs(infoHeader.biHeight); i++) // negative height
        {
            for (int j = 0; j < pitch; j+=3)
            {
                rawBitmapData[ i * pitch + j    ] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] & 0x1)      << 7  : 0xCD;  // B
    
               rawBitmapData[ i * pitch + j + 1] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] & 0x6)  << 5  : 0xCD;  // G
    
               rawBitmapData[ i * pitch + j + 2] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] & 0x18) << 3  : 0xCD; // R
            } 
        }
    }
    fwrite (rawBitmapData,infoHeader.biSizeImage,1,fBMP);
}


int main(int argc, const char * argv[])
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
        if (!strcmp (argv[i],"--custom-palette"))
        {
            customPalette = true;
        }
        if (!strcmp (argv[i],"--dithering"))
        {
            dithering = true;
        }

        if (!strcmp (argv[i],"--grayscale"))
        {
            grayscale = true;
        }

    }

    FILE * in = fopen(argv[1], "rb");
    FILE * out = fopen (argv[2], "wb");

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

    uint8_t * rawBSDMBitmapData;
    char magic [4];

    if (fread (magic,1,4,in) != 4)
    {
        return -1;
    }
    fseek (in,0,0); // set to begining 
    if ( !strncmp(magic,"BM",2)) // BMP to BSDM
    {
        BITMAPFILEHEADER BMPfileHeaderIN;
        BITMAPINFOHEADER BMPinfoHeaderIN;

        if (readBMPHeaders(in,BMPfileHeaderIN,BMPinfoHeaderIN))
        {
            return 1;
        }
        uint8_t * rawBMPBitmapData = readRawBMPData (in,BMPfileHeaderIN,BMPinfoHeaderIN);
        convertSaveBSDM (out,BMPfileHeaderIN,BMPinfoHeaderIN,rawBMPBitmapData,grayscale,customPalette,dithering);
    }
    else if ( !strncmp(magic,"BSDM",4) ) // BSDM to BMP, 24 bit RGB Windows bitmap
    {
        BSDMHEADER BSDMheaderIN;

        if (readBSDMHeader(in,BSDMheaderIN))
        {
            return 1;
        }

        if (BSDMheaderIN.isCustomPalette)
        {
            BSDM_PALETTE palette;
            readBSDMPalette(in,palette);
            convertSaveBMPPalette (in,out,palette,BSDMheaderIN);
        }
        else 
        {
            uint8_t * rawBSDMBitmapData = readRawBSDMData (in,BSDMheaderIN);
            //lzw_decompress // <--------- dekompresja surowych danych bitmapy BSDM 
            convertSaveBMP (out,rawBSDMBitmapData,BSDMheaderIN,grayscale,dithering);
        }
    }
    else 
    {
        std::cout << "[!] Unknown file format !" << std::endl;
    }

    fclose (in);
    fclose (out);
    
    return 0;
}
