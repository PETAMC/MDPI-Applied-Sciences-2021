/*
 * sobel2.h
 *
 *  Created on: 02.03.2020
 *      Author: Ralf Stemmer
 */

#ifndef SOBEL2_H
#define SOBEL2_H

#include <sdf.h>
#include <stdint.h>

// Definition of the matrices used for the Sobel algorithm
extern const uint8_t ConvMatrixDim;     // 3,  9
extern const uint8_t ConvMatrixSize;    // 9, 81
extern const  int8_t ConvMatrix_X[];
extern const  int8_t ConvMatrix_Y[];
// Definition of the input image
extern const uint8_t ImageHeight;   // \_ Should never be more than 250 to have
extern const uint8_t ImageWidth;    // /  some safety margin against overflows
extern const uint8_t Image[];       // -- Pointer to Image[ImageHeight · ImageWidth]; 1 Pixel = 1 Byte (0 ... 255)

// The following arrays without explicit sized
// have an implicit size of ConvMatrixSize
void GetPixel(
        token_t position[2],    /* in/out ([x,y])*/
        token_t tokensOut[]     /* out */
        );

void GX(
        token_t tokensIn[],     /* in  */
        token_t tokensOut[1]    /* out */
        );

void GY(
        token_t tokensIn[],     /* in  */
        token_t tokensOut[1]    /* out */
        );

token_t ABS(
        token_t tokensIn1[1],   /* in */
        token_t tokensIn2[1]    /* in */
        );

#endif

