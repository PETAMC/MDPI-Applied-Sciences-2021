#include "decoding.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "image.h"


// Pixel: 0x00BBGGRR
void CreateRGBPixels(
        token_t Y[8*8]      /*in*/, 
        token_t Cr[8*8]     /*in*/, 
        token_t Cb[8*8]     /*in*/,
        token_t Pixels[8*8] /*out*/
        )
{
    int16_t R, G, B;    // While conversion, negative values and 9bit values are possible
    for(uint8_t i=0; i<8*8; i++)
    {
        // Source: https://en.wikipedia.org/wiki/YCbCr
        R = Y[i]                             + 1.402f   * (Cr[i]-128.0f);
        G = Y[i] - 0.34414f * (Cb[i]-128.0f) - 0.71414f * (Cr[i]-128.0f);
        B = Y[i] + 1.772f   * (Cb[i]-128.0f);

        // Check boundaries
        if(R < 0)
            R = 0;
        else if(R > 255)
            R = 255;
        if(G < 0)
            G = 0;
        else if(G > 255)
            G = 255;
        if(B < 0)
            B = 0;
        else if(B > 255)
            B = 255;

        // Create Pixel
        Pixels[i] = (token_t)B << 16 | G << 8 | R;
    }
}

void ColorConversion(
        token_t Y[8*8]      /*in*/, 
        token_t Cr[8*8]     /*in*/, 
        token_t Cb[8*8]     /*in*/,
        token_t R[8*8]      /*out*/, 
        token_t G[8*8]      /*out*/, 
        token_t B[8*8]      /*out*/
        )
{

    for(int i=0; i<8*8; i++)
    {
        // Source: https://en.wikipedia.org/wiki/YCbCr
        R[i] = Y[i]                             + 1.402f   * (Cr[i]-128.0f);
        G[i] = Y[i] - 0.34414f * (Cb[i]-128.0f) - 0.71414f * (Cr[i]-128.0f);
        B[i] = Y[i] + 1.772f   * (Cb[i]-128.0f);

        if(R[i] < 0)
            R[i] = 0;
        else if(R[i] > 255)
            R[i] = 255;
        if(G[i] < 0)
            G[i] = 0;
        else if(G[i] > 255)
            G[i] = 255;
        if(B[i] < 0)
            B[i] = 0;
        else if(B[i] > 255)
            B[i] = 255;
    }
}


void ConvertRGB24to16(
        token_t Pixels24[8*8] /*in*/, 
        token_t Pixels16[8*8] /*out*/
        )
{
    for(uint8_t i=0; i<8*8; i++)
    {
        int16_t r,g,b; // The conversion algorithm can temporarly create 9bit values

        // Separate R,G,B from pixel
        r = (Pixels24[i]      ) & 0x000000FF;
        g = (Pixels24[i] >>  8) & 0x000000FF;
        b = (Pixels24[i] >> 16) & 0x000000FF;

        // lazy rounding
        r += 0x04;
        g += 0x02;
        b += 0x04;
        if(r > 255) r = 255;
        if(g > 255) g = 255;
        if(b > 255) b = 255;

        // Reduce color bit width
        r = (r >> 3) & 0x1F; //RRRRRRRR >> 3 ->  RRRRR (5)
        g = (g >> 2) & 0x3F; //GGGGGGGG >> 2 -> GGGGGG (6)
        b = (b >> 3) & 0x1F; //BBBBBBBB >> 3 ->  BBBBB (5)

        Pixels16[i] = (r << 11) | (g << 5) | (b << 0);
    }
}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

