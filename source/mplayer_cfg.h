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
} XMPlayerCfg_t;

extern XMPlayerCfg_t XMPlayerCfg;