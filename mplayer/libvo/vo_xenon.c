#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
// libxenon miss
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <debug.h>
#include <stdio.h>
#include <usb/usbmain.h>
#include <ppc/timebase.h>
#include <xetypes.h>
#include <xenon_soc/xenon_power.h>
#include <xenon_smc/xenon_smc.h>

#include <sys/time.h>
#include <time/time.h>

#include <byteswap.h>

#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>

#include <limits.h>
// libxenon miss
#include <sys/time.h>
#include <time/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <time/time.h>
#include <xenon_smc/xenon_smc.h>
#include <console/console.h>
#include <diskio/ata.h>

#include <libfat/fat.h>

#include <math.h>

#include <debug.h>


#include "config.h"
#include "video_out.h"
#include "video_out_internal.h"

#include "fastmemcpy.h"

#include "libavutil/common.h"
#include "sub/font_load.h"
#include "sub/sub.h"


void mplayer_osd_close();
void mplayer_osd_draw(int level);
void mplayer_osd_open();

// Parameters:
//
//   sampler2D UTexture;
//   sampler2D VTexture;
//   sampler2D YTexture;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   YTexture     s0       1
//   UTexture     s1       1
//   VTexture     s2       1
static const uint32_t ps[] = {
        0x102a1100, 0x00000110, 0x000000c4, 0x00000000, 0x00000024, 0x000000c4,
        0x000000ec, 0x00000000, 0x00000000, 0x0000009c, 0x0000001c, 0x0000008d,
        0xffff0300, 0x00000003, 0x0000001c, 0x00000000, 0x00000086, 0x00000058,
        0x00030001, 0x00010000, 0x00000064, 0x00000000, 0x00000074, 0x00030002,
        0x00010000, 0x00000064, 0x00000000, 0x0000007d, 0x00030000, 0x00010000,
        0x00000064, 0x00000000, 0x55546578, 0x74757265, 0x00ababab, 0x0004000c,
        0x00010001, 0x00010000, 0x00000000, 0x56546578, 0x74757265, 0x00595465,
        0x78747572, 0x65007073, 0x5f335f30, 0x00322e30, 0x2e323033, 0x35332e30,
        0x00ababab, 0x00000000, 0x00000001, 0x00000000, 0x00000000, 0x00000014,
        0x01fc0010, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000040,
        0x00000084, 0x10000200, 0x00000004, 0x00000000, 0x00000821, 0x00010001,
        0x00000001, 0x00003050, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xbf5ee978, 0xbf8a76c8, 0x3fcc49ba, 0x400126e9, 0x3ec83127, 0x3f5020c5,
        0x3f94fdf4, 0x00000000, 0xbd94fdf4, 0xbe483127, 0xbed020c5, 0x00000000,
        0x00153002, 0x00001200, 0xc4000000, 0x00005005, 0x00002200, 0x00000000,
        0x10282001, 0x1f1ffff8, 0x00004000, 0x10182001, 0x1f1fffc7, 0x00004000,
        0x10082001, 0x1f1ffe3f, 0x00004000, 0xc8070000, 0x0062bec0, 0x8b02feff,
        0xc8030001, 0x00b01ab0, 0x8b02fdfd, 0xc88dc000, 0x00c6c6b0, 0xab02fe01,
        0x64800000, 0x00000061, 0xe2000000, 0xc8028000, 0x021bc600, 0xe0000000,
        0x00000000, 0x00000000, 0x00000000
};

static const uint32_t vs[] = {
        0x102a1101, 0x00000090, 0x00000060, 0x00000000, 0x00000024, 0x00000000,
        0x00000058, 0x00000000, 0x00000000, 0x00000030, 0x0000001c, 0x00000023,
        0xfffe0300, 0x00000000, 0x00000000, 0x00000000, 0x0000001c, 0x76735f33,
        0x5f300032, 0x2e302e32, 0x30333533, 0x2e3000ab, 0x00000000, 0x00000060,
        0x00010001, 0x00000000, 0x00000000, 0x00000821, 0x00000001, 0x00000002,
        0x00000001, 0x00000290, 0x00100003, 0x00305004, 0x00003050, 0x00001006,
        0x30052003, 0x00001200, 0xc2000000, 0x00001005, 0x00001200, 0xc4000000,
        0x00001006, 0x00002200, 0x00000000, 0x05f81000, 0x00000688, 0x00000000,
        0x05f80000, 0x00000fc8, 0x00000000, 0xc80f803e, 0x00000000, 0xe2010100,
        0xc8038000, 0x00b0b000, 0xe2000000, 0x00000000, 0x00000000, 0x00000000
};

// Parameters:
//
//   sampler2D YTexture;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   YTexture     s0       1
static const uint32_t ps_osd[] = {
        0x102a1100, 0x000000ac, 0x0000003c, 0x00000000, 0x00000024, 0x00000000,
        0x00000088, 0x00000000, 0x00000000, 0x00000060, 0x0000001c, 0x00000053,
        0xffff0300, 0x00000001, 0x0000001c, 0x00000000, 0x0000004c, 0x00000030,
        0x00030000, 0x00010000, 0x0000003c, 0x00000000, 0x59546578, 0x74757265,
        0x00ababab, 0x0004000c, 0x00010001, 0x00010000, 0x00000000, 0x70735f33,
        0x5f300032, 0x2e302e32, 0x30333533, 0x2e3000ab, 0x00000000, 0x0000003c,
        0x10000000, 0x00000004, 0x00000000, 0x00000821, 0x00010001, 0x00000001,
        0x00003050, 0x00011002, 0x00001200, 0xc4000000, 0x00001003, 0x00002200,
        0x00000000, 0x10080001, 0x1f1ff688, 0x00004000, 0xc80f8000, 0x00000000,
        0xe2000000, 0x00000000, 0x00000000, 0x00000000
};

static const vo_info_t info = {
        "Xenon video output",
        "xenon",
        "Ced2911",
        "#libxenon"
};

const LIBVO_EXTERN(xenon)


typedef struct AVSurface {
        struct XenosSurface * surface;
        void * data;
        uint32_t pitch;
} AVSurface;

typedef struct YUVSurface {
        AVSurface Y;
        AVSurface U;
        AVSurface V;
} YUVSurface;

static uint32_t max_width, max_height;

static uint32_t image_width, image_height;
static uint32_t osd_width, osd_height, osd_texture_width, osd_texture_height;

// used for aspect ratio
struct vo_rect src_rect;
struct vo_rect dst_rect;
struct vo_rect borders;

static struct XenosVertexBuffer *vb = NULL;
static struct XenosDevice * g_pVideoDevice = NULL;
static struct XenosShader * g_pVertexShader = NULL;
static struct XenosShader * g_pPixelTexturedShader = NULL;
static struct XenosShader * g_pPixeOsdShader = NULL;

static struct XenosDevice _xe;
static YUVSurface * g_pTexture = NULL;
static struct XenosSurface * g_pOsdSurf = NULL;

typedef struct verticeFormats {
        float x, y, z, w;
        float u, v;
} verticeFormats;

static int is_osd_populated = 0;

static YUVSurface * video_create_yuvsurf(int w, int h);
static void video_lock_yuvsurf(YUVSurface*);
static void video_unlock_yuvsurf(YUVSurface*);
static void video_delete_yuvsurf(YUVSurface *surf);

static YUVSurface * video_create_yuvsurf(int width, int height) {
        YUVSurface * surf = (YUVSurface *) malloc(sizeof (YUVSurface));
        surf->Y.surface = Xe_CreateTexture(g_pVideoDevice, width, height, 1, XE_FMT_8, 0);
        surf->U.surface = Xe_CreateTexture(g_pVideoDevice, width >> 1, height >> 1, 1, XE_FMT_8, 0);
        surf->V.surface = Xe_CreateTexture(g_pVideoDevice, width >> 1, height >> 1, 1, XE_FMT_8, 0);

        surf->Y.pitch = surf->Y.surface->wpitch;
        surf->U.pitch = surf->U.surface->wpitch;
        surf->V.pitch = surf->V.surface->wpitch;

        surf->Y.data = surf->Y.surface->base;
        surf->U.data = surf->U.surface->base;
        surf->V.data = surf->V.surface->base;

        return surf;
}

static void video_delete_yuvsurf(YUVSurface *surf) {
        if (surf) {
                TR;
                Xe_DestroyTexture(g_pVideoDevice, surf->Y.surface);
                Xe_DestroyTexture(g_pVideoDevice, surf->U.surface);
                Xe_DestroyTexture(g_pVideoDevice, surf->V.surface);
                TR;
                free(surf);
                TR;
                surf = NULL;
        }
}

static void video_lock_yuvsurf(YUVSurface* yuv) {
        if (yuv) {
                yuv->Y.data = (unsigned char *) Xe_Surface_LockRect(g_pVideoDevice, yuv->Y.surface, 0, 0, 0, 0, XE_LOCK_WRITE);
                yuv->U.data = (unsigned char *) Xe_Surface_LockRect(g_pVideoDevice, yuv->U.surface, 0, 0, 0, 0, XE_LOCK_WRITE);
                yuv->V.data = (unsigned char *) Xe_Surface_LockRect(g_pVideoDevice, yuv->V.surface, 0, 0, 0, 0, XE_LOCK_WRITE);

                yuv->Y.pitch = yuv->Y.surface->wpitch;
                yuv->U.pitch = yuv->U.surface->wpitch;
                yuv->V.pitch = yuv->V.surface->wpitch;
        }
}

static void video_unlock_yuvsurf(YUVSurface* yuv) {
        if (yuv) {
                Xe_Surface_Unlock(g_pVideoDevice, yuv->Y.surface);
                Xe_Surface_Unlock(g_pVideoDevice, yuv->U.surface);
                Xe_Surface_Unlock(g_pVideoDevice, yuv->V.surface);
        }
}

static void dump_rect(struct vo_rect * rect, const char * name) {
        printf("%s.bottom %d\n", name, rect->bottom);
        printf("%s.top %d\n", name, rect->top);
        printf("%s.left %d\n", name, rect->left);
        printf("%s.right %d\n", name, rect->right);
        printf("%s.width %d\n", name, rect->width);
        printf("%s.height  %d\n", name, rect->height);
}

/** @brief Calculate scaled fullscreen movie rectangle with
 *  preserved aspect ratio.
 */
static void calc_fs_rect(void) {
        calc_src_dst_rects(image_width, image_height, &src_rect, &dst_rect, &borders, NULL);

        dump_rect(&src_rect, "src_rect");
        dump_rect(&dst_rect, "dst_rect");
        dump_rect(&borders, "borders");
}

static int draw_slice(uint8_t *src[], int stride[], int w, int h, int x, int y) {
        char *dst; /**< Pointer to the destination image */

        if ((!g_pVideoDevice) || (g_pTexture == NULL))
                return 0;

        /* Copy Y */
        dst = (char *) g_pTexture->Y.data;
        dst = dst + g_pTexture->Y.pitch * y + x;
        memcpy_pic(dst, src[0], w, h, g_pTexture->Y.pitch, stride[0]);

        w /= 2;
        h /= 2;
        x /= 2;
        y /= 2;

        /* Copy U */
        dst = (char *) g_pTexture->U.data;
        dst = dst + g_pTexture->U.pitch * y + x;
        memcpy_pic(dst, src[1], w, h, g_pTexture->U.pitch, stride[1]);

        /* Copy V */
        dst = (char *) g_pTexture->V.data;
        dst = dst + g_pTexture->V.pitch * y + x;
        memcpy_pic(dst, src[2], w, h, g_pTexture->V.pitch, stride[2]);


        return 0; /* Success */
}

/** @brief Maps MPlayer alpha to D3D
 *         0x0 -> transparent and discarded by alpha test
 *         0x1 -> 0xFF to become opaque
 *         other alpha values are inverted +1 (2 = -2)
 *         These values are then inverted again with
                   the texture filter D3DBLEND_INVSRCALPHA
 */
static void vo_draw_alpha_a8(int w, int h, unsigned char* src,
        unsigned char *srca, int srcstride,
        unsigned char* dstbase, int dststride) {
        int y;
        for (y = 0; y < h; y++) {
                unsigned char *dst = (unsigned char*) dstbase;
                int x;
                for (x = 0; x < w; x++) {
                        dst[x] = src[x];
                }
                src += srcstride;
                srca += srcstride;
                dstbase += dststride;
        }
}

/** @brief Callback function to render the OSD to the texture
 */
static void draw_alpha(int x0, int y0, int w, int h, unsigned char *src,
        unsigned char *srca, int stride) {
        unsigned char * dst = (unsigned char *) Xe_Surface_LockRect(g_pVideoDevice, g_pOsdSurf, 0, 0, 0, 0, XE_LOCK_WRITE);

        vo_draw_alpha_a8(w, h, src, srca, stride,
                (unsigned char *) dst + g_pOsdSurf->wpitch * y0 + x0, g_pOsdSurf->wpitch);

        Xe_Surface_Unlock(g_pVideoDevice, g_pOsdSurf);

        is_osd_populated = 1;
}

static void draw_osd(void) {
        if (vo_osd_changed(0)) {
                // Clear osd
                memset(g_pOsdSurf->base, 0, g_pOsdSurf->wpitch * g_pOsdSurf->hpitch);

                vo_draw_text(g_pOsdSurf->width, g_pOsdSurf->height, draw_alpha);

/*
                vo_draw_text_ext(osd_width, osd_height,
                        borders.left, borders.top,
                        borders.left, borders.top,
                        image_width, image_height,
                        draw_alpha
                        );
*/
        }
}

static void ShowFPS(void) {
        static unsigned long lastTick = 0;
        static int frames = 0;
        unsigned long nowTick;
        frames++;
        nowTick = mftb() / (PPC_TIMEBASE_FREQ / 1000);
        if (lastTick + 1000 <= nowTick) {

                printf("%d fps\r\n", frames);

                frames = 0;
                lastTick = nowTick;
        }
}
extern int osd_level;
extern unsigned int osd_visible;
static int last_osd_level = 0;
extern int gui_input_use;

static void flip_page(void) {

        if (g_pTexture == NULL)
                return;

        ShowFPS();

        // Sync gpu
        Xe_Sync(g_pVideoDevice);

        // vsync - take care slow down video ... 
        if (vo_vsync)
                while (!Xe_IsVBlank(g_pVideoDevice));

        // refresh texture cache
        video_lock_yuvsurf(g_pTexture);
        video_unlock_yuvsurf(g_pTexture);

        // Reset states
        Xe_InvalidateState(g_pVideoDevice);
        Xe_SetClearColor(g_pVideoDevice, 0xFF000000);

        Xe_SetBlendOp(g_pVideoDevice, XE_BLENDOP_ADD);
        Xe_SetSrcBlend(g_pVideoDevice, XE_BLEND_ONE);
        Xe_SetDestBlend(g_pVideoDevice, XE_BLEND_INVSRCALPHA);
        Xe_SetAlphaFunc(g_pVideoDevice, XE_CMP_GREATER);
        Xe_SetAlphaTestEnable(g_pVideoDevice, 1);

        // Select stream and shaders
        Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
        Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, 10);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelTexturedShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

        // select texture
        Xe_SetTexture(g_pVideoDevice, 0, g_pTexture->Y.surface);
        Xe_SetTexture(g_pVideoDevice, 1, g_pTexture->U.surface);
        Xe_SetTexture(g_pVideoDevice, 2, g_pTexture->V.surface);

        // Draw
        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_RECTLIST, 0, 1);


        // Draw osd
        if (is_osd_populated) {
                Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixeOsdShader, 0);
                Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

                Xe_SetTexture(g_pVideoDevice, 0, g_pOsdSurf);
                Xe_SetStreamSource(g_pVideoDevice, 0, vb, 3 * sizeof (verticeFormats), 10);
                Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_RECTLIST, 0, 1);
        }

        if (osd_level) {
                // display always
                if (osd_level >= 2)
                        mplayer_osd_draw(osd_level);
                else {
                        // only display for a number of frame
                        if (osd_visible)
                                mplayer_osd_draw(2);
                }
        }

        if (osd_level == 3) {
                gui_input_use = 1;
        } else {
                gui_input_use = 0;
        }

        // Resolve
        Xe_Resolve(g_pVideoDevice);
        // Render in background
        Xe_Execute(g_pVideoDevice);
        //Xe_Sync(g_pVideoDevice);

}

static int draw_frame(uint8_t *src[]) {
        TR;
        return 0;
}

static int query_format(uint32_t format) {
        if (format != IMGFMT_YV12)
                return 0;
        return (VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW
                | VFCAP_OSD | VFCAP_HWSCALE_UP | VFCAP_HWSCALE_DOWN);
}

static void create_xenon_texture() {
        g_pTexture = video_create_yuvsurf(image_width, image_height);
        g_pOsdSurf = Xe_CreateTexture(g_pVideoDevice, osd_texture_width, osd_texture_height, 1, XE_FMT_8, 0);

        memset(g_pOsdSurf->base, 0, g_pOsdSurf->wpitch * g_pOsdSurf->hpitch);
}

static void destroy_xenon_texture() {
        if (g_pTexture)
                video_delete_yuvsurf(g_pTexture);
        if (g_pOsdSurf)
                Xe_DestroyTexture(g_pVideoDevice, g_pOsdSurf);
}

static void vo_xenon_fullscreen() {
        vo_fs = !vo_fs;
}

static void update_vb() {

        float screenAspectRatio = (float) max_width / max_height;
        float videoAspectRatio = (float) image_width / image_height;

        printf("screenAspectRatio : %f\r\n", screenAspectRatio);
        printf("videoAspectRatio : %f\r\n", videoAspectRatio);

        verticeFormats Rect[6];


        if (!vo_fs)
        {

                if (screenAspectRatio > videoAspectRatio) {
                        //float w = (float)screenAspectRatio/videoAspectRatio;
                        float w = (float) videoAspectRatio / screenAspectRatio;
                        // scale the w
                        Rect[0].x = -w;
                        Rect[0].y = 1;
                        Rect[0].u = 0;
                        Rect[0].v = 0;
                        Rect[0].z = 0.0;
                        Rect[0].w = 1.0;

                        // bottom left
                        Rect[1].x = -w;
                        Rect[1].y = -1;
                        Rect[1].u = 0;
                        Rect[1].v = 1;
                        Rect[1].z = 0.0;
                        Rect[1].w = 1.0;

                        // top right
                        Rect[2].x = w;
                        Rect[2].y = 1;
                        Rect[2].u = 1;
                        Rect[2].v = 0;
                        Rect[2].z = 0.0;
                        Rect[2].w = 1.0;

                } else {
                        float h = (float) screenAspectRatio / videoAspectRatio;
                        // scale the h
                        Rect[0].x = -1;
                        Rect[0].y = h;
                        Rect[0].u = 0;
                        Rect[0].v = 0;
                        Rect[0].z = 0.0;
                        Rect[0].w = 1.0;

                        // bottom left
                        Rect[1].x = -1;
                        Rect[1].y = -h;
                        Rect[1].u = 0;
                        Rect[1].v = 1;
                        Rect[1].z = 0.0;
                        Rect[1].w = 1.0;

                        // top right
                        Rect[2].x = 1;
                        Rect[2].y = h;
                        Rect[2].u = 1;
                        Rect[2].v = 0;
                        Rect[2].z = 0.0;
                        Rect[2].w = 1.0;
                }
        } else {
                // scale the w
                Rect[0].x = -1;
                Rect[0].y = 1;
                Rect[0].u = 0;
                Rect[0].v = 0;
                Rect[0].z = 0.0;
                Rect[0].w = 1.0;

                // bottom left
                Rect[1].x = -1;
                Rect[1].y = -1;
                Rect[1].u = 0;
                Rect[1].v = 1;
                Rect[1].z = 0.0;
                Rect[1].w = 1.0;

                // top right
                Rect[2].x = 1;
                Rect[2].y = 1;
                Rect[2].u = 1;
                Rect[2].v = 0;
                Rect[2].z = 0.0;
                Rect[2].w = 1.0;
        }

        // OSD
        // scale the w
        Rect[3].x = -1;
        Rect[3].y = 1;
        Rect[3].u = 0;
        Rect[3].v = 0;
        Rect[3].z = 0.0;
        Rect[3].w = 1.0;

        // bottom left
        Rect[4].x = -1;
        Rect[4].y = -1;
        Rect[4].u = 0;
        Rect[4].v = 1;
        Rect[4].z = 0.0;
        Rect[4].w = 1.0;

        // top right
        Rect[5].x = 1;
        Rect[5].y = 1;
        Rect[5].u = 1;
        Rect[5].v = 0;
        Rect[5].z = 0.0;
        Rect[5].w = 1.0;

        void *v = Xe_VB_Lock(g_pVideoDevice, vb, 0, 4096, XE_LOCK_WRITE);
        memcpy(v, Rect, 6 * sizeof (verticeFormats));
        Xe_VB_Unlock(g_pVideoDevice, vb);
}

static int config(uint32_t width, uint32_t height, uint32_t d_width, uint32_t d_height, uint32_t flags, char *title, uint32_t format) {
        image_width = width;
        image_height = height;

        osd_width = 1280;
        osd_height = 720;

        osd_texture_width = 1280;
        osd_texture_height = 720;
        
        // Destroy surface
        destroy_xenon_texture();

        // Create surface
        create_xenon_texture();

        printf("width:%d\r\n", width);
        printf("height:%d\r\n", height);

        printf("osd_width:%d\r\n", osd_width);
        printf("osd_height:%d\r\n", osd_height);


        printf("d_width:%d\r\n", d_width);
        printf("d_height:%d\r\n", d_height);

        printf("osd_texture_width:%d\r\n", osd_texture_width);
        printf("osd_texture_height:%d\r\n", osd_texture_height);

        printf("flags:%08x\r\n", flags);
        printf("format:%08x\r\n", format);

        // update vb with correct aspect ratio
        calc_fs_rect();

        vo_fs = (flags & VOFLAG_FULLSCREEN);
        update_vb();
        
        mplayer_osd_open();
        return 0;
}

static void uninit(void) {
        mplayer_osd_close();
}

static void check_events(void) {
        //TR;
}

// source/video.c
extern struct XenosDevice * GetVideoDevice();
struct XenosVertexBuffer * GetSharedVertexBuffer();

static int preinit(const char *arg) {
        struct XenosVBFFormat vbf = {
                2,
                {
                        {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
                        {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
                }
        };

        struct XenosSurface * fb = NULL;

        //g_pVideoDevice = &_xe;

        //Xe_Init(g_pVideoDevice);

        g_pVideoDevice = GetVideoDevice();

        fb = Xe_GetFramebufferSurface(g_pVideoDevice);

        max_width = fb->width;
        max_height = fb->height;

        Xe_SetRenderTarget(g_pVideoDevice, fb);

        /* create shaders */
        if (g_pPixelTexturedShader == NULL) {
                g_pPixelTexturedShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) ps);
                Xe_InstantiateShader(g_pVideoDevice, g_pPixelTexturedShader, 0);
        }

        if (g_pPixeOsdShader == NULL) {
                g_pPixeOsdShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) ps_osd);
                Xe_InstantiateShader(g_pVideoDevice, g_pPixeOsdShader, 0);
        }

        if (g_pVertexShader == NULL) {
                g_pVertexShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) vs);
                Xe_InstantiateShader(g_pVideoDevice, g_pVertexShader, 0);
                Xe_ShaderApplyVFetchPatches(g_pVideoDevice, g_pVertexShader, 0, &vbf);
        }

        //edram_init(g_pVideoDevice);

        /* create vb */
        //vb = Xe_CreateVertexBuffer(g_pVideoDevice, 6 * sizeof (verticeFormats));
        vb = GetSharedVertexBuffer();

        update_vb();

        Xe_SetClearColor(g_pVideoDevice, 0);

        return 0;
}

static int control(uint32_t request, void *data) {
        switch (request) {
                case VOCTRL_GET_IMAGE: /* Direct Rendering. Not implemented yet. */
                        return VO_NOTIMPL;
                case VOCTRL_QUERY_FORMAT:
                        return query_format(*((uint32_t*) data));
                case VOCTRL_FULLSCREEN:
                        vo_xenon_fullscreen();
                        update_vb();
                        return VO_TRUE;
        }
        return VO_NOTIMPL;
}
