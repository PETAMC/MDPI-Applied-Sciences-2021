#include "decoding.h"

#include <stdbool.h>
#include <stdio.h>
#include "image.h"


void InverseQuantization(const uint8_t quantizationtable[8*8], token_t QuantizedCoefficients[8*8], token_t DCTCoefficients[8*8])
{
    const uint8_t ZigZagMap[8*8] = 
        {
               0,  1,  8, 16,  9,  2,  3, 10,
              17, 24, 32, 25, 18, 11,  4,  5,
              12, 19, 26, 33, 40, 48, 41, 34,
              27, 20, 13,  6,  7, 14, 21, 28,
              35, 42, 49, 56, 57, 50, 43, 36,
              29, 22, 15, 23, 30, 37, 44, 51,
              58, 59, 52, 45, 38, 31, 39, 46,
              53, 60, 61, 54, 47, 55, 62, 63
        };

    for(int i=0; i<8*8; i++)
    {
        token_t tmp;
        tmp = QuantizedCoefficients[i] * quantizationtable[i];

        // shall be 11 Bit signed values as specified in F.1.1.4: https://www.w3.org/Graphics/JPEG/itu-t81.pdf
        if(tmp < -1024)
            tmp = -1024;
        else if(tmp > 1023)
            tmp = 1023;

        DCTCoefficients[ZigZagMap[i]] = tmp;
    }

#ifdef DEBUG
    printf("\e[1;34mDCTCoefficients[%i]:\e[0;36m\n", component);
    for(int y=0; y<8; y++)
    {
        printf("\t");
        for(int x=0; x<8; x++)
        {
            printf("%d\t", DCTCoefficients[y*8+x]);
        }
        printf("\n");
    }
#endif
    return;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

