#include "decoding.h"
#include <math.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "image.h"


#ifndef M_PI
#define M_PI 3.1415926f
#endif

void IDCT(token_t source[8*8], token_t destination[8*8])
{
    for(int dsty=0; dsty<8; dsty++)
    {
        for(int dstx=0; dstx<8; dstx++)
        {
            float sum;
            sum = 0.0f;
            for(int srcy=0; srcy<8; srcy++)
            {
                for(int srcx=0; srcx<8; srcx++)
                {
                    float cosx, cosy;
                    // Source: https://www.w3.org/Graphics/JPEG/itu-t81.pdf
                    cosx = cosf( ( M_PI * (2.0f * dstx + 1.0f) * srcx ) / 16.0f);
                    cosy = cosf( ( M_PI * (2.0f * dsty + 1.0f) * srcy ) / 16.0f);

                    float cx, cy;
                    if(srcx == 0)
                        cx = 1.0f / sqrtf(2.0f);
                    else
                        cx = 1.0f;

                    if(srcy == 0)
                        cy = 1.0f / sqrtf(2.0f);
                    else
                        cy = 1.0f;

                    sum += source[srcy*8+srcx] * cx * cy * cosx * cosy;
                }
            }
            
            // Level shift as told in F.1.1.3: https://www.w3.org/Graphics/JPEG/itu-t81.pdf
            destination[dsty*8+dstx] = roundf(0.25f * sum + 128.0f);


            // Seen at https://github.com/matja/jpegdecoder/blob/master/jpegdecoder.c#L272
            if(destination[dsty*8+dstx] < 0)
                destination[dsty*8+dstx] = 0;
            else if(destination[dsty*8+dstx] > 255)
                destination[dsty*8+dstx] = 255;
        }
    }

#ifdef DEBUG
    extern int dez2str(char* dst, long src);
    extern void Print(const char *string);
    Print("\e[1;34mSlow-Matrix:\e[0;36m\n");
    for(int y=0; y<8; y++)
    {
        Print("\t");
        for(int x=0; x<8; x++)
        {
            char data[20];
            dez2str(data, destination[y*8+x]);
            Print(data);
            Print("\t");
        }
        Print("\n");
    }
#endif
}


void FastIDCT(token_t source[8*8], token_t destination[8*8])
{
    // Source:
    //  Author: Robert Danielsen
    //  Modified: T.W., 11-Nov-1996, Adopted from Momusys VM
    //  Modified by Ralf Stemmer (ralf.stemmer@uol.de)
    //  Modifications:
    //      - Using float instead of double
    //      - Adding offset of 128 to the result
    //  Link: https://github.com/usamaaftab80/multi-p2p/blob/5c46a17192e6483df1e560337fc0a50bdd7cc0b2/vic264/codec/h263/idctenc.c#L312

    int   j1, i, j;
    float b[8], b1[8], dd[8][8];
    float f0 = 0.7071068f;
    float f1 = 0.4903926f;
    float f2 = 0.4619398f;
    float f3 = 0.4157348f;
    float f4 = 0.3535534f;
    float f5 = 0.2777851f;
    float f6 = 0.1913417f;
    float f7 = 0.0975452f;
    float e, f, g, h;

    /* Horizontal */

    /* Descan coefficients first */

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            b[j] = (float)source[8 * i + j];
        }
        e = b[1] * f7 - b[7] * f1;
        h = b[7] * f7 + b[1] * f1;
        f = b[5] * f3 - b[3] * f5;
        g = b[3] * f3 + b[5] * f5;

        b1[0] = (b[0] + b[4]) * f4;
        b1[1] = (b[0] - b[4]) * f4;
        b1[2] = b[2] * f6 - b[6] * f2;
        b1[3] = b[6] * f6 + b[2] * f2;
        b[4]  = e + f;
        b1[5] = e - f;
        b1[6] = h - g;
        b[7]  = h + g;

        b[5] = (b1[6] - b1[5]) * f0;
        b[6] = (b1[6] + b1[5]) * f0;
        b[0] = b1[0] + b1[3];
        b[1] = b1[1] + b1[2];
        b[2] = b1[1] - b1[2];
        b[3] = b1[0] - b1[3];

        for (j = 0; j < 4; j++)
        {
            j1 = 7 - j;
            dd[i][j] = b[j] + b[j1];
            dd[i][j1] = b[j] - b[j1];
        }
    }

        /* Vertical */

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            b[j] = dd[j][i];
        }
        e = b[1] * f7 - b[7] * f1;
        h = b[7] * f7 + b[1] * f1;
        f = b[5] * f3 - b[3] * f5;
        g = b[3] * f3 + b[5] * f5;

        b1[0] = (b[0] + b[4]) * f4;
        b1[1] = (b[0] - b[4]) * f4;
        b1[2] = b[2] * f6 - b[6] * f2;
        b1[3] = b[6] * f6 + b[2] * f2;
        b[4] = e + f;
        b1[5] = e - f;
        b1[6] = h - g;
        b[7] = h + g;

        b[5] = (b1[6] - b1[5]) * f0;
        b[6] = (b1[6] + b1[5]) * f0;
        b[0] =  b1[0] + b1[3];
        b[1] =  b1[1] + b1[2];
        b[2] =  b1[1] - b1[2];
        b[3] =  b1[0] - b1[3];

        for (j = 0; j < 4; j++)
        {
            j1 = 7 - j;
            dd[j][i]  = b[j] + b[j1];
            dd[j1][i] = b[j] - b[j1];
        }
    }

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
#define MNINT(a) ((a) < 0 ? (int)(a - 0.5f) : (int)(a + 0.5f))
            destination[i*8+j]  = (token_t)(MNINT(dd[i][j]) + 128.0f);
        }
    }
#ifdef DEBUG
    extern int dez2str(char* dst, long src);
    extern void Print(const char *string);
    Print("\e[1;34mFast-Matrix:\e[0;36m\n");
    for(int y=0; y<8; y++)
    {
        Print("\t");
        for(int x=0; x<8; x++)
        {
            char data[20];
            dez2str(data, destination[y*8+x]);
            Print(data);
            Print("\t");
        }
        Print("\n");
    }
#endif
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

