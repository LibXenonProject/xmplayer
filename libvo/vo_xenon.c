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
static const uint32_t ps_osd[] ={
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

static uint32_t image_width, image_height, osd_width, osd_height;

static struct XenosVertexBuffer *vb = NULL;
static struct XenosVertexBuffer *vb_osd = NULL;
static struct XenosDevice * g_pVideoDevice = NULL;
static struct XenosShader * g_pVertexShader = NULL;
static struct XenosShader * g_pPixelTexturedShader = NULL;
static struct XenosShader * g_pPixeOsdShader = NULL;

static struct XenosDevice _xe;
static YUVSurface * g_pTexture = NULL;
static struct XenosSurface * g_pOsdSurf=NULL;

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
		Xe_DestroyTexture(g_pVideoDevice, surf->Y.surface);
		Xe_DestroyTexture(g_pVideoDevice, surf->U.surface);
		Xe_DestroyTexture(g_pVideoDevice, surf->V.surface);

		free(surf);
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
static void vo_draw_alpha_l8a8(int w, int h, unsigned char* src,
                               unsigned char *srca, int srcstride,
                               unsigned char* dstbase, int dststride)
{
    int y;
    for (y = 0; y < h; y++) {
        unsigned short *dst = (unsigned short*)dstbase;
        int x;
        for (x = 0; x < w; x++) {
            dst[x] = (-srca[x] << 8) | src[x];
        }
        src     += srcstride;
        srca    += srcstride;
        dstbase += dststride;
    }	
}

/** @brief Callback function to render the OSD to the texture
 */
static void draw_alpha(int x0, int y0, int w, int h, unsigned char *src,
                       unsigned char *srca, int stride)
{
	unsigned char * dst = (unsigned char *)Xe_Surface_LockRect(g_pVideoDevice,g_pOsdSurf, 0, 0, 0, 0, XE_LOCK_WRITE);

    vo_draw_alpha_l8a8(w, h, src, srca, stride,
        (unsigned char *)dst+ g_pOsdSurf->wpitch*y0 + 2*x0, g_pOsdSurf->wpitch);

	Xe_Surface_Unlock(g_pVideoDevice,g_pOsdSurf);

    is_osd_populated = 1;
}

static void draw_osd(void) {
	//TR
	//osd_has_changed = vo_osd_changed(0);
	int border_x=0,border_y=0;
	int src_width=image_width,src_height=image_height;
	
	
	if (vo_osd_changed(0)) 
	{
		memset(g_pOsdSurf->base,0,g_pOsdSurf->wpitch*g_pOsdSurf->hpitch);
	}
	vo_draw_text_ext(osd_width, osd_height, border_x, border_y,
                         border_x, border_y, src_width, src_height, draw_alpha);
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

static void flip_page(void) {
	if (g_pTexture == NULL)
		return;
	
	//while (!Xe_IsVBlank(g_pVideoDevice));
	// Sync gpu
	Xe_Sync(g_pVideoDevice);
	
	// refresh texture cache
	video_lock_yuvsurf(g_pTexture);
	video_unlock_yuvsurf(g_pTexture);

	//ShowFPS();
	// Reset states
	Xe_InvalidateState(g_pVideoDevice);
	Xe_SetClearColor(g_pVideoDevice, 0x88888888);
	
	Xe_SetBlendOp(g_pVideoDevice, XE_BLENDOP_ADD);
    Xe_SetSrcBlend(g_pVideoDevice, XE_BLEND_SRCALPHA);
    Xe_SetDestBlend(g_pVideoDevice, XE_BLEND_INVSRCALPHA);
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
	
	
	if(is_osd_populated)
	{
		Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixeOsdShader, 0);
		Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);
		
		Xe_SetTexture(g_pVideoDevice, 0, g_pOsdSurf);
		
		Xe_SetStreamSource(g_pVideoDevice, 0, vb_osd, 0, 10);
		
		Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_RECTLIST, 0, 1);
	}

	// Resolve
	Xe_Resolve(g_pVideoDevice);
	// Render in background
	Xe_Execute(g_pVideoDevice);
}

static int
draw_frame(uint8_t *src[]) {
	TR;
	return 0;
}

static int
query_format(uint32_t format) {
	if (IMGFMT_IS_HWACCEL(format))
		return 0;
	return VFCAP_CSP_SUPPORTED;
}

static int config(uint32_t width, uint32_t height, uint32_t d_width, uint32_t d_height, uint32_t flags, char *title, uint32_t format) {
	TR;
	image_width = width;
	image_height = height;
	
	osd_width = d_width;
	osd_height = d_height;

	if (g_pTexture)
		video_delete_yuvsurf(g_pTexture);
	
	if(g_pOsdSurf)
		Xe_DestroyTexture(g_pVideoDevice,g_pOsdSurf);

	g_pTexture = video_create_yuvsurf(width, height);
	g_pOsdSurf = Xe_CreateTexture(g_pVideoDevice,osd_width,osd_height,1, XE_FMT_8, 0);
	
	memset(g_pOsdSurf->base,0,g_pOsdSurf->wpitch*g_pOsdSurf->hpitch);

	printf("width:%d\r\n", width);
	printf("height:%d\r\n", height);
	printf("d_width:%d\r\n", d_width);
	printf("d_height:%d\r\n", d_height);
	printf("flags:%08x\r\n", flags);
	printf("format:%08x\r\n", format);

	return 0;
}

static void
uninit(void) {
}

static void check_events(void) {
}

static int preinit(const char *arg) {
	struct XenosVBFFormat vbf = {
		2,
		{
			{XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
			{XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
		}
	};

	verticeFormats *Rect = NULL;
	verticeFormats *Rect_osd = NULL;

	g_pVideoDevice = &_xe;
	Xe_Init(g_pVideoDevice);

	Xe_SetRenderTarget(g_pVideoDevice, Xe_GetFramebufferSurface(g_pVideoDevice));

	/* create shaders */
	g_pPixelTexturedShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) ps);
	Xe_InstantiateShader(g_pVideoDevice, g_pPixelTexturedShader, 0);
	
	g_pPixeOsdShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) ps_osd);
	Xe_InstantiateShader(g_pVideoDevice, g_pPixeOsdShader, 0);

	g_pVertexShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) vs);
	Xe_InstantiateShader(g_pVideoDevice, g_pVertexShader, 0);
	Xe_ShaderApplyVFetchPatches(g_pVideoDevice, g_pVertexShader, 0, &vbf);

	edram_init(g_pVideoDevice);

	/* create vb */
	vb = Xe_CreateVertexBuffer(g_pVideoDevice, 3 * sizeof (verticeFormats));
	Rect = Xe_VB_Lock(g_pVideoDevice, vb, 0, 3 * sizeof (verticeFormats), XE_LOCK_WRITE);
	vb_osd = Xe_CreateVertexBuffer(g_pVideoDevice, 3 * sizeof (verticeFormats));
	Rect_osd = Xe_VB_Lock(g_pVideoDevice, vb_osd, 0, 3 * sizeof (verticeFormats), XE_LOCK_WRITE);
	{
		int i = 0;

		// top left
		Rect[0].x = -1;
		Rect[0].y = 1;
		Rect[0].u = 0;
		Rect[0].v = 0;

		// bottom left
		Rect[1].x = -1;
		Rect[1].y = -1;
		Rect[1].u = 0;
		Rect[1].v = 1;

		// top right
		Rect[2].x = 1;
		Rect[2].y = 1;
		Rect[2].u = 1;
		Rect[2].v = 0;

		for (i = 0; i < 3; i++) {
			Rect[i].z = 0.0;
			Rect[i].w = 1.0;
			
			Rect_osd[i].x=Rect[i].x;
			Rect_osd[i].y=Rect[i].y;
			Rect_osd[i].z=Rect[i].z;
			Rect_osd[i].w=Rect[i].w;
			
			Rect_osd[i].u=Rect[i].u;
			Rect_osd[i].v=Rect[i].v;
		}
	}
	Xe_VB_Unlock(g_pVideoDevice, vb);
	Xe_VB_Unlock(g_pVideoDevice, vb_osd);

	Xe_SetClearColor(g_pVideoDevice, 0);

	console_close();

	TR;


	return 0;
}

static int control(uint32_t request, void *data) {
	switch (request) {
		case VOCTRL_QUERY_FORMAT:
			return query_format(*((uint32_t*) data));
	}
	return VO_NOTIMPL;
}
