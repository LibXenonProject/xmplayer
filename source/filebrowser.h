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
	BROWSER_TYPE type;
} BROWSERENTRY;

extern BROWSERINFO browser;
extern BROWSERENTRY * browserList;
extern char rootdir[10];

int UpdateDirName(int method);
int FileSortCallback(const void *f1, const void *f2);
void ResetBrowser();
int BrowserChangeFolder();
int BrowseDevice();
int BrowseDevice(const char * dir, const char * root);
bool MakeFilePath(char filepath[], int type, char * filename = NULL, int filenum = -2);
void CleanupPath(char * path);

int extIsValidVideoExt(char * ext);
int extIsValidAudioExt(char * ext);
int extIsValidPictureExt(char * ext);

BROWSER_TYPE file_type(const char * filename);

// always return 1
int extAlwaysValid(char *ext);
extern int (*extValid)(char * ext);
#endif
