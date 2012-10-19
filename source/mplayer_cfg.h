#pragma once

#define APPNAME "XMPlayer"
#define APPVERSION "0.0.2"
#define PREF_FILE_NAME 	"xmplayer.xml"
#define MAX_SHARES 5

 #ifdef __cplusplus 
extern "C" {
#endif	

	enum{
		LANG_ENGLISH,
		LANG_FRENCH,
		LANG_LENGTH
	};

	typedef struct {
		char ip[81];
		char share[81];
		char user[26];
		char pass[26];
		char name[41];
	} SMBSettings;

	typedef struct {
		int language;
		int exit_action;
		int sort_order;
		char subcp[100];
		char subcp_desc[100];	
		char sublang[100];
		char sublang_desc[100];
		unsigned int subcolor;
		unsigned int border_color;
		int vsync;
		int framedrop;
		char alang[100];
		char alang_desc[100];	
		int volume;
		int softvol;
		SMBSettings smb[MAX_SHARES];
	} XMPlayerCfg_t;

	extern XMPlayerCfg_t XMPlayerCfg;

#ifdef __cplusplus 
}
#endif
