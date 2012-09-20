#pragma once

#include "mplayer_func.h"

// mplayer stuff
extern "C" {
	#include "../mplayer/mplayer.h"
	#include "../mplayer/sub/sub.h"
	#include "../mplayer/sub/font_load.h"
	#include "../mplayer/mpcommon.h"
	#include "../mplayer/libvo/video_out.h"
	extern float soft_vol_max;
	extern float start_volume;
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

// menu_osd.cpp
void loadOsdRessources();
// menu.cpp
double playerSeekPrompt(char * seekfile);
xmplayer_seek_information * playerSeekOpen(char * file);
void format_time(char * dest, double time);
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label);
