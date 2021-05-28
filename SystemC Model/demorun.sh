#!/usr/bin/env bash

Iterations=256  # 128⨯128 pixels JPEG / 8⨯8 pixels per MCU -> 256 MCUs
Experiment="JPEG-functional"

./model --experiment $Experiment --iterations $Iterations

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

