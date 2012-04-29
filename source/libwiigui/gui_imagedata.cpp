/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_imagedata.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

#include <libpng15/png.h>
#include "../_pnginfo.h"

static int fd = 0;

struct file_buffer_t {
    char name[256];
    unsigned char *data;
    long length;
    long offset;
};

struct pngMem {
    unsigned char *png_end;
    unsigned char *data;
    int size;
    int offset; //pour le parcours
};

static int offset = 0;

static void png_mem_read(png_structp png_ptr, png_bytep data, png_size_t length) {
    struct file_buffer_t *src = (struct file_buffer_t *) png_get_io_ptr(png_ptr);
    /* Copy data from image buffer */
    memcpy(data, src->data + src->offset, length);
    /* Advance in the file */
    src->offset += length;
}

/**
 * Constructor for the GuiImageData class.
 */
GuiImageData::GuiImageData(const u8 * i, int maxw, int maxh) {
    data = NULL;
    width = 0;
    height = 0;

    if (i){
        //data = DecodePNG(i, &width, &height, maxw, maxh);
        data = loadPNGFromMemory((unsigned char*)i);
        width = data->width;
        height = data->height;
    }
}

/**
 * Destructor for the GuiImageData class.
 */
GuiImageData::~GuiImageData() {
    if (data) {
        //free(data);
        Xe_DestroyTexture(g_pVideoDevice,data);
        data = NULL;
    }
}

XenosSurface * GuiImageData::GetImage() {
    return data;
}

int GuiImageData::GetWidth() {
    return width;
}

int GuiImageData::GetHeight() {
    return height;
}
