#include <errno.h> 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/*
Source: https://de.wikipedia.org/wiki/JPEG_File_Interchange_Format

FF D8   SOI     Start Of Image
FF E0   APP0    JFIF tag
FF Cn   SOFn    Start of Frame Marker, legt Art der Kompression fest:
FF C0   SOF0    Baseline DCT
FF C1   SOF1    Extended sequential DCT
FF C2   SOF2    Progressive DCT
FF C3   SOF3    Lossless (sequential)
FF C4   DHT     Definition der Huffman-Tabellen
FF C5   SOF5    Differential sequential DCT
FF C6   SOF6    Differential progressive DCT
FF C7   SOF7    Differential lossless (sequential)
FF C8   JPG     reserviert für JPEG extensions
FF C9   SOF9    Extended sequential DCT
FF CA   SOF10   Progressive DCT
FF CB   SOF11   Lossless (sequential)
FF CD   SOF13   Differential sequential DCT
FF CE   SOF14   Differential progressive DCT
FF CF   SOF15   Differential lossless (sequential)
FF CC   DAC     Definition der arithmetischen Codierung
FF DB   DQT     Definition der Quantisierungstabellen
FF DD   DRI     Define Restart Interval
FF E1   APP1    Exif-Daten
FF EE   APP14   Oft für Copyright-Einträge
FF En   APPn    n=2..F allg. Zeiger
FF FE   COM     Kommentare
FF DA   SOS     Start of Scan
FF D9   EOI     End of Image 

Other sources:
https://www.impulseadventure.com/photo/jpeg-huffman-coding.html
*/

#define TAG_StartOfImage        0xD8FF
#define TAG_JFIFTag             0xE0FF
#define TAG_QuantizationTable   0xDBFF
#define TAG_BaselineDCT         0xC0FF
#define TAG_HuffmanTable        0xC4FF
#define TAG_StartOfScan         0xDAFF

/*
 * Notes:
 * HT:
 *  0x00: End of Component
 *  0xF0: 16 * 0-values
 *      Source: https://www.impulseadventure.com/photo/jpeg-huffman-coding.html
 */

struct HuffmanTable
{
    struct HuffmanTable *next;
    uint8_t  class; // 0: DC, 1: AC
    uint8_t  id;
    int codecount;  // Number of codes in the table
    uint16_t *codes;
    uint8_t  *values;
    uint16_t *masks;
};

struct BaselineDCTColor
{
    uint8_t xresolution;
    uint8_t yresolution;
    uint8_t qtid;   // Quantisation table ID
};
struct BaselineDCT
{
    uint16_t xdimension;
    uint16_t ydimension;

    struct BaselineDCTColor Y;
    struct BaselineDCTColor Cb;
    struct BaselineDCTColor Cr;
};

struct QuantizationTable
{
    struct QuantizationTable *next;
    uint8_t id;
    uint8_t table[8*8]; // y · 8 + x
};

struct ImageData
{
    uint8_t YDCHTid;
    uint8_t YACHTid;
    uint8_t CbDCHTid;
    uint8_t CbACHTid;
    uint8_t CrDCHTid;
    uint8_t CrACHTid;

    uint16_t size;
    uint8_t  *data;
};


// This struct contains all infos needed to decode one color channel (Y,Cr,CB)
struct JPEGComponent
{
    uint8_t xresolution;
    uint8_t yresolution;
    uint8_t quantizationtable[8*8];
    
    int       DCHTsize;
    uint16_t *DCHTcodes;
    uint8_t  *DCHTvalues;
    uint16_t *DCHTmasks;

    int       ACHTsize;
    uint16_t *ACHTcodes;
    uint8_t  *ACHTvalues;
    uint16_t *ACHTmasks;
};

int ReadFile(const char *path);
int ReadJFIFTag(FILE *file);
int ReadQuantizationTable(FILE *file, struct QuantizationTable **quantizationtable);
int ReadBaselineDCT(FILE *file, struct BaselineDCT *baselinedct);
int ReadHuffmanTable(FILE *file, struct HuffmanTable **huffmantable);
int ReadScan(FILE *file, struct ImageData *data);

int SeparateComponents(
        struct HuffmanTable      *huffmantables,
        struct BaselineDCT       *baselinedct,
        struct QuantizationTable *quantizationtables,
        struct ImageData         *imagedata,

        struct JPEGComponent *Y,
        struct JPEGComponent *Cr,
        struct JPEGComponent *Cb
        );

int WriteHeaderFile(const char *path, 
        uint16_t xdimension,
        uint16_t ydimension,
        struct ImageData     *imagedata,
        struct JPEGComponent *Y,
        struct JPEGComponent *Cr,
        struct JPEGComponent *Cb
        );
void WriteQuantizationTable(FILE *file, const char *tablename, uint8_t table[8*8]);
void WriteHuffmanTable(FILE *file, const char *nameprefix, int tablesize, uint16_t *codes, uint8_t *values, uint16_t *masks);

int main(int argc, char *argv[])
{
    ReadFile(argv[1]);
    return EXIT_SUCCESS;
}

int ReadFile(const char *path)
{
    FILE *file;
    file = fopen(path, "rb");
    if(file == NULL)
    {
        fprintf(stderr, "Opening \"%s\" failed with error \"%s\"\n", path, strerror(errno));
        return -1;
    }

    // Read expected 0xFF, 0xD8 - Marker for the start of an Image
    uint16_t StartOfImage;
    fread(&StartOfImage, 2, 1, file);
    if(StartOfImage != TAG_StartOfImage)
    {
        fprintf(stderr, "Start Of Image marker missing! Expected: 0xFFD8 Got: 0x%04X\n", StartOfImage);
        fclose(file);
        return -1;
    }

    struct HuffmanTable      *huffmantables = NULL;
    struct BaselineDCT       baselinedct;
    struct QuantizationTable *quantizationtable = NULL;
    struct ImageData         imagedata;
    // Now read all the part of the file
    do
    {
        uint16_t tag;
        int error;
        size_t count;

        count = fread(&tag, 2, 1, file);
        if(count == 0)
            break;
        switch(tag)
        {
            case TAG_JFIFTag:
                error = ReadJFIFTag(file);
                break;

            case TAG_QuantizationTable:
                error = ReadQuantizationTable(file, &quantizationtable);
                break;

            case TAG_BaselineDCT:
                error = ReadBaselineDCT(file, &baselinedct);
                break;

            case TAG_HuffmanTable:
                error = ReadHuffmanTable(file, &huffmantables);
                break;

            case TAG_StartOfScan:
                error = ReadScan(file, &imagedata);
                break;

            default:
                {
                    fprintf(stderr, "\e[1;33mUnknown tag 0x%04X @ 0x%04lX!", tag, ftell(file)-2);
                    if((tag & 0xFF) != 0xFF)
                    {
                        fprintf(stderr, "\e[1;31m Exiting");
                        error = -1;
                        break;
                    }
                    uint16_t length=0;
                    fread(&length, 2, 1, file);
                    length = be16toh(length) - 2; // -2 for the already read length
                    fprintf(stderr, "\e[1;30m Skipping %d bytes … \n", length);
                    fseek(file, length, SEEK_CUR);
                }

        }
        if(error)
        {
            fclose(file);
            return error;
        }
    }
    while(!feof(file));


    fclose(file);

    struct JPEGComponent Y, Cr, Cb;
    SeparateComponents(huffmantables, &baselinedct, quantizationtable, &imagedata, &Y, &Cr, &Cb);

    WriteHeaderFile("./image.c", baselinedct.xdimension, baselinedct.ydimension, &imagedata, &Y, &Cr, &Cb);
    return 0;
}



int ReadJFIFTag(FILE *file)
{
    printf("\e[1;37mJFIF Tag\n");
    uint16_t length;
    fread(&length, 2, 1, file);
    length = be16toh(length);

    uint8_t identifier[5];
    fread(&identifier, 1, 5, file);
    
    printf("\e[1;34mLength: \e[0;36m%d\n", (int)length);
    printf("\e[1;34mID: \e[0;36m%02X %02X %02X %02X %02X\n",
            identifier[0], identifier[1], identifier[2], identifier[3], identifier[4]);

    if(identifier[0] != 'J' 
    || identifier[1] != 'F' 
    || identifier[2] != 'I' 
    || identifier[3] != 'F' 
    || identifier[4] != 0x00)
    {
        fprintf(stderr, "Invalid ID: %02X %02X %02X %02X %02X\n", 
                identifier[0], identifier[1], identifier[2], identifier[3], identifier[4]);
        return -1;
    }

    uint8_t version[2];
    fread(&version, 1, 2, file);
    printf("\e[1;34mVersion \e[0;36m%d.%d\n", version[0], version[1]);

    uint8_t units;
    fread(&units, 1, 1, file);
    
    uint16_t x_density, y_density;
    fread(&x_density, 2, 1, file);
    fread(&y_density, 2, 1, file);
    x_density = be16toh(x_density);
    y_density = be16toh(y_density);
    printf("\e[1;34mX;Y-Density: \e[0;36m%d;%d in \e[0;36m%d \e[1;30m(0:aspect ratio; 1:dots/inch; 2:dots/cm)\n",
            x_density, y_density, units);

    uint8_t thumbnail_width, thumbnail_height;
    fread(&thumbnail_width,  1, 1, file);
    fread(&thumbnail_height, 1, 1, file);
    printf("\e[1;34mThumbnail dimensions: \e[0;36m%d·%d\n", thumbnail_width, thumbnail_height);

    fseek(file, thumbnail_width * thumbnail_height, SEEK_CUR);

    return 0;
}



/*
struct QuantizationTable
{
    struct QuantizationTable *next;
    uint8_t id;
    uint8_t table[8*8]; // y · 8 + x
}
 */
int ReadQuantizationTable(FILE *file, struct QuantizationTable **quantizationtable)
{
    printf("\e[1;37mQuantization Table\n");

    uint16_t length;
    fread(&length, 2, 1, file);
    length = be16toh(length);
    printf("\e[1;34mLength: \e[0;36m%d\n", (int)length);

    if(length != 67 && length != 132) // [ Index;Precision (1byte) + (8·8) ] · x + length (2bytes)
    {
        fprintf(stderr, "Unexpected length of quantization table frame: %d. Expected: 67!\n", length);
        return -1;
    }

    int numQTs;
    numQTs = (length - 2) / (8*8);
    printf("\e[1;34mEncoded QTs: \e[0;36m%d\n", numQTs);

    for(int QT = 0; QT < numQTs; QT++)
    {
        struct QuantizationTable *qt;
        qt = malloc(sizeof(struct QuantizationTable));
        if(qt == NULL)
        {
            fprintf(stderr, "malloc returned NULL!\n");
            return -1;
        }

        printf("\e[1;34mReading Quantization Table \e[1;36m%d\n", QT);
        uint8_t header;
        fread(&header, 1, 1, file);
        int precision, tableindex; // Table index: 0:Y, 1:Cr, 2:Cb
        tableindex = header & 0x0F;
        precision  = (header >> 4) & 0x0F;
        printf("\e[1;34m\tHeader: \e[0;36m%02X\n", header);
        printf("\e[1;34m\tTable Index: \e[0;36m%d \e[1;30m(0:Y, 1:Cb, 2:Cr)\n", (int)tableindex);
        printf("\e[1;34m\tPrecision: \e[0;36m%d\n", (int)precision);
        qt->id = tableindex;


        uint8_t qtable[64];
        fread(&(qt->table), 1, 64, file);
#ifdef DEBUG
        printf("\e[1;34m\tQuantization table: \e[1;30m(ZigZag order)\n");
        for(int x=0; x<8; x++)
        {
            printf("\t");
            for(int y=0; y<8; y++)
            {
                printf(" \e[0;36m0x%02X\e[1;34m,", qt->table[y*8+x]);
            }
            printf("\n");
        }
#endif

        qt->next = *quantizationtable;
        *quantizationtable= qt;
    }

    return 0;
}


int ReadBaselineDCT(FILE *file, struct BaselineDCT *baselinedct)
{
    // Source http://home.elka.pw.edu.pl/~mmanowie/psap/neue/1%20JPEG%20Overview.htm
    printf("\e[1;37mBaseline DCT\n");

    uint16_t length;
    fread(&length, 2, 1, file);
    length = be16toh(length);
    printf("\e[1;34mLength: \e[0;36m%d\n", (int)length);

    uint8_t numpixels;
    fread(&numpixels, 1, 1, file);
    printf("\e[1;34mPixels/Block: \e[0;36m%d \e[1;30m(?)\n", (int)numpixels);

    uint16_t width, height;
    fread(&height, 2, 1, file);
    fread(&width,  2, 1, file);
    height = be16toh(height);
    width  = be16toh(width);
    printf("\e[1;34mDimensions: \e[0;36m%d·%d\n", width, height);
    baselinedct->xdimension = width;
    baselinedct->ydimension = height;

    uint8_t numcolors;
    fread(&numcolors, 1, 1, file);
    printf("\e[1;34mNum. Colors: \e[0;36m%d\n", numcolors);
    if(numcolors != 3)
    {
        fprintf(stderr, "Unexpected number of colors: %d. Expected: 3!\n", numcolors);
        return -1;
    }

    for(int color = 0; color < numcolors; color++)
    {
        struct BaselineDCTColor *dctcolor;
        switch(color)
        {
            case 0:
                dctcolor = &baselinedct->Y;  break;
            case 1:
                dctcolor = &baselinedct->Cb; break;
            case 2:
                dctcolor = &baselinedct->Cr; break;
            default:
                fprintf(stderr, "Unexpected color index: %d. Expected: {0,1,2}!\n", color);
                return -1;
        }

        printf("\e[1;34mFor color \e[0;36m%d\n", color);
        uint8_t colorid;
        uint8_t sampling;   // Sampling resolution (vertical:horizontal)
        uint8_t qtindex;    // Quantization Table index
        fread(&colorid,  1, 1, file);
        fread(&sampling, 1, 1, file);
        fread(&qtindex,  1, 1, file);
        int verticalsampling;
        int horizontalsampling;
        verticalsampling   = (sampling >> 4) & 0x0F;
        horizontalsampling = sampling & 0x0F;
        printf("\t\e[1;34mColor ID:   \e[0;36m%d\n", colorid);
        printf("\t\e[1;34mSampling resolution: \e[0;36m%d·%d\n", verticalsampling, horizontalsampling);
        printf("\t\e[1;34mQT Index:   \e[0;36m%d\n", qtindex);
        dctcolor->xresolution = verticalsampling;
        dctcolor->yresolution = horizontalsampling;
        dctcolor->qtid        = qtindex;

    }

    return 0;
}



// This function prepends one or more Huffman tables.
int ReadHuffmanTable(FILE *file, struct HuffmanTable **huffmantable)
{
    // Source: http://imrannazar.com/Let%27s-Build-a-JPEG-Decoder%3A-Huffman-Tables
    printf("\e[1;37mHuffman Table\n");

    uint16_t length;
    fread(&length, 2, 1, file);
    length = be16toh(length);
    printf("\e[1;34mLength: \e[0;36m%d\n", (int)length);
    length -= 2; // subtract length

    // There may be more then one HuffmanTable in this segment
    for(int HT=0; length; HT++)
    {
        struct HuffmanTable *ht;
        ht = malloc(sizeof(struct HuffmanTable));
        if(ht == NULL)
        {
            fprintf(stderr, "malloc returned NULL!\n");
            return -1;
        }

        printf("\e[1;34mReading Huffman Table \e[1;36m%d\n", HT);
        uint16_t code = 0;
        uint8_t tableid;
        uint8_t tableclass;
        fread(&tableid, 1, 1, file);
        length -= 1;
        tableclass = tableid >> 4;
        tableid    = tableid & 0x0F;
        printf("\e[1;34mClass: \e[0;36m0x%02X\n", tableclass);
        printf("\e[1;34mID:    \e[0;36m0x%02X\n", tableid);
        ht->class = tableclass;
        ht->id = tableid;

        uint8_t counts[16];
        fread(&counts, 1, 16, file);
        length -= 16;
        printf("\e[1;34mCounts per Length: ");
        int codecount = 0;
        for(int i=0; i<16; i++)
        {
            printf("\e[1;30m%d:\e[0;36m%d ", i+1, counts[i]);
            codecount += counts[i];
        }
        printf("\n");

        ht->codecount = codecount;
        ht->codes  = malloc(sizeof(uint16_t) * codecount);
        ht->values = malloc(sizeof(uint8_t)  * codecount);
        ht->masks  = malloc(sizeof(uint16_t) * codecount);
        if(ht->codes == NULL || ht->values == NULL || ht->masks == NULL)
        {
            fprintf(stderr, "malloc returned NULL!\n");
            return -1;
        }


        int tableindex = 0;
        uint16_t mask  = 0x0000;
        for(int i=0; i<16; i++)
        {
            mask |= 1<<i;

            for(int j = 0; j < counts[i]; j++)
            {
                uint8_t value;
                fread(&value, 1, 1, file);
                length -= 1;

                ht->codes [tableindex] = code;
                ht->values[tableindex] = value;
                ht->masks [tableindex] = mask;
                tableindex++;

                code++;
            }
            code <<= 1;
        }

        ht->next = *huffmantable;
        *huffmantable = ht;
    }

    return 0;
}



int ReadScan(FILE *file, struct ImageData *imagedata)
{
    //Source http://home.elka.pw.edu.pl/~mmanowie/psap/neue/1%20JPEG%20Overview.htm
    printf("\e[1;37mScan\n");

    uint16_t length;
    fread(&length, 2, 1, file);
    length = be16toh(length);
    printf("\e[1;34mLength: \e[0;36m%d\n", (int)length);

    uint8_t numcolors;
    fread(&numcolors, 1, 1, file);
    printf("\e[1;34mNumber of Colors: \e[0;36m%d\n", (int)numcolors);

    if(numcolors != 3)
    {
        fprintf(stderr, "Unexpected number of colors: %d. Expected: 3!\n", numcolors);
        return -1;
    }

    for(int color = 0; color < numcolors; color++)
    {
        printf("\e[1;34mFor color \e[0;36m%d\n", color);
        uint8_t colorid;
        uint8_t htindex;    // Huffman Table index
        fread(&colorid,  1, 1, file);
        fread(&htindex,  1, 1, file);
        int htindex_DC;
        int htindex_AC;
        htindex_DC = (htindex >> 4) & 0x0F;
        htindex_AC = htindex & 0x0F;
        printf("\t\e[1;34mColor ID:   \e[0;36m%d\n", colorid);
        printf("\t\e[1;34mHT Index:   \e[0;36m0x%02X\e[1;34m -> DC:\e[0;36m%d\e[1;34m, AC:\e[0;36m%d\n", 
                htindex, htindex_DC, htindex_AC);

        switch(colorid) // Color/Component ID starts at 1!
        {
            case 1/*Y*/:
                printf("\t\e[1;34mMapped to Y\n");
                imagedata->YDCHTid = htindex_DC;
                imagedata->YACHTid = htindex_AC;
                break;
            case 2/*Cb*/:
                printf("\t\e[1;34mMapped to Cb\n");
                imagedata->CbDCHTid = htindex_DC;
                imagedata->CbACHTid = htindex_AC;
                break;
            case 3/*Cr*/:
                printf("\t\e[1;34mMapped to Cr\n");
                imagedata->CrDCHTid = htindex_DC;
                imagedata->CrACHTid = htindex_AC;
                break;
        }
    }

    uint8_t SoSS, EoSS, SS; // Start/End of Spectral Selection, Spectral Selection (Not used in Baseline DCT)
    fread(&SoSS, 1, 1, file);
    fread(&EoSS, 1, 1, file);
    fread(&SS,   1, 1, file);
    if(SoSS != 0x00 || EoSS != 0x3F || SS != 0x00)
    {
        fprintf(stderr, "Unexpected Spectral Selection entries in Baseline DCT Scan!\n");
        return -1;
    }

    // Read Actual compressed data:
    printf("\e[1;34mReading compressed data\n\t");
    imagedata->size = 0;
    imagedata->data = NULL;
    while(true)
    {
        uint8_t rawdata;
        fread(&rawdata, 1, 1, file);   // 0xFF is always followed by 0x00!
        if(rawdata == 0xFF)
        {
            fread(&rawdata, 1, 1, file);
            if(rawdata == 0x00)
                rawdata = 0xFF;
            else if(rawdata == 0xD9)
                break;  // End of Scan
            else
            {
                fprintf(stderr, "Unexpected byte after leading 0xFF: 0x%02X. Expected: 0x00 or 0xD9\n", rawdata);
                return -1;
            }
        }

        imagedata->size += 1;
        imagedata->data = realloc(imagedata->data, imagedata->size * sizeof(uint8_t));
        if(imagedata->data == NULL)
        {
            fprintf(stderr, "remalloc returned NULL!\n");
            return -1;
        }
        imagedata->data[imagedata->size - 1] = rawdata;
#ifdef DEBUG
        printf("\e[0;35m%02X\e[1;34m, ", rawdata);
#endif
    }
    printf("\n");

    return 0;
}



int SeparateComponents(
        struct HuffmanTable      *huffmantables,
        struct BaselineDCT       *baselinedct,
        struct QuantizationTable *quantizationtables,
        struct ImageData         *imagedata,

        struct JPEGComponent *Y,
        struct JPEGComponent *Cr,
        struct JPEGComponent *Cb
        )
{
    printf("\e[1;37mMapping data to Components\n");
    // Resolutions
    Y->xresolution  = baselinedct->Y.xresolution;
    Y->yresolution  = baselinedct->Y.yresolution;
    Cr->xresolution = baselinedct->Cr.xresolution;
    Cr->yresolution = baselinedct->Cr.yresolution;
    Cb->xresolution = baselinedct->Cb.xresolution;
    Cb->yresolution = baselinedct->Cb.yresolution;

    // Quantization Tables
    struct QuantizationTable *qt;
    qt = quantizationtables;
    while(qt != NULL)
    {
        printf("\e[1;34mMapping QT ID:\e[0;36m%d\e[1;34m to \e[0;35m", qt->id);
        if(qt->id == baselinedct->Y.qtid)
        {
            printf("Y ");
            memcpy(Y->quantizationtable, qt->table, sizeof(uint8_t)*8*8);
        }
        if(qt->id == baselinedct->Cr.qtid)
        {
            printf("Cr ");
            memcpy(Cr->quantizationtable, qt->table, sizeof(uint8_t)*8*8);
        }
        if(qt->id == baselinedct->Cb.qtid)
        {
            printf("Cb ");
            memcpy(Cb->quantizationtable, qt->table, sizeof(uint8_t)*8*8);
        }
        qt = qt->next;
        printf("\n");
    }

    // Huffman Tables
    struct HuffmanTable *ht;
    ht = huffmantables;
    while(ht != NULL)
    {
        printf("\e[1;34mMapping HT Class:\e[0;36m%d\e[1;34m; ID:\e[0;36m%d\e[1;34m to \e[0;35m", ht->class, ht->id);
        if(ht->class == 0 /*DC*/)
        {
            if(ht->id == imagedata->YDCHTid)
            {
                printf("Y_DC ");
                Y->DCHTsize   = ht->codecount;
                Y->DCHTcodes  = ht->codes;
                Y->DCHTvalues = ht->values;
                Y->DCHTmasks  = ht->masks;
            }
            if(ht->id == imagedata->CrDCHTid)
            {
                printf("Cr_DC ");
                Cr->DCHTsize   = ht->codecount;
                Cr->DCHTcodes  = ht->codes;
                Cr->DCHTvalues = ht->values;
                Cr->DCHTmasks  = ht->masks;
            }
            if(ht->id == imagedata->CbDCHTid)
            {
                printf("Cb_DC ");
                Cb->DCHTsize   = ht->codecount;
                Cb->DCHTcodes  = ht->codes;
                Cb->DCHTvalues = ht->values;
                Cb->DCHTmasks  = ht->masks;
            }
        }
        else if(ht->class == 1 /*AC*/)
        {
            if(ht->id == imagedata->YDCHTid)
            {
                printf("Y_AC ");
                Y->ACHTsize   = ht->codecount;
                Y->ACHTcodes  = ht->codes;
                Y->ACHTvalues = ht->values;
                Y->ACHTmasks  = ht->masks;
            }
            if(ht->id == imagedata->CrDCHTid)
            {
                printf("Cr_AC ");
                Cr->ACHTsize   = ht->codecount;
                Cr->ACHTcodes  = ht->codes;
                Cr->ACHTvalues = ht->values;
                Cr->ACHTmasks  = ht->masks;
            }
            if(ht->id == imagedata->CbDCHTid)
            {
                printf("Cb_AC ");
                Cb->ACHTsize   = ht->codecount;
                Cb->ACHTcodes  = ht->codes;
                Cb->ACHTvalues = ht->values;
                Cb->ACHTmasks  = ht->masks;
            }
        }
        ht = ht->next;
        printf("\n");
    }
    return 0;
}

int WriteHeaderFile(const char *path, 
        uint16_t xdimension,
        uint16_t ydimension,
        struct ImageData     *imagedata,
        struct JPEGComponent *Y,
        struct JPEGComponent *Cr,
        struct JPEGComponent *Cb
        )
{
    FILE *file;
    file = fopen(path, "w");
    if(file == NULL)
    {
        fprintf(stderr, "Opening \"%s\" failed with error \"%s\"\n", path, strerror(errno));
        return -1;
    }

    fprintf(file, "#ifndef JPEGDEFINITION\n");
    fprintf(file, "#define JPEGDEFINITION\n\n");

    fprintf(file, "#include <stdint.h>\n\n");

    // BaselineDCT and Image data
    fprintf(file, "const uint16_t Image_xdimension = %d;\n", xdimension);
    fprintf(file, "const uint16_t Image_ydimension = %d;\n", ydimension);
    fprintf(file, "const uint16_t Image_size       = %d; // number of bytes of compressed data\n", imagedata->size);
    fprintf(file, "const uint8_t  Image_data[]     = {");
    for(int i=0; i<imagedata->size; i++)
    {
        fprintf(file, "0x%02X", imagedata->data[i]);
        if(i < imagedata->size - 1)
            fprintf(file, ", ");
    }
    fprintf(file, "};\n");
    fprintf(file, "\n");

    fprintf(file, "const uint8_t  Y_xresolution  = %d;\n", Y->xresolution);
    fprintf(file, "const uint8_t  Y_yresolution  = %d;\n", Y->yresolution);
    fprintf(file, "const uint8_t  Cb_xresolution = %d;\n", Cb->xresolution);
    fprintf(file, "const uint8_t  Cb_yresolution = %d;\n", Cb->yresolution);
    fprintf(file, "const uint8_t  Cr_xresolution = %d;\n", Cr->xresolution);
    fprintf(file, "const uint8_t  Cr_yresolution = %d;\n", Cr->yresolution);
    fprintf(file, "\n");

    // Quantization Tables
    WriteQuantizationTable(file, " Y_quantizationtable", Y->quantizationtable);
    WriteQuantizationTable(file, "Cb_quantizationtable", Cb->quantizationtable);
    WriteQuantizationTable(file, "Cr_quantizationtable", Cr->quantizationtable);
    fprintf(file, "\n");

    // Huffman Tables
    WriteHuffmanTable(file, "  Y_DCHT_",  Y->DCHTsize,  Y->DCHTcodes,  Y->DCHTvalues,  Y->DCHTmasks);
    WriteHuffmanTable(file, "  Y_ACHT_",  Y->ACHTsize,  Y->ACHTcodes,  Y->ACHTvalues,  Y->ACHTmasks);
    WriteHuffmanTable(file, " Cb_DCHT_", Cb->DCHTsize, Cb->DCHTcodes, Cb->DCHTvalues, Cb->DCHTmasks);
    WriteHuffmanTable(file, " Cb_ACHT_", Cb->ACHTsize, Cb->ACHTcodes, Cb->ACHTvalues, Cb->ACHTmasks);
    WriteHuffmanTable(file, " Cr_DCHT_", Cr->DCHTsize, Cr->DCHTcodes, Cr->DCHTvalues, Cr->DCHTmasks);
    WriteHuffmanTable(file, " Cr_ACHT_", Cr->ACHTsize, Cr->ACHTcodes, Cr->ACHTvalues, Cr->ACHTmasks);
    fprintf(file, "\n");


    fprintf(file, "#endif\n");
    fprintf(file, "// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4\n");
    return 0;
}



void WriteQuantizationTable(FILE *file, const char *tablename, uint8_t table[8*8])
{
    fprintf(file, "const uint8_t %s[8*8] = \n", tablename);
    fprintf(file, "\t{\n");
    for(int y=0; y<8; y++)
    {
        fprintf(file, "\t\t");
        for(int x=0; x<8; x++)
        {
            fprintf(file, "0x%02X", table[y*8+x]);
            if(y == 8-1 && x == 8-1)
                fprintf(file, "\t");
            else
                fprintf(file, ", ");
        }
        fprintf(file, "\n");
    }
    fprintf(file, "\t};\n");
}

void WriteHuffmanTable(FILE *file, const char *nameprefix, int tablesize, uint16_t *codes, uint8_t *values, uint16_t *masks)
{
    fprintf(file, "const int      %ssize     = %d;\n", nameprefix, tablesize);
    fprintf(file, "const uint16_t %scodes[]  = {", nameprefix);
    for(int i=0; i<tablesize; i++)
    {
        fprintf(file, "0x%04X", codes[i]);
        if(i < tablesize-1)
            fprintf(file, ", ");
    }
    fprintf(file, "};\n");
    fprintf(file, "const uint8_t  %svalues[] = {", nameprefix);
    for(int i=0; i<tablesize; i++)
    {
        fprintf(file, "0x%02X", values[i]);
        if(i < tablesize-1)
            fprintf(file, ", ");
    }
    fprintf(file, "};\n");
    fprintf(file, "const uint16_t %smasks[]  = {", nameprefix);
    for(int i=0; i<tablesize; i++)
    {
        fprintf(file, "0x%04X", masks[i]);
        if(i < tablesize-1)
            fprintf(file, ", ");
    }
    fprintf(file, "};\n");
    fprintf(file, "\n");
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

