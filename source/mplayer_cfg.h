#pragma once

#define APPNAME "XMPlayer"
#define APPVERSION "0.0.1"
#define PREF_FILE_NAME 	"xmplayer.xml"

enum{
	LANG_ENGLISH,
	LANG_FRENCH,
	LANG_LENGTH
};

typedef struct {
	int language;
	int exit_action;
	int sort_order;
	char subcp[100];
	char subcp_desc[100];	
	char sublang[100];
	char sublang_desc[100];
	char subcolor[20];
	char border_color[20];
	int vsync;
	int framedrop;
	char alang[100];
	char alang_desc[100];	
	int volume;
	int softvol;
} XMPlayerCfg_t;

extern XMPlayerCfg_t XMPlayerCfg;
