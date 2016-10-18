#!/bin/sh
for i in `ls -1 f*tex`; do
    rubber $i
    dvisvgm --no-fonts --scale=1.5 `basename $i tex`dvi
    convert -density 85 `basename $i tex`svg `basename $i tex`png
    rubber --clean $i
done



