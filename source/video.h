/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * Modified by Ced2911, 2011
 *
 * video.h
 * Video routines
 ***************************************************************************/

#ifndef _VIDEO_H_
#define _VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif
	
#include <xetypes.h>
#include <xenos/xe.h>

typedef union {
    struct {
        unsigned char a;
        unsigned char b;
        unsigned char g;
        unsigned char r;
    };
    unsigned int lcol;
} XeColor;

extern struct XenosDevice * g_pVideoDevice;

struct XenosDevice * GetVideoDevice();
struct XenosVertexBuffer * GetSharedVertexBuffer();

void InitVideo();
void ResetVideo_Menu();

void Menu_Frame();
void Menu_Render();
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, struct XenosSurface * data, f32 degrees, f32 scaleX, f32 scaleY, u8 alphaF);
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, XeColor color, u8 filled);
void Menu_T(struct XenosSurface * surf, f32 texWidth, f32 texHeight, int16_t screenX, int16_t screenY, XeColor color);

extern int screenheight;
extern int screenwidth;
extern u32 FrameTimer;

struct XenosSurface *loadPNGFromMemory(unsigned char *PNGdata);

#ifdef __cplusplus
}
#endif
#endif
