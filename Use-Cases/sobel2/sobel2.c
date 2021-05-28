/*
 * sobel2.c
 *
 *  Created on: 02.03.20
 *      Author: Ralf Stemmer
 */

#include <sobel2.h>

int image_x = 0;
int image_y = 0;


void GetPixel(token_t position[2], token_t tokensOut[])
{	
    int16_t pixel_x = (int16_t) position[0];
    int16_t pixel_y = (int16_t) position[1];

    // Get a chunk of pixels as input for GX and GY
    int i = 0;
    for(int8_t matrix_y = 0; matrix_y < ConvMatrixDim; matrix_y++)
    {
        for(int8_t matrix_x = 0; matrix_x < ConvMatrixDim; matrix_x++)
        {
            int16_t chunk_x;
            int16_t chunk_y;
            chunk_x = pixel_x + matrix_x - (ConvMatrixDim >> 1);
            chunk_y = pixel_y + matrix_y - (ConvMatrixDim >> 1);

            if(chunk_x < 0)
                chunk_x = 0;
            else if(chunk_x >= ImageWidth)
                chunk_x = ImageWidth - 1;

            if(chunk_y < 0)
                chunk_y = 0;
            else if(chunk_y >= ImageHeight)
                chunk_y = ImageHeight - 1;

            tokensOut[i] = Image[chunk_y * ImageWidth + chunk_x];
            i++;
        }
    }

    // Calculate position for the next pixel
#ifdef EXTENDEDALU
    pixel_x = (pixel_x + 1) % ImageWidth;
    if(pixel_x == 0)
        pixel_y = (pixel_y + 1) % ImageHeight;
#else
    pixel_x++;
    if(pixel_x >= ImageWidth)
    {
        pixel_x = 0;
        pixel_y++;
        if(pixel_y >= ImageHeight)
        {
            pixel_x = 0;
            pixel_y = 0;
        }
    }
#endif

    // Write next pixel position into the output tokens
    position[0] = pixel_x;
    position[1] = pixel_y;

    return;
}


void GX(token_t tokensIn[], token_t tokensOut[1])
{
    tokensOut[0] = 0;

    for(uint8_t i = 0; i < ConvMatrixSize; i++)
    {
        tokensOut[0] += tokensIn[i] * ConvMatrix_X[ConvMatrixSize-1 - i];
    }

    return;
}


void GY(token_t tokensIn[], token_t tokensOut[1])
{
    tokensOut[0] = 0;

    for(uint8_t i = 0; i < ConvMatrixSize; i++)
    {
        tokensOut[0] += tokensIn[i] * ConvMatrix_Y[ConvMatrixSize-1 - i];
    }

    return;
}


token_t ABS(token_t tokensIn1[1], token_t tokensIn2[1])
{
    if(tokensIn1[0] < 0) tokensIn1[0] = tokensIn1[0] * -1;
    if(tokensIn2[0] < 0) tokensIn2[0] = tokensIn2[0] * -1;

    token_t result;
    // Usually, the formula is sqrt(GX² + GY²), we hacked it this way:
    result = (tokensIn1[0] + tokensIn2[0]) & 0x000000FF;
    //result = (tokensIn1[0] + tokensIn2[0]) >> 2;
    return result;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
