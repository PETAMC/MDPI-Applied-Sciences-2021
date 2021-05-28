#!/usr/bin/env bash


SOURCEPATH="../Use-Cases/sobel2"
INCLUDEPATHS="-I$SOURCEPATH -I./ -I$SOURCEPATH/../sdf"
CFLAGS="-fPIC -shared -g3"

SOURCES=$(find $SOURCEPATH -type f -name "*.c")
clang $CFLAGS $INCLUDEPATHS $SOURCES ./apps/sobel2data_matrix.c -o ./apps/sobel2.so
clang $CFLAGS ./apps/sobel2data_image.c          -o ./apps/sobel2data.so
clang $CFLAGS ./apps/sobel2data_image_black.c    -o ./apps/sobel2data_black.so
clang $CFLAGS ./apps/sobel2data_image_cross.c    -o ./apps/sobel2data_cross.so
clang $CFLAGS ./apps/sobel2data_image_gradient.c -o ./apps/sobel2data_gradient.so


