#!/bin/sh

if [ "$#" -ne 2 ]; then
    echo "[!] ./test.sh <folder path with images> [bmp/png] <--to-bmp/--to-png>"
    exit
fi
if [ "$2" == "--to-png" ]; then
	ext="png"
fi
if [ "$2" == "--to-bmp" ]; then
	ext="bmp"
fi

if [[ ! -d $1 ]]; then
    echo "[!] $1 is not a directory !"
    exit
fi
mkdir -p img/bsdm
mkdir -p img/pngbmp
for f in $1/*
do
	IFS='.' read -r -a arr <<< "$f"
	IFS='/' read -r -a arr <<< "$arr"
	fileName="${arr[1]}"
	echo "[*] Processing $f\n"
	./bsdmConvert $f 'img/bsdm/'${fileName}'DEFAULT.bsdm' && \
	./bsdmConvert $f 'img/bsdm/'${fileName}'D.bsdm' --dithering && \
	./bsdmConvert $f 'img/bsdm/'${fileName}'DG.bsdm' --dithering --grayscale && \
	./bsdmConvert $f 'img/bsdm/'${fileName}'C.bsdm' --custom-palette && \
	./bsdmConvert $f 'img/bsdm/'${fileName}'CD.bsdm' --custom-palette --dithering && \
	./bsdmConvert $f 'img/bsdm/'${fileName}'G.bsdm' --grayscale && 
	./bsdmConvert 'img/bsdm/'${fileName}'DEFAULT.bsdm' "img/pngbmp/${fileName}DEFAULT.${ext}" $2 && \
	./bsdmConvert 'img/bsdm/'${fileName}'D.bsdm' "img/pngbmp/${fileName}D.${ext}" $2 && \
	./bsdmConvert 'img/bsdm/'${fileName}'DG.bsdm' "img/pngbmp/${fileName}DG.${ext}" $2 && \
	./bsdmConvert 'img/bsdm/'${fileName}'C.bsdm' "img/pngbmp/${fileName}C.${ext}" $2 && \
	./bsdmConvert 'img/bsdm/'${fileName}'CD.bsdm' "img/pngbmp/${fileName}CD.${ext}" $2 && \
	./bsdmConvert 'img/bsdm/'${fileName}'G.bsdm' "img/pngbmp/${fileName}G.${ext}" $2
done

if [ $? -eq 0 ]; then
    echo "[*] Success"
else
    echo "[!] Error !"
fi

