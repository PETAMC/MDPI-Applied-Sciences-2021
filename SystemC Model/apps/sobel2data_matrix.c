#ifndef SOBEL2DATA_MATRIX
#define SOBEL2DATA_MATRIX
#include <stdint.h>

const uint8_t ConvMatrixDim  = 3;
const uint8_t ConvMatrixSize = 9;
// Matrix sources: https://en.wikipedia.org/wiki/Sobel_operator
const  int8_t ConvMatrix_X[81] = {
            1,  0, -1,
            2,  0, -2,
            1,  0, -1
        };
const  int8_t ConvMatrix_Y[81] = {
             1,  2,  1,
             0,  0,  0,
            -1, -2, -1
        };

#endif

