#ifndef JPEGDEFINITION
#define JPEGDEFINITION

#include <stdint.h>

extern const uint16_t Image_xdimension;
extern const uint16_t Image_ydimension;
extern const uint16_t Image_size; // number of bytes of compressed data
extern const uint8_t  Image_data[];

extern const uint8_t  Y_xresolution;
extern const uint8_t  Y_yresolution;
extern const uint8_t  Cr_xresolution;
extern const uint8_t  Cr_yresolution;
extern const uint8_t  Cb_xresolution;
extern const uint8_t  Cb_yresolution;

extern const uint8_t  Y_quantizationtable[8*8];
extern const uint8_t Cr_quantizationtable[8*8];
extern const uint8_t Cb_quantizationtable[8*8];

extern const int        Y_DCHT_size;
extern const uint16_t   Y_DCHT_codes[];
extern const uint8_t    Y_DCHT_values[];
extern const uint16_t   Y_DCHT_masks[];

extern const int        Y_ACHT_size;
extern const uint16_t   Y_ACHT_codes[];
extern const uint8_t    Y_ACHT_values[];
extern const uint16_t   Y_ACHT_masks[];

extern const int       Cr_DCHT_size;
extern const uint16_t  Cr_DCHT_codes[];
extern const uint8_t   Cr_DCHT_values[];
extern const uint16_t  Cr_DCHT_masks[];

extern const int       Cr_ACHT_size;
extern const uint16_t  Cr_ACHT_codes[];
extern const uint8_t   Cr_ACHT_values[];
extern const uint16_t  Cr_ACHT_masks[];

extern const int       Cb_DCHT_size;
extern const uint16_t  Cb_DCHT_codes[];
extern const uint8_t   Cb_DCHT_values[];
extern const uint16_t  Cb_DCHT_masks[];

extern const int       Cb_ACHT_size;
extern const uint16_t  Cb_ACHT_codes[];
extern const uint8_t   Cb_ACHT_values[];
extern const uint16_t  Cb_ACHT_masks[];


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
