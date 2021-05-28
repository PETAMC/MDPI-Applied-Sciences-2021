#!/usr/bin/env bash

SOURCEPATH="../Use-Cases/jpegdecoder"
INCLUDEPATHS="-I$SOURCEPATH -I./"

SOURCES=$(find $SOURCEPATH -type f -name "*.c")
clang -fPIC -shared $INCLUDEPATHS $SOURCES -o ./apps/jpeg.so
clang -fPIC -shared -I./app ./apps/image.c -o ./apps/jpegdata.so

