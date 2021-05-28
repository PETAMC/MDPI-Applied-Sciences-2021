#!/usr/bin/env bash

HEADER="-I."

clang -DxDEBUG -DVERSION="\"1.1.0\"" -Wno-multichar --std=gnu99 $HEADER -O2 -g -o jpegtranslator main.c

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

