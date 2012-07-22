#pragma once

#define APPNAME "XMPlayer"
#define APPVERSION "0.0.1"
#define PREF_FILE_NAME 	"xmplayer.xml"

enum{
	LANG_ENGLISH,
	LANG_FRENCH,
	LANG_LENGTH
};
/*siz - added subtitle size: 14/07/2012 - Start */
enum{ 
	SUB_SMALL,
	SUB_NORMAL,
	SUB_BIG,
	SUB_LENGTH
};
/*siz - added subtitle size: 14/07/2012 - End */
typedef struct {
	int language;
	int exit_action;
	int subtitle_size; /*siz - added subtitle size: 14/07/2012 */
} XMPlayerCfg_t;

extern XMPlayerCfg_t XMPlayerCfg;
