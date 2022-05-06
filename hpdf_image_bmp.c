
#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf.h"
#include "hpdf_image.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static const char* COL_CMYK = "DeviceCMYK";
static const char* COL_RGB = "DeviceRGB";
static const char* COL_GRAY = "DeviceGray";


#pragma pack(2)
typedef struct _bitmap_file_header
{
    short fileType;
    int fileSize;
    int reserved;
    int pixelDataOffset;
} BitmapFileHeader;

#pragma pack()
typedef struct _bitmap_info_header
{
    int headerSize;
    int imageWidth;
    int imageHeight;
    short planes;
    short bitsPerPixel;
    int compression;
    int imageSize;
    int xPixelsPerMeter;
    int yPixelsPerMeter;
    int totalColors;
    int importantColors;
} BitmapInfoHeader;

static HPDF_STATUS LoadBmpData(HPDF_Dict image,  HPDF_Xref xref, HPDF_Stream bmp_data, int pixelDataOffset, HPDF_BOOL delayed_loading);

#if 0
cp_mat* init_cp_mat(unsigned int width, unsigned int height, unsigned int channel) {
    unsigned int i;
    cp_mat* mat = NULL;
    if (width <= 0 || height <= 0 || channel <= 0) {
        return mat;
    }
    mat = (cp_mat*)malloc(sizeof(cp_mat));
    if (mat != NULL) {
        mat->width = width;
        mat->height = height;
        mat->channel = channel;
        mat->data = (unsigned char**)malloc(sizeof(unsigned char*) * height);
        if (mat->data != NULL) {
            mat->data[0] = (unsigned char*)malloc(sizeof(unsigned char) * width * height * channel);
            if (mat->data[0] != NULL) {
                for (i = 1; i < height; i++) {
                    mat->data[i] = mat->data[i - 1] + width * channel;
                }
            }
        }
    }
    return mat;
}

void free_cp_mat(cp_mat* mat) {
    if (mat != NULL) {
        if (mat->data != NULL) {
            free(mat->data[0]);
            free(mat->data);
            mat->data = NULL;
        }
        free(mat);
        mat = NULL;
    }
}

cp_mat* cp_read_bmp_image(char* file) {
    cp_mat* mat = NULL;
    FILE* f = NULL;
    BitmapFileHeader bitmapFileHeader;
    BitmapInfoHeader bitmapInfoHeader;
    unsigned char* buf = NULL;
    int row_size, i, j;//, pos = 0;
    int padding = 0;
    char padding_buf[8];
    size_t read_size;
    int ret;
#if _WINDOWS
    ret = fopen_s(&f, file, "rb+");
#else
    f = fopen(file, "rb+");
#endif
    if (f != NULL) {
        read_size = fread(&bitmapFileHeader, sizeof(BitmapFileHeader), 1, f);
        read_size = fread(&bitmapInfoHeader, sizeof(BitmapInfoHeader), 1, f);
        mat = init_cp_mat(bitmapInfoHeader.imageWidth, bitmapInfoHeader.imageHeight, bitmapInfoHeader.bitsPerPixel == 1 ? 1 : bitmapInfoHeader.bitsPerPixel / 8);
        if (mat != NULL) {
            // define row size for reading
            row_size = 4 * ((bitmapInfoHeader.imageWidth * bitmapInfoHeader.bitsPerPixel + 31) / 32);
            padding = row_size - bitmapInfoHeader.imageWidth;

            //printf("offsetBits: %d\n", bitmapFileHeader.pixelDataOffset);
            fseek(f, bitmapFileHeader.pixelDataOffset, SEEK_SET);

            // read grayscale images
            if (bitmapInfoHeader.bitsPerPixel == 8) {
                for (i = 0; i < bitmapInfoHeader.imageHeight; i++) {
                    read_size = fread(&mat->data[bitmapInfoHeader.imageHeight - i - 1][0], 1, bitmapInfoHeader.imageWidth, f);
                    read_size = fread(&padding_buf, 1, padding, f);
                }
            }
            else if (bitmapInfoHeader.bitsPerPixel == 24) {
                for (i = 0; i < bitmapInfoHeader.imageHeight; i++) {
                    read_size = fread(&mat->data[bitmapInfoHeader.imageHeight - i - 1][0], 1, row_size, f);
                }
            }
            else if (bitmapInfoHeader.bitsPerPixel == 1) {
                printf("w: %d\n", row_size);
                buf = (unsigned char*)malloc(row_size);
                for (i = 0; i < bitmapInfoHeader.imageHeight; i++) {
                    read_size = fread(&buf[0], 1, row_size, f);
                    for (j = 0; j < bitmapInfoHeader.imageWidth; j++) {
                        mat->data[bitmapInfoHeader.imageHeight - i - 1][j] = (1 - ((buf[j / 8] >> (7 - j % 8)) & 0x01)) * 0xFF;
                    }
                }
                free(buf);
            }
        }
        fclose(f);
    }
    return mat;;
}

#endif

HPDF_Image
HPDF_Image_LoadBmpImage(HPDF_MMgr        mmgr,
    HPDF_Stream      bmp_data,
    HPDF_Xref        xref,
    HPDF_BOOL        delayed_loading)
{
    HPDF_STATUS ret;
    HPDF_Dict image;
    BitmapFileHeader bitmapFileHeader;
    
    HPDF_UINT len = sizeof(BitmapFileHeader);

    HPDF_PTRACE((" HPDF_Image_LoadBmpImage\n"));

    HPDF_MemSet(&bitmapFileHeader, 0x00, len);
    ret = HPDF_Stream_Read(bmp_data, &bitmapFileHeader, &len);
    if (ret != HPDF_OK || bitmapFileHeader.fileType != 0x4D42) {
        HPDF_SetError(mmgr->error, HPDF_INVALID_BMP_IMAGE, 0);
        return NULL;
    }

    image = HPDF_DictStream_New(mmgr, xref);
    if (!image)
        return NULL;

    image->header.obj_class |= HPDF_OSUBCLASS_XOBJECT;
    ret += HPDF_Dict_AddName(image, "Type", "XObject");
    ret += HPDF_Dict_AddName(image, "Subtype", "Image");
    if (ret != HPDF_OK)
        return NULL;

    if (LoadBmpData(image, xref, bmp_data, bitmapFileHeader.pixelDataOffset, delayed_loading) != HPDF_OK)
        return NULL;




    return image;
}



static HPDF_STATUS LoadBmpData(HPDF_Dict image, HPDF_Xref xref, HPDF_Stream  bmp_data, int pixelDataOffset, HPDF_BOOL delayed_loading) {
    HPDF_STATUS ret = HPDF_OK;
    int bit_depth, color_type;
    unsigned char* buf = NULL, *data = NULL;
    BitmapInfoHeader bitmapInfoHeader;
    HPDF_UINT size;
    int len = sizeof(BitmapInfoHeader);
    int row_size, i, j;//, pos = 0;
    int padding = 0;
    char padding_buf[8];
    size_t read_size;

    HPDF_PTRACE((" HPDF_Image_LoadBmpImage\n"));

    ret = HPDF_Stream_Read(bmp_data, &bitmapInfoHeader, &len);
    if (ret != HPDF_OK) {
        return NULL;
    }

    if (HPDF_Dict_AddNumber(image, "Width", bitmapInfoHeader.imageWidth) != HPDF_OK)
        return NULL;

    if (HPDF_Dict_AddNumber(image, "Height", bitmapInfoHeader.imageHeight) != HPDF_OK)
        return NULL;

    if (HPDF_Dict_AddNumber(image, "BitsPerComponent", 8) != HPDF_OK)
        return NULL;

    if (bitmapInfoHeader.bitsPerPixel == 1 || bitmapInfoHeader.bitsPerPixel == 8) {
        size = bitmapInfoHeader.imageWidth * bitmapInfoHeader.imageHeight;
        ret = HPDF_Dict_AddName(image, "ColorSpace", COL_GRAY);
    } else if (bitmapInfoHeader.bitsPerPixel == 24) {
        ret = HPDF_Dict_AddName(image, "ColorSpace", COL_RGB);
        size = bitmapInfoHeader.imageWidth * bitmapInfoHeader.imageHeight * (bitmapInfoHeader.bitsPerPixel/8);
    } else if (bitmapInfoHeader.bitsPerPixel == 32) {
        ret = HPDF_Dict_AddName(image, "ColorSpace", COL_CMYK);
        size = bitmapInfoHeader.imageWidth * bitmapInfoHeader.imageHeight * (bitmapInfoHeader.bitsPerPixel / 8);
    }


    data = malloc(sizeof(unsigned char) * size);

    if (bitmapInfoHeader.bitsPerPixel > 1) {
        // 14: sizeof(BitmapFileHeader)
        // sizeof(BitmapInfoHeader)
        // xxx pixelDataOffset - 14 - sizeof(BitmapInfoHeader)
        // read until pixelDataOffset
        len = sizeof(unsigned char) * (pixelDataOffset - sizeof(BitmapFileHeader) - sizeof(BitmapInfoHeader));
        buf = (unsigned char*)malloc(len);
        ret = HPDF_Stream_Read(bmp_data, buf, &len);

        ret = HPDF_Stream_Read(bmp_data, data, &size);

        for (i = 0; i < 12; i++) {
            for (j = 0; j < 12; j++) {
                printf("%02X ", data[i * 12 + j]);
            }
            printf("\n");
        }

        printf("\n");
        if (ret != HPDF_OK)
            return NULL;
        ret = HPDF_Stream_Write(image->stream, data, size);
        if (ret != HPDF_OK)
            return NULL;
    }
    else { // 1bit
        buf = (unsigned char*)malloc(row_size);
        for (i = 0; i < bitmapInfoHeader.imageHeight; i++) {
            ret = HPDF_Stream_Read(bmp_data, buf, &row_size);
            for (j = 0; j < bitmapInfoHeader.imageWidth; j++) {
                data[(bitmapInfoHeader.imageHeight - i - 1) * bitmapInfoHeader.imageWidth + j] = (1 - ((buf[j / 8] >> (7 - j % 8)) & 0x01)) * 0xFF;
            }
        }

        if (HPDF_Stream_WriteToStream(bmp_data, image->stream, 0, NULL) != HPDF_OK)
            return NULL;
        
    }
    free(buf);
    free(data);
    
        // read grayscale images
#if 0
        else if (bitmapInfoHeader.bitsPerPixel == 1) {
            printf("w: %d\n", row_size);
            
            for (i = 0; i < bitmapInfoHeader.imageHeight; i++) {
                read_size = fread(&buf[0], 1, row_size, f);
                for (j = 0; j < bitmapInfoHeader.imageWidth; j++) {
                    mat->data[bitmapInfoHeader.imageHeight - i - 1][j] = (1 - ((buf[j / 8] >> (7 - j % 8)) & 0x01)) * 0xFF;
                }
            }
            free(buf);
        }
    }


    if (HPDF_Stream_WriteToStream(bmp_data, image->stream, 0, NULL) != HPDF_OK)
        return NULL;

    if (image->stream->size != size) {
        HPDF_SetError(image->error, HPDF_INVALID_IMAGE, 0);
        return NULL;
    }
#endif
    return ret;
}