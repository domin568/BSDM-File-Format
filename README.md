# BSDM file format project
BSDM is image file format that uses LZW coding to compress image data.
Format uses only 32 colors.
- Dedicated 32 colors 24 bit (generated using Median Cut)
- Default 32 colors, 2 bits for R, 2 bits for G, 1 bit for B

## Features
convertBSDM program can convert from BMP or PNG to BSDM format with some optional changes
- Dithering
- Custom palette
- Grayscale

Best results when using custom palette with dithering

## Manual instalation

Dependency is libpng provided in this git repository, install it first
```
$ make
$ mv bsdmConvert ~/bin
``` 

## Usage 
```
$ ./bsdmConvert <in> <out> <--to-bmp>/<--to-png> [--custom-palette] [--dithering] [--grayscale]
```
or use special testing script which process all images from given directory
```
$ ./test.sh directory <--to-bmp/--to-png>
```
## TODO

- [x] Add PNG 24 bit support
- [ ] Add JPEG support 
