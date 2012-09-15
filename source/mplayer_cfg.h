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
	char subcolor[8];
	char sub_bcolor[8];	
} XMPlayerCfg_t;

extern XMPlayerCfg_t XMPlayerCfg;
