#include <stdio.h>
#include <malloc.h>
#include <xenos/xenos.h>
#include <xenos/xe.h>

#include <libpng15/png.h>
#include "_pnginfo.h"

extern struct XenosDevice * g_pVideoDevice;

int fd = 0;

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

int offset = 0;

static void png_mem_read(png_structp png_ptr, png_bytep data, png_size_t length) {
    struct file_buffer_t *src = (struct file_buffer_t *) png_get_io_ptr(png_ptr);
    /* Copy data from image buffer */
    memcpy(data, src->data + src->offset, length);
    /* Advance in the file */
    src->offset += length;
}

//Lits un fichier png en mémoire

struct XenosSurface *loadPNGFromMemory(unsigned char *PNGdata) {
    int y = 0;
    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    //        int number_of_passes;
    png_bytep * row_pointers;

    offset = 0;

    struct file_buffer_t *file;
    file = (struct file_buffer_t *) malloc(sizeof (struct file_buffer_t));
    file->length = 1024 * 1024 * 5;
    file->data = (unsigned char *) malloc(file->length); //5mo ...
    file->offset = 0;
    memcpy(file->data, PNGdata, file->length);

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
        printf("[read_png_file] png_create_read_struct failed\n");
        return 0;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("[read_png_file] png_create_info_struct failed\n");
        return 0;
    }

    png_set_read_fn(png_ptr, (png_voidp *) file, png_mem_read); //permet de lire à  partir de pngfile buff

    //png_set_sig_bytes(png_ptr, 8);//on avance de 8 ?

    png_read_info(png_ptr, info_ptr);

    width = info_ptr->width;
    height = info_ptr->height;
    color_type = info_ptr->color_type;
    bit_depth = info_ptr->bit_depth;

    //        number_of_passes = png_set_interlace_handling(png_ptr);


    if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
        printf("no support :( \n bit_depth = %08x\n", bit_depth);
        return 0;
    }

    if (color_type == PNG_COLOR_TYPE_RGB)
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_BEFORE);

    png_set_swap_alpha(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    //On créer la surface
    struct XenosSurface *surface = Xe_CreateTexture(g_pVideoDevice, width, height, 1, XE_FMT_8888 | XE_FMT_ARGB, 0);

    uint8_t *data = (uint8_t*) Xe_Surface_LockRect(g_pVideoDevice, surface, 0, 0, 0, 0, XE_LOCK_WRITE);

    row_pointers = (png_bytep*) malloc(sizeof (png_bytep) * surface->height);
    for (y = 0; y < surface->height; y++)
        row_pointers[y] = data + surface->wpitch * y;

    png_read_image(png_ptr, row_pointers);


    Xe_Surface_Unlock(g_pVideoDevice, surface);

    free(file->data);
    free(file);
    free(row_pointers);

    return surface;
}

int LoadFile(const char* strFileName, void** ppFileData, unsigned int * pdwFileSize) {
    if (pdwFileSize)
        *pdwFileSize = 0L;

    //usb_do_poll();

    printf("LoadFile %s\r\n", strFileName);

    // Open the file for reading
    FILE * fd = fopen(strFileName, "rb");

    if (fd == NULL) {
        printf("LoadFile : file %s not found\r\n", strFileName);
        return -2;
    }


    // Seek to end to get filesize
    fseek(fd, 0, SEEK_END);

    // Get the filesize
    unsigned int dwFileSize = ftell(fd);

    // Seek to start
    fseek(fd, 0, SEEK_SET);

    void * pFileData = malloc(dwFileSize);

    if (pFileData == NULL) {
        fclose(fd);
        return -1;
    }
    unsigned int dwBytesRead = 0;
    dwBytesRead = fread(pFileData, 1, dwFileSize, fd);

    printf("dwBytesRead = %d\r\n", dwBytesRead);

    // Finished reading file
    fclose(fd);

    if (pdwFileSize)
        *pdwFileSize = dwFileSize;

    *ppFileData = pFileData;
    return 0;
}

int LoadTextureFromFile(char * pSrcFile, struct XenosSurface **ppTexture) {
    struct XenosSurface * dest = NULL;
    
    unsigned char * PNGdata = NULL;
    unsigned int size = 0;
    LoadFile(pSrcFile, (void**) &PNGdata, &size);
    if (PNGdata != NULL) {
        dest = loadPNGFromMemory(PNGdata);
        ppTexture[0]=dest;
    }
    else{
        printf("Error reading %s\r\n", pSrcFile);
    }
    free(PNGdata);
    
    return dest==NULL?1:0;
}