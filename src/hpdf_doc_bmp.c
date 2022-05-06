

#include "hpdf_conf.h"
#include "hpdf_utils.h"
#include "hpdf.h"
#include "hpdf_image.h"

static HPDF_Image
LoadBmpImageFromStream(HPDF_Doc pdf, HPDF_Stream imagedata, HPDF_BOOL delayed_loading);

HPDF_EXPORT(HPDF_Image)
HPDF_LoadBmpImageFromMem(HPDF_Doc pdf,  const HPDF_BYTE* buffer, HPDF_UINT size) {
    HPDF_Stream imagedata;
    HPDF_Image image;

    HPDF_PTRACE((" HPDF_LoadBmpImageFromMem\n"));

    if (!HPDF_HasDoc(pdf)) {
        return NULL;
    }

    /* create file stream */
    imagedata = HPDF_MemStream_New(pdf->mmgr, size);

    if (!HPDF_Stream_Validate(imagedata)) {
        HPDF_RaiseError(&pdf->error, HPDF_INVALID_STREAM, 0);
        return NULL;
    }

    if (HPDF_Stream_Write(imagedata, buffer, size) != HPDF_OK) {
        HPDF_Stream_Free(imagedata);
        return NULL;
    }

    image = LoadBmpImageFromStream(pdf, imagedata, HPDF_FALSE);

    /* destroy file stream */
    HPDF_Stream_Free(imagedata);

    if (!image) {
        HPDF_CheckError(&pdf->error);
    }

    return image;

}


HPDF_EXPORT(HPDF_Image)
HPDF_LoadBmpImageFromFile(HPDF_Doc pdf, const char* filename) {
    HPDF_Stream imagedata;
    HPDF_Image image;

    HPDF_PTRACE((" HPDF_LoadBmpImageFromFile\n"));

    if (!HPDF_HasDoc(pdf))
        return NULL;

    /* create file stream */
    imagedata = HPDF_FileReader_New(pdf->mmgr, filename);

    if (HPDF_Stream_Validate(imagedata))
        image = LoadBmpImageFromStream(pdf, imagedata, HPDF_FALSE);
    else
        image = NULL;

    /* destroy file stream */
    if (imagedata)
        HPDF_Stream_Free(imagedata);

    if (!image)
        HPDF_CheckError(&pdf->error);

    return image;
}

/* delaied loading version of HPDF_LoadPngImageFromFile */
HPDF_EXPORT(HPDF_Image)
HPDF_LoadBmpImageFromFile2(HPDF_Doc pdf, const char* filename) {
    HPDF_Stream imagedata;
    HPDF_Image image;
    HPDF_String fname;

    HPDF_PTRACE((" HPDF_LoadBmpImageFromFile2\n"));

    if (!HPDF_HasDoc(pdf))
        return NULL;

    /* check whether file name is valid or not. */
    imagedata = HPDF_FileReader_New(pdf->mmgr, filename);

    if (HPDF_Stream_Validate(imagedata))
        image = LoadBmpImageFromStream(pdf, imagedata, HPDF_TRUE);
    else
        image = NULL;

    /* destroy file stream */
    if (imagedata)
        HPDF_Stream_Free(imagedata);

    if (!image) {
        HPDF_CheckError(&pdf->error);
        return NULL;
    }

    /* add file-name to image dictionary as a hidden entry.
     * it is used when the image data is needed.
     */
    fname = HPDF_String_New(pdf->mmgr, filename, NULL);
    if (!fname) {
        HPDF_CheckError(&pdf->error);
        return NULL;
    }

    fname->header.obj_id |= HPDF_OTYPE_HIDDEN;

    if ((HPDF_Dict_Add(image, "_FILE_NAME", fname)) != HPDF_OK) {
        HPDF_CheckError(&pdf->error);
        return NULL;
    }

    return image;
}

#ifndef LIBHPDF_HAVE_NOPNGLIB
static HPDF_Image
LoadBmpImageFromStream(HPDF_Doc      pdf,
    HPDF_Stream   imagedata,
    HPDF_BOOL     delayed_loading)
{
    HPDF_Image image;

    HPDF_PTRACE((" HPDF_LoadBmpImageFromStream\n"));

    image = HPDF_Image_LoadBmpImage(pdf->mmgr, imagedata, pdf->xref,
        delayed_loading);

    if (image && (pdf->compression_mode & HPDF_COMP_IMAGE))
        image->filter = HPDF_STREAM_FILTER_FLATE_DECODE;

    return image;
}

#else
static HPDF_Image
LoadBmpImageFromStream(HPDF_Doc      pdf,
    HPDF_Stream   imagedata,
    HPDF_BOOL     delayed_loading)
{
    HPDF_SetError(&pdf->error, HPDF_UNSUPPORTED_FUNC, 0);
    HPDF_UNUSED(delayed_loading);
    HPDF_UNUSED(imagedata);

    return NULL;
}

#endif /* LIBHPDF_HAVE_NOPNGLIB */

