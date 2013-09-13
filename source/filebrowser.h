/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.h
 *
 * Generic file routines - reading, writing, browsing
 ****************************************************************************/

#ifndef _FILEBROWSER_H_
#define _FILEBROWSER_H_

#include <unistd.h>
#include <xetypes.h>
#include <diskio/disc_io.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <string>

#define MAXJOLIET 255
#define MAXDISPLAY 90

enum BROWSER_TYPE {
	BROWSER_TYPE_UNKNOW,
	BROWSER_TYPE_VIDEO,
	BROWSER_TYPE_AUDIO,
	BROWSER_TYPE_PICTURE,
	BROWSER_TYPE_ELF,
	BROWSER_TYPE_NAND,
	BROWSER_TYPE_MAX
};

typedef struct {
	char dir[MAXPATHLEN]; // directory path of browserList
	int numEntries; // # of entries in browserList
	int selIndex; // currently selected index of browserList
	int pageIndex; // starting index of browserList page display
} BROWSERINFO;

typedef struct {
	char isdir; // 0 - file, 1 - directory
	char filename[MAXJOLIET + 1]; // full filename
	char displayname[MAXDISPLAY + 1]; // name for browser display
	char moddate[20];
	time_t date;
	BROWSER_TYPE type;
} BROWSERENTRY;

typedef struct _vidext {
	std::string vid_ext;

} VID_EXT;

typedef struct _audext {
	std::string aud_ext;

} AUD_EXT;

typedef struct _picext {
	std::string pic_ext;

} PIC_EXT;

#define VID_EXT_SIZE 28
#define AUD_EXT_SIZE 6
#define PIC_EXT_SIZE 5

extern VID_EXT video_extensions[VID_EXT_SIZE];
extern AUD_EXT audio_extensions[AUD_EXT_SIZE];
extern PIC_EXT picture_extensions[PIC_EXT_SIZE];

extern BROWSERINFO browser;
extern BROWSERENTRY * browserList;
extern char rootdir[10];

int UpdateDirName(int method);
int FileSortCallback(const void *f1, const void *f2);
void BrowserSortList();
void ResetBrowser();
int BrowserChangeFolder();
int BrowseDevice();
int BrowseDevice(std::string dir, std::string root);
bool MakeFilePath(char filepath[], int type, char * filename = NULL, int filenum = -2);
void CleanupPath(char * path);

int extIsValidVideoExt(std::string ext);
int extIsValidAudioExt(std::string ext);
int extIsValidPictureExt(std::string ext);

BROWSER_TYPE file_type(char * filename);

// always return 1
int extAlwaysValid(std::string ext);
extern int (*extValid)(std::string ext);
#endif
