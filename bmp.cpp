//  Created by Dominik Tamiołło on 19.03.2016.

#include <iostream>
#include <fstream>
#include <math.h>
#include <map>
#include <vector>
#include <inttypes.h>
#include <string>
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

template <int size>
struct COLORPALETTE
{
    int colors [size];
    
}__attribute__((packed));

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

template <typename T>
struct bsdm_palette_entry
{

};

// TODOOOOOOOOOOOOOOOOOOOOOOOOO

std::vector<int> lzw_compression (uint8_t **data, uint32_t width,uint32_t height)
{
    std::map <std::string,int> lzwDic;
   int dicSize = 32;

    // 5 bits 

   for (int i = 0; i < 32; i++)
    {
        lzwDic[std::string(1,i)] = i;
    }
    
    std::string toCompress = "";
    for(int i=0;i<width;i++){
        for(int j=0;j<height;j++){

            toCompress += data[i][j];

        }
    }

    std::string prev;
    char curr;
    std::string pc;
    std::vector<int> encodedVal; 

    for(int i=0;i<toCompress.length();i++){

        curr = toCompress[i];
        pc = prev + curr;
        if(lzwDic.count(pc))
        prev = pc;
        else {

            encodedVal.push_back(lzwDic[prev]);
            lzwDic[pc] = dicSize++;
            prev = std::string(1,curr);

        }
    }

    if(!prev.empty())
    encodedVal.push_back(lzwDic[prev]);

    return encodedVal;
}

std::string lzw_decompress(std::vector<int> encoded){

    std::map<int,std::string> lzwDic;

    int dicSize = 32;

    for(int i=0;i<32;i++){
        lzwDic[i] = std::string(1,i);
    }

    std::string prev(1,*encoded.begin());
    std::string decompressed = prev;
    std::string in;
    int curr;

    for(int i=0;i<encoded.size();i++){
        curr = encoded[i];

        if(lzwDic.count(curr))
        in = lzwDic[curr];
        else if(curr == dicSize){
            in = prev + prev[0];
        }


    decompressed += in;
    lzwDic[dicSize++] = prev + in[0];


    prev = in;
    }

    return decompressed;
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
    uint32_t rawDataSize = pitch * infoHeader.biHeight;
    uint8_t * data = new uint8_t [rawDataSize];    

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
void transcodePixels5bits (const uint8_t * src, uint8_t * dst, bool direction, const BITMAPINFOHEADER & infoHeader)
{
    // direction == 0 --> BMP to BSDM
    // direction == 1 --> BSDM to BMP
    
    if (direction == 0)
    {
        // assuming BMP is 24 bpp
        // blue is least important color so using one bit less for this color
        for (int i = 0; i < infoHeader.biSizeImage; i+=3)
        {
            uint8_t r = src[i];
            uint8_t g = src[i+1];
            uint8_t b = src[i+2];

            // 1
            {
                r = r >> 6;
                g = g >> 6;
                b = b >> 7;
                dst[i/3] = r << 3;
                dst[i/3] = dst[i/3] | (g << 1);
                dst[i/3] = dst[i/3] | b;
            }
        }
        
    }
    else if (direction == 1)
    {

    }
}
void convertSaveBSDM (FILE * fBSDM, const BITMAPFILEHEADER & fileHeader, const BITMAPINFOHEADER & infoHeader, uint8_t * rawBMPBitmapData, int mode, bool isDedicatedPaletteUsed)
{
    BSDMHEADER bsdm_header;
    memcpy(&bsdm_header,"BSDM",4);
    bsdm_header.width = infoHeader.biWidth;
    bsdm_header.height = infoHeader.biHeight;
    bsdm_header.bitsPerPixel = 5;
    bsdm_header.mode = mode;
    bsdm_header.isCustomPalette = isDedicatedPaletteUsed;
    bsdm_header.sizeOfHeader = sizeof (BSDMHEADER);

    fwrite (&bsdm_header,1,sizeof(BSDMHEADER),fBSDM);

    uint8_t * rawBSDMBitmapData = new uint8_t [infoHeader.biSizeImage/3];

    if (bsdm_header.isCustomPalette)
    {

    }
    else 
    {
        transcodePixels5bits (rawBMPBitmapData,rawBSDMBitmapData,0, infoHeader);
        fwrite(rawBSDMBitmapData,1,infoHeader.biSizeImage/3,fBSDM);
    }

    fclose(fBSDM);

}
void convertSaveBMP (FILE * fBMP, uint8_t * rawBSDMBitmapData, const BSDMHEADER & bsdmHeader, bool isDedicatedPaletteUsed)
{
    if (isDedicatedPaletteUsed)
    {

    }
    else 
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

        for (int i = 0; i < std::abs(infoHeader.biHeight); i++)
        {
            for (int j = 0; j < pitch; j+=3)
            {
                rawBitmapData[ i * pitch + j    ] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] & 0x1)      << 7  : 0xCD;  // B
                //asm (".byte 0xcc");
                rawBitmapData[ i * pitch + j + 1] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] & 0x6)  << 5  : 0xCD;  // G
                //asm (".byte 0xcc");
                rawBitmapData[ i * pitch + j + 2] = j < infoHeader.biWidth * 3 ? \
                (rawBSDMBitmapData[ i * bsdmHeader.width + j / 3] & 0x18) << 3  : 0xCD; // R
            } 
        }
        fwrite (rawBitmapData,infoHeader.biSizeImage,1,fBMP);
        //asm (".byte 0xcc");
        std::cout << "AAAA" << std::endl;   
    }
}

int main(int argc, const char * argv[])
{
    if (argc < 3)
    {
        std::cout << "[!] Usage: ./bsdm <in> <out> " << std::endl;
    }

    FILE * in = fopen(argv[1], "rb");
    FILE * out = fopen (argv[2], "wb");

     // allocating memory for these in specific functions
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

        convertSaveBSDM (out,BMPfileHeaderIN,BMPinfoHeaderIN,rawBMPBitmapData,0,false);
    }
    else if ( !strncmp(magic,"BSDM",4) ) // BSDM to BMP, 24 bit RGB Windows bitmap
    {
        // TODO PALETTE
        BSDMHEADER BSDMheaderIN;
        if (readBSDMHeader(in,BSDMheaderIN))
        {
            return 1;
        }
        uint8_t * rawBSDMBitmapData = readRawBSDMData (in,BSDMheaderIN);
        convertSaveBMP (out,rawBSDMBitmapData,BSDMheaderIN,false);
    }

    fclose (in);
    fclose (out);
    
    return 0;
}
