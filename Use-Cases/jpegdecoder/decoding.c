#include "decoding.h"
#include "image.h"
#include <stdint.h>
#include <stdbool.h>

uint8_t GetNextBitFromImage(bool reset)
{
    static int byteindex = 0;
    static int bitindex  = 0;

    if(reset)
    {
        byteindex = 0;
        bitindex  = 0;
        return 0xFF;
    }

    uint8_t bit;
    bit  = Image_data[byteindex] >> (7-bitindex);
    bit &= 0x01;    // be aware of arithmetic shifting

    if(bitindex >= 7)
    {
        bitindex = 0;
        byteindex++;
    }
    else
    {
        bitindex++;
    }

    return bit;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

