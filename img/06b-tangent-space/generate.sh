#!/bin/sh
for i in `ls -1 f*tex`; do
    rubber $i
    dvisvgm -e --no-fonts --scale=1.5 `basename $i tex`dvi
    rubber --clean $i
done

for i in `ls -1 *svg`; do
    convert -density 100 $i `basename $i svg`png
done

