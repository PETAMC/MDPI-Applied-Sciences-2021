#!/usr/bin/env bash

set -e

HEADER="-I./libhistogram"
LIBS="-lpthread $(pkg-config --libs cairo) $(pkg-config --libs freetype2) $(pkg-config --libs fontconfig) $(pkg-config --libs libconfig++) -L. -lhistogram"
CPP="clang++"
CPPFLAGS="-g3 -O3 -std=c++17 $(pkg-config --cflags cairo) $(pkg-config --cflags freetype2) $(pkg-config --cflags libconfig++)"

# build libhistogram
SOURCE=$(find ./libhistogram -type f -name "*.cpp")
for c in $SOURCE ;
do
    echo -e "\e[1;34mCompiling $c …\e[0m"
    $CPP $HEADER $CPPFLAGS -c -o "${c%.*}.o" $c
done

OBJECTS=$(find ./libhistogram -type f -name "*.o")
if [ -e ./libhistogram.a ] ; then
    rm ./libhistogram.a
fi
ar rv ./libhistogram.a $OBJECTS
rm $OBJECTS

# build tools
SOURCE=$(find ./src -type f -name "*.cpp")
for c in $SOURCE ;
do
    echo -e "\e[1;34mCompiling $c …\e[0m"
    $CPP $HEADER $CPPFLAGS -c -o "${c%.*}.o" $c
done

OBJECTS=$(find ./src -type f -name "*.o")

echo -e "\e[1;34mBuilding pdfplot …\e[0m"
$CPP -o pdfplot ./src/plot.o $LIBS
./pdfplot --version

echo -e "\e[1;34mBuilding pdfcompare …\e[0m"
$CPP -o pdfcompare ./src/compare.o $LIBS
./pdfcompare --version

rm $OBJECTS

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

