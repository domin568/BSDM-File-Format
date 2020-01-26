#!/bin/sh
./bmp $1 outDEFAULT.bsdm && \
./bmp $1 outD.bsdm --dithering && \
./bmp $1 outDG.bsdm --dithering --grayscale && \
./bmp $1 outC.bsdm --custom-palette && \
./bmp $1 outCD.bsdm --custom-palette --dithering && \
./bmp $1 outG.bsdm --grayscale && \
./bmp outDEFAULT.bsdm outDEFAULT.bmp && \
./bmp outD.bsdm outD.bmp && \
./bmp outDG.bsdm outDG.bmp  && \
./bmp outC.bsdm outC.bmp && \
./bmp outCD.bsdm outCD.bmp && \
./bmp outG.bsdm outG.bmp
echo "[*] Success !"
