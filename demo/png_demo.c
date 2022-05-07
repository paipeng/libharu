/*
 * << Haru Free PDF Library 2.0.0 >> -- png_demo.c
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"

#ifndef HPDF_NOPNGLIB

jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

void
draw_image (HPDF_Doc     pdf,
            const char  *filename,
            float        x,
            float        y,
            float       dpi,
            const char  *text)
{
#ifdef __WIN32__
    const char* FILE_SEPARATOR = "\\";
#else
    const char* FILE_SEPARATOR = "/";
#endif
    char filename1[255];
    char print_text[256];

    HPDF_Page page = HPDF_GetCurrentPage (pdf);
    HPDF_Image image;

    strcpy(filename1, "C:\\pngsuite");
    strcat(filename1, FILE_SEPARATOR);
    strcat(filename1, filename);

    if (strstr(filename, ".bmp")) {
        image = HPDF_LoadBmpImageFromFile(pdf, filename1);
    } else {
        image = HPDF_LoadPngImageFromFile(pdf, filename1);
    }


    snprintf(print_text, sizeof(char) * 256, "%s (%.0f dpi)", text, dpi);
    /* Draw image to the canvas. */
    if (strstr(filename, ".bmp")) {
        HPDF_Page_DrawImage(page, image, x, y, HPDF_Image_GetWidth(image)*72.0/dpi,
            HPDF_Image_GetHeight(image)*72.0/dpi);
    }
    else {
#if 0
        HPDF_Page_DrawImage(page, image, x, y, HPDF_Image_GetWidth(image),
            HPDF_Image_GetHeight(image));
#endif
    }
    /* Print the text. */
    HPDF_Page_BeginText (page);
    HPDF_Page_SetTextLeading (page, 16);
    HPDF_Page_MoveTextPos (page, x, y);
    HPDF_Page_ShowTextNextLine (page, filename);
    HPDF_Page_ShowTextNextLine (page, print_text);
    HPDF_Page_EndText (page);
}


int main (int argc, char **argv)
{
    HPDF_Doc  pdf;
    HPDF_Font font;
    HPDF_Page page;
    char fname[256];
    HPDF_Destination dst;
    float dpi;

    strcpy (fname, argv[0]);
    strcat (fname, ".pdf");

    pdf = HPDF_New (error_handler, NULL);
    if (!pdf) {
        printf ("error: cannot create PdfDoc object\n");
        return 1;
    }

    /* error-handler */
    if (setjmp(env)) {
        HPDF_Free (pdf);
        return 1;
    }

    HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    /* create default-font */
    font = HPDF_GetFont (pdf, "Helvetica", NULL);

    /* add a new page object. */
    page = HPDF_AddPage (pdf);
#if 0
    HPDF_Page_SetWidth (page, 550);
    HPDF_Page_SetHeight (page, 650);
#else
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
#endif
    double pageWidth = HPDF_Page_GetWidth(page);
    double pageHeight = HPDF_Page_GetHeight(page);
    printf("pdf A4 page size: %f-%f\n", pageWidth, pageHeight);
    dst = HPDF_Page_CreateDestination (page);
    HPDF_Destination_SetXYZ (dst, 0, HPDF_Page_GetHeight (page), 1);
    HPDF_SetOpenAction(pdf, dst);

    HPDF_Page_BeginText (page);
    HPDF_Page_SetFontAndSize (page, font, 20);
    HPDF_Page_MoveTextPos (page, 220, HPDF_Page_GetHeight (page) - 70);
    HPDF_Page_ShowText (page, "Code Test");
    HPDF_Page_EndText (page);

    HPDF_Page_SetFontAndSize (page, font, 12);

    dpi = 600;
    draw_image(pdf, "code_1bit.bmp", 100, HPDF_Page_GetHeight(page) - 550, dpi,
        "1bit");

    draw_image(pdf, "code_rgb.bmp", 200, HPDF_Page_GetHeight(page) - 550, dpi,
        "rgb");
    draw_image(pdf, "code_gray.bmp", 300, HPDF_Page_GetHeight(page) - 550, dpi,
        "gray");

    dpi = 1200;
    draw_image(pdf, "code_1bit_2x.bmp", 100, HPDF_Page_GetHeight(page) - 450, dpi,
        "1bit 2x");

    draw_image(pdf, "code_rgb_2x.bmp", 200, HPDF_Page_GetHeight(page) - 450, dpi,
        "rgb 2x");
    draw_image(pdf, "code_gray_2x.bmp", 300, HPDF_Page_GetHeight(page) - 450, dpi,
        "gray 2x");

    dpi = 600;
    draw_image(pdf, "code_1bit_2x.bmp", 100, HPDF_Page_GetHeight(page) - 350, dpi,
        "1bit 2x");

    draw_image(pdf, "code_rgb_2x.bmp", 200, HPDF_Page_GetHeight(page) - 350, dpi,
        "rgb 2x");
    draw_image(pdf, "code_gray_2x.bmp", 300, HPDF_Page_GetHeight(page) - 350, dpi,
        "gray 2x");


    dpi = 300;
    draw_image(pdf, "code_1bit.bmp", 100, HPDF_Page_GetHeight(page) - 250, dpi,
        "1bit");

    draw_image(pdf, "code_rgb.bmp", 200, HPDF_Page_GetHeight(page) - 250, dpi,
        "rgb");
    draw_image(pdf, "code_gray.bmp", 300, HPDF_Page_GetHeight(page) - 250, dpi,
        "gray");
#if 0
    draw_image (pdf, "basn0g01.png", 100, HPDF_Page_GetHeight (page) - 150,
                "1bit grayscale.");
    draw_image (pdf, "basn0g02.png", 200, HPDF_Page_GetHeight (page) - 150,
                "2bit grayscale.");
    draw_image (pdf, "basn0g04.png", 300, HPDF_Page_GetHeight (page) - 150,
                "4bit grayscale.");
    draw_image (pdf, "basn0g08.png", 400, HPDF_Page_GetHeight (page) - 150,
                "8bit grayscale.");

    draw_image (pdf, "basn2c08.png", 100, HPDF_Page_GetHeight (page) - 250,
                "8bit color.");
    draw_image (pdf, "basn2c16.png", 200, HPDF_Page_GetHeight (page) - 250,
                "16bit color.");

    draw_image (pdf, "basn3p01.png", 100, HPDF_Page_GetHeight (page) - 350,
                "1bit pallet.");
    draw_image (pdf, "basn3p02.png", 200, HPDF_Page_GetHeight (page) - 350,
                "2bit pallet.");
    draw_image (pdf, "basn3p04.png", 300, HPDF_Page_GetHeight (page) - 350,
                "4bit pallet.");
    draw_image (pdf, "basn3p08.png", 400, HPDF_Page_GetHeight (page) - 350,
                "8bit pallet.");

    draw_image (pdf, "basn4a08.png", 100, HPDF_Page_GetHeight (page) - 450,
                "8bit alpha.");
    draw_image (pdf, "basn4a16.png", 200, HPDF_Page_GetHeight (page) - 450,
                "16bit alpha.");

    draw_image (pdf, "basn6a08.png", 100, HPDF_Page_GetHeight (page) - 550,
                "8bit alpha.");
    draw_image (pdf, "basn6a16.png", 200, HPDF_Page_GetHeight (page) - 550,
                "16bit alpha.");
#endif
    /* save the document to a file */
    HPDF_SaveToFile (pdf, fname);

    /* clean up */
    HPDF_Free (pdf);

    return 0;
}

#else /* HPDF_NOPNGLIB */

int main()
{
    printf("WARNING: if you want to run this demo, \n"
           "make libhpdf with HPDF_USE_PNGLIB option.\n");
    return 0;
}

#endif /* HPDF_NOPNGLIB */

