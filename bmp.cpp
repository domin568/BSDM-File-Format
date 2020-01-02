//  Created by Dominik Tamiołło on 19.03.2016.

#include <iostream>
#include <fstream>
#include <math.h>
#include <map>
#include <vector>

struct BITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned int reserved;
    unsigned int bfOffBits;
    
}__attribute__((packed));

struct BITMAPINFOHEADER
{
    unsigned int bfSize;
    unsigned int biWidth;
    unsigned int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
    
}__attribute__((packed));

template <int size>
struct COLORPALETTE
{
    int colors [size];
    
}__attribute__((packed));


struct bsdm_header 
{
    unsigned char magic [4];
    uint32_t width;
    uint32_t height;
    uint8_t bitsPerPixel;
    uint8_t mode;
    uint32_t nColors;
    uint8_t isCustomPalette;
    uint32_t sizeOfHeader;
}__attribute__((packed));

template <typename T>
struct bsdm_palette_entry
{

};

uint8_t lzw_encode (uint8_t * data, uint32_t size)
{
    std::map <uint32_t,uint8_t *> m;
    for (int i = 0; i < 0xff; i++)
    {
        data[i] = (unsigned char)i;
    }
    uint8_t c = data[0];
    uint32_t pos = 1;
    while (pos < size)
    {
        uint8_t s = data[pos];
    }
    return 0;
}
struct color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    
}__attribute__((packed));

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
    
    if (!(readHelper(bmpFile,&infoHeader.bfSize) &&
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
void readRawBMPData (FILE * bmpFile, uint8_t * data, BITMAPFILEHEADER & fileHeader, BITMAPINFOHEADER & infoHeader)
{
    int pitch = (infoHeader.biWidth*3 + 3) & ~3U;

    fseek(bmpFile,fileHeader.bfOffBits,0);
    uint32_t rawDataSize = pitch * infoHeader.biHeight;

    uint8_t * line = new uint8_t [pitch];
    uint8_t tmp;
    int offset = rawDataSize - pitch;
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

        memcpy (data+offset,line,pitch);
        offset -= pitch;
    }
}
int main(int argc, const char * argv[])
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    FILE * fBMP = fopen(argv[1], "rb");
    uint8_t * rawBitmapData;
    
    if (readBMPHeaders(fBMP,fileHeader,infoHeader))
    {
        return 1;
    }

    readRawBMPData (fBMP,rawBitmapData,fileHeader,infoHeader);    



    
    asm(".byte 0xcc");
    

    // headers

    /*
    
    BITMAPFILEHEADER * fileHeader = (BITMAPFILEHEADER *)data;

    BITMAPINFOHEADER * infoHeader = (BITMAPINFOHEADER *)(data + sizeof(BITMAPFILEHEADER));
    
    COLORPALETTE <16> * palette = (COLORPALETTE <16> *) (data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
    memset(palette, 0, 16);
    
    palette->colors[0] = 0x0000ff00;
    palette->colors[1] = 0x00ee0000;
    palette->colors[2] = 0xff000000;
    
    uint8_t * bitmapData = (uint8_t*)(palette + 1) ;
    
    int pitch = (WIDTH + 3) & ~3;
    
    int diff = WIDTH - pitch;
    
    for (int j = 0 ; j < HEIGHT;j++)
    {
        for (int i = 0 ; i < pitch/2;i++)
        {
            if (i >= WIDTH/2)
            {
                bitmapData[i + j * 6] = (uint8_t)0;
            }
            else
            {
                bitmapData[i + j * 6] = (uint8_t)0x11;
            }
        }
    }
    memset(fileHeader, 0, sizeof(*fileHeader));
    
    fileHeader->bfType = 0x424d; // magic
    int sizeOfStructures = (sizeof(*fileHeader) + sizeof(*infoHeader));
    fileHeader->bfOffBits = sizeOfStructures;
    
    memset(infoHeader, 0, sizeof(*infoHeader));
    
    infoHeader->bfSize = 40 ;
    infoHeader->biWidth = WIDTH;
    infoHeader->biHeight = HEIGHT;
    infoHeader->biPlanes = 1;
    infoHeader->biBitCount = BITPERPIXEL;
    infoHeader->biCompression = 0;
    infoHeader->biSizeImage = pitch * HEIGHT;
    infoHeader->biXPelsPerMeter = 1;
    infoHeader->biYPelsPerMeter = 1;
    
    
    std::fstream file;
    
    file.open("file.bmp",std::ios::out);
    
    std::cout << "SIZE" << sizeOfStructures + sizeof(COLORPALETTE<16>) + pitch * HEIGHT /2 << std::endl;
    
    file.write((char *)data, sizeOfStructures + sizeof(COLORPALETTE<16>) + pitch * HEIGHT / 2 + 1);
    
    file.close();

    */
    
    return 0;
}
