#pragma once

#include "mplayer_func.h"
#include <string>
// mplayer stuff
extern "C" {
	#include "../mplayer/mplayer.h"
	#include "../mplayer/sub/sub.h"
	#include "../mplayer/sub/font_load.h"
	#include "../mplayer/mpcommon.h"
	#include "../mplayer/libvo/video_out.h"
	extern float soft_vol_max;
	extern float start_volume;
	extern float vo_xenon_subtitle[4];
	extern float vo_xenon_outline[4];
}

// fucked by include path
extern "C" {
	#include <stdint.h>

	struct controller_data_s
	{
		signed short s1_x, s1_y, s2_x, s2_y;
		int s1_z, s2_z, lb, rb, start, back, a, b, x, y, up, down, left, right;
		unsigned char lt, rt;
		int logo;
	};

	int get_controller_data(struct controller_data_s *d, int port);

	void set_controller_data(int port, const struct controller_data_s *d);

	void set_controller_rumble(int port, uint8_t l, uint8_t r);
}

// mplayer_func.c
extern "C" {
	extern char MPLAYER_DATADIR[100]; 
	extern char MPLAYER_CONFDIR[100]; 
	extern char MPLAYER_LIBDIR[100];
}


enum
{
	MENU_BACK = -1,
	HOME_PAGE = 1,
	MENU_MPLAYER,
	MENU_ELF,
	BROWSE = 0x10,
	BROWSE_VIDEO,
	BROWSE_AUDIO,
	BROWSE_PICTURE,
	BROWSE_ALL,
	SETTINGS,
	SETTINGS_GLOBAL,
	SETTINGS_AUDIO,		
	SETTINGS_VIDEO,	
	SETTINGS_SUBTITLES,	
	SETTINGS_NETWORK,
	SETTINGS_NETWORK_SMB,
	OSD = 0x20,
};


typedef struct _cp {
	const char *cpname;
	const char *language;
} CP;

typedef struct _lang {
	const char *language;
	const char *abbrev;
	const char *abbrev2;
} LANG;

#define LANGUAGE_SIZE 135
#define CODEPAGE_SIZE 27

extern LANG languages[LANGUAGE_SIZE];
extern CP codepages[CODEPAGE_SIZE];

int GetAudioLangIndex();
int GetSubLangIndex();
int GetCodepageIndex();

/** 
 * Color 
 **/
typedef struct
{
        //RRGGBB00
        unsigned int hex;
        char * string;
       
 /*       void fromFloat(float * v) {
                hex = 0;
                hex = (v[0] * 255.f) | (v[1] * 255.f) >> 8 | (v[2] * 255.f)>> 16 | (v[3] * 255.f) >> 24;
        } */
       
        void toFloat(float *v) {
                const float col[4] = {
                        (float)((hex & 0xFF)) * (1.0f / 255.0f),
                        (float)((hex & 0xFF00) >> 8) * (1.0f / 255.0f),
                        (float)((hex & 0xFF0000) >> 16) * (1.0f / 255.0f),
                        (float)((hex & 0xFF0000000) >> 24) * (1.0f / 255.0f),
                };
               
                memcpy(v, col, 4 * sizeof(float));
        }
} color;

#define NB_COLOR 4

extern color colors[NB_COLOR];

char * getColorFromHex(unsigned int hex, color * pColor, int max);
unsigned int getColorFromString(char * str, color * pColor, int max);
int getColorIndex(unsigned int hex, color * pColor, int max);


// menu_osd.cpp
void LoadOsdRessources();
// menu.cpp
double playerSeekPrompt(char * seekfile);
xmplayer_seek_information * playerSeekOpen(char * file);
void format_time(char * dest, double time);
int WindowPrompt(std::string title, std::string msg, std::string btn1Label, std::string btn2Label);
int SmallWindowPrompt(std::string btn1Label, std::string btn2Label);
