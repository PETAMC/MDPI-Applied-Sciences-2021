#!/usr/bin/env bash

set -e

INSTALLPATH=/usr/local/bin
install -m 755 -v -s -g root -o root ./pdfplot    $INSTALLPATH
install -m 755 -v -s -g root -o root ./pdfcompare $INSTALLPATH

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

