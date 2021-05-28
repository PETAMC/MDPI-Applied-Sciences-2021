#include "decoding.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "image.h"

// Internals

// Does not care about Image_size.
// To start from the beginning, call with reset=true - the returned value is then invalid!
uint8_t GetNextBitFromImage(bool reset);

// Huffman decoding
void GetEncodedImageBlockComponent(enum component_t component, token_t dcoffset, token_t matrix[8*8]);
uint8_t GetHuffmanDecodedValueFromImage(int tablesize, const uint16_t *codes, const uint8_t *values, const uint16_t *masks);
signed int DCCodeToInteger(uint16_t dccode, uint8_t dclength);



void GetEncodedImageBlock(token_t DCOffsets[3], token_t Y[8*8], token_t Cr[8*8], token_t Cb[8*8],
        token_t BlockInfo[4])
{
    // ">> 3" = "/ 8"
    const uint16_t  numofblocks = (Image_xdimension >> 3) * (Image_ydimension >> 3);
    static uint16_t blockcount  = 0;

    // When this is the first block, …
    if(blockcount == 0)
    {
        GetNextBitFromImage(true);   // … reset decoder
        DCOffsets[COMP_Y ] = 0;
        DCOffsets[COMP_Cb] = 0;
        DCOffsets[COMP_Cr] = 0;
    }

    // For Y, Cr, Cb
    GetEncodedImageBlockComponent(COMP_Y,  DCOffsets[COMP_Y ], Y);
    GetEncodedImageBlockComponent(COMP_Cb, DCOffsets[COMP_Cb], Cb);
    GetEncodedImageBlockComponent(COMP_Cr, DCOffsets[COMP_Cr], Cr);
    DCOffsets[COMP_Y ] = Y[0];
    DCOffsets[COMP_Cb] = Cb[0];
    DCOffsets[COMP_Cr] = Cr[0];

    // Output additional information
    if(BlockInfo != NULL)
    {
        BlockInfo[0] = Image_xdimension;
        BlockInfo[1] = Image_ydimension;
        BlockInfo[2] = blockcount;
        BlockInfo[3] = numofblocks;
    }

    // Update internal state for next block
    blockcount++;
    if(blockcount >= numofblocks)
        blockcount = 0;
}


void GetEncodedImageBlockComponent(enum component_t component, token_t dcoffset, token_t matrix[8*8])
{
    int             DCHT_size;
    const uint16_t *DCHT_codes;
    const uint8_t  *DCHT_values;
    const uint16_t *DCHT_masks;
    int             ACHT_size;
    const uint16_t *ACHT_codes;
    const uint8_t  *ACHT_values;
    const uint16_t *ACHT_masks;
    
    switch(component)
    {
        case COMP_Y:
            DCHT_size   = Y_DCHT_size;
            DCHT_codes  = Y_DCHT_codes;
            DCHT_values = Y_DCHT_values;
            DCHT_masks  = Y_DCHT_masks;
            ACHT_size   = Y_ACHT_size;
            ACHT_codes  = Y_ACHT_codes;
            ACHT_values = Y_ACHT_values;
            ACHT_masks  = Y_ACHT_masks;
            break;

        case COMP_Cr:
            DCHT_size   = Cr_DCHT_size;
            DCHT_codes  = Cr_DCHT_codes;
            DCHT_values = Cr_DCHT_values;
            DCHT_masks  = Cr_DCHT_masks;
            ACHT_size   = Cr_ACHT_size;
            ACHT_codes  = Cr_ACHT_codes;
            ACHT_values = Cr_ACHT_values;
            ACHT_masks  = Cr_ACHT_masks;
            break;

        case COMP_Cb:
            DCHT_size   = Cb_DCHT_size;
            DCHT_codes  = Cb_DCHT_codes;
            DCHT_values = Cb_DCHT_values;
            DCHT_masks  = Cb_DCHT_masks;
            ACHT_size   = Cb_ACHT_size;
            ACHT_codes  = Cb_ACHT_codes;
            ACHT_values = Cb_ACHT_values;
            ACHT_masks  = Cb_ACHT_masks;
            break;
    }

    // Decode DC
    // read code length
    uint8_t dclength;
    dclength = GetHuffmanDecodedValueFromImage(
            DCHT_size, 
            DCHT_codes,
            DCHT_values,
            DCHT_masks
            );

    // read code word
    uint16_t dccode = 0;
    for(int i=0; i<dclength; i++)
        dccode = (dccode << 1) | (uint16_t)GetNextBitFromImage(false);

    // decode DC value
    signed int dcvalue;
    dcvalue = DCCodeToInteger(dccode, dclength);

#ifdef DEBUG
    printf("\e[1;34mDC length: \e[0;36m0x%02X\n", dclength);
    printf("\e[1;34mDC code:   \e[0;36m0x%02X\n", dccode);
    printf("\e[1;34mDC value:  \e[0;36m%d\n",     dcvalue);
#endif

    matrix[0] = dcoffset + dcvalue; // Difference Encoding!

    // Decode AC values
    for(int i=1; i<8*8; i++)
    {
        uint8_t acrlc;
        acrlc = GetHuffmanDecodedValueFromImage(
                ACHT_size, 
                ACHT_codes,
                ACHT_values,
                ACHT_masks
                );

        if(acrlc == 0x00) // End Of Block - All left values are 0x00
        {
            for(; i<8*8; i++)
                matrix[i] = 0x00;
            break;
        }
        else if(acrlc == 0xF0) // ZRL Code
        {
            for(int zeros=0; zeros<16; zeros++, i++)
                matrix[i] = 0x00;
            continue;
        }

        // Decode Run-Length-Code
        uint8_t numzeros, aclength;
        numzeros  = (acrlc >> 4) & 0x0F; // Beware of arithmetic right shift!
        aclength  = acrlc & 0x0F;

        // Set zeros
        for(int zeros=0; zeros<numzeros; zeros++, i++)
            matrix[i] = 0x00;

        // Get value code
        uint16_t accode = 0;
        for(int acvaluebit=0; acvaluebit<aclength; acvaluebit++)
            accode = (accode << 1) | (uint16_t)GetNextBitFromImage(false);

        // decode AC value
        signed int acvalue;
        acvalue = DCCodeToInteger(accode, aclength); // Same table for AC and DC

        matrix[i] = acvalue;
    }

#ifdef DEBUG
    printf("\e[1;34mMatrix[%i]:\e[0;36m\n", component);
    for(int y=0; y<8; y++)
    {
        printf("\t");
        for(int x=0; x<8; x++)
        {
            printf("%d\t", matrix[y*8+x]);
        }
        printf("\n");
    }
#endif
}


signed int DCCodeToInteger(uint16_t dccode, uint8_t dclength)
{
#ifdef DEBUG
    if(dccode != 0)
    {
        printf("\e[1;30mDCCodeToInteger(0x%04X, %d) -> 0x%04X & 0x%04X\n", 
                dccode, dclength,
                dccode, 1<<(dclength-1));
    }
#endif
    signed int dcvalue;
    if(dccode & ( 1 << (dclength-1) ))  // if first bit is 1, then this is a positive value
        dcvalue = (signed int)(dccode);
    else 
        dcvalue = (signed int)(dccode - ( (1<<dclength) - 1 ) );

    return dcvalue;
}


uint8_t GetHuffmanDecodedValueFromImage(int tablesize, const uint16_t *codes, const uint8_t *values, const uint16_t *masks)
{
    uint16_t codeword = 0x0000;
    uint16_t wordmask = 0x0000;
    uint8_t  value    = 0xFF;       // 0xFF is invalid and indicates failed decoding
    do
    {
        codeword = (codeword << 1) | GetNextBitFromImage(false);
        wordmask = (wordmask << 1) | 1;

        for(int htindex=0; htindex < tablesize; htindex++)
        {
            // Check Mask - If they do not match, skip this code word
            if(wordmask ^ masks[htindex])
                continue;

            if(codeword == codes[htindex])
            {
                value = values[htindex];
                break;
            }
        }
    }
    while(value == 0xFF); // Until a valid value was decoded
    return value;
}




// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

