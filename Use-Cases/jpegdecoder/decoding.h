#ifndef JPEGDECODING
#define JPEGDECODING

/*
 * CHANGELOG
 * 24.05.19 - Interface of InverQuantization changed to avoid a configuration-depending branching behavior
 * 13.05.19 - BlockInfo parameter of GetEncodedImageBlock is optional and can be NULL
 *          - Detailed comments added to this file
 *          - Cleaning up header file
 */
#include <stdint.h>

typedef int token_t;    // TODO: Include sdf.h to get token_t definition
enum component_t {COMP_Y=0, COMP_Cb=1, COMP_Cr=2};
enum blockinfoindex_t {BII_IMG_WIDTH=0, BII_IMG_HEIGHT=1, BII_CURRENTBLOCK=2, BII_MAXBLOCKS=3};

// Actors
/*
 * File: huffman.c
 * Features:
 *      Dependent X-Time: Yes, per Iteration (Image Input Data)
 *      Floatingpoint:    No
 *      Mult./Div.:       No
 */
void GetEncodedImageBlock(
        token_t DCOffset[3]   /*in/out*/,
        token_t Y[8*8]        /*out*/,
        token_t Cr[8*8]       /*out*/,
        token_t Cb[8*8]       /*out*/,
        token_t BlockInfo[4]  /*out [optional]*/); // 0: ImgW, 1: ImgH, 2: Block, 3: MaxBlocks

/*
 * File: quantization.c
 * Features:
 *      Dependent X-Time: Yes, per Instance (quantization table)
 *                        Yes, per Iteration (QuantizedCoefficients get rounded
 *      Floatingpoint:    No
 *      Mult./Div.:       Yes (64 multiplications)
 */
void InverseQuantization(
        const uint8_t quantizationtable[8*8],   /*config*/
        token_t QuantizedCoefficients[8*8] /*in*/,
        token_t DCTCoefficients[8*8]       /*out*/);

/*
 * File: IDCT.c
 * Features:
 *      Dependent X-Time: Yes, per Iteration (rounding)
 *      Floatingpoint:    Yes (thousands of times incl sqrtf and cosf)
 *      Mult./Div.:       Yes (thousands of multiplications and dividations)
 */
void IDCT(
        token_t source[8*8]      /*in*/,
        token_t destination[8*8] /*out*/);

/*
 * File: IDCT.c
 * Features:
 *      Dependent X-Time: No
 *      Floatingpoint:    Yes (Lots of *,+,-)
 *      Mult./Div.:       No
 */
void FastIDCT(
        token_t source[8*8]      /*in*/,
        token_t destination[8*8] /*out*/);

/*
 * File: ColorConversion.c
 * Features:
 *      Dependent X-Time: Yes, per Iteration (rounding)
 *      Floatingpoint:    Yes (~100 +,-,·,/)
 *      Mult./Div.:       No (Only float)
 */
void CreateRGBPixels(
        token_t Y[8*8]      /*in*/, 
        token_t Cr[8*8]     /*in*/, 
        token_t Cb[8*8]     /*in*/,
        token_t Pixels[8*8] /*out*/
        );
/*
 * File: ColorConversion.c
 * Features:
 *      Dependent X-Time: Yes, per Iteration (rounding)
 *      Floatingpoint:    Yes (~100 +,-,·,/)
 *      Mult./Div.:       No (Only float)
 */
void ColorConversion(
        token_t Y[8*8]      /*in*/, 
        token_t Cr[8*8]     /*in*/, 
        token_t Cb[8*8]     /*in*/,
        token_t R[8*8]      /*out*/, 
        token_t G[8*8]      /*out*/, 
        token_t B[8*8]      /*out*/
        );
/*
 * File: ColorConversion.c
 * Features:
 *      Dependent X-Time: Yes, per Iteration (rounding)
 *      Floatingpoint:    No
 *      Mult./Div.:       No
 */
void ConvertRGB24to16(
        token_t Pixels24[8*8] /*in*/, 
        token_t Pixels16[8*8] /*out*/
        );


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

