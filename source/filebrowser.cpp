/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * filebrowser.cpp
 *
 * Generic file routines - reading, writing, browsing
 ***************************************************************************/

#include <dirent.h>
#include <xetypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <wiiuse/wpad.h>
//#include <sys/dir.h>
#include <malloc.h>
#include <debug.h>
#include <fcntl.h>
#include "filebrowser.h"
#include "mplayer_cfg.h"

BROWSERINFO browser;
BROWSERENTRY * browserList = NULL; // list of files/folders in browser

char rootdir[10];

/****************************************************************************
 * CleanupPath()
 * Cleans up the filepath, removing double // and replacing \ with /
 ***************************************************************************/
void CleanupPath(char * path) {
    if (!path || path[0] == 0)
        return;

    int pathlen = strlen(path);
    int j = 0;
    for (int i = 0; i < pathlen && i < MAXPATHLEN; i++) {
        if (path[i] == '\\')
            path[i] = '/';

        if (j == 0 || !(path[j - 1] == '/' && path[i] == '/'))
            path[j++] = path[i];
    }
    path[j] = 0;
}

/****************************************************************************
 * ResetBrowser()
 * Clears the file browser memory, and allocates one initial entry
 ***************************************************************************/
void ResetBrowser() {
    browser.numEntries = 0;
    browser.selIndex = 0;
    browser.pageIndex = 0;

    // Clear any existing values
    if (browserList != NULL) {
        free(browserList);
        browserList = NULL;
    }
    // set aside space for 1 entry
    browserList = (BROWSERENTRY *) malloc(sizeof (BROWSERENTRY));
    memset(browserList, 0, sizeof (BROWSERENTRY));
}

/****************************************************************************
 * UpdateDirName()
 * Update curent directory name for file browser
 ***************************************************************************/
int UpdateDirName() {
    int size = 0;
    char * test;
    char temp[1024];

    /* current directory doesn't change */
    if (strcmp(browserList[browser.selIndex].filename, ".") == 0) {
        return 0;
    }/* go up to parent directory */
    else if (strcmp(browserList[browser.selIndex].filename, "..") == 0) {
        /* determine last subdirectory namelength */
        sprintf(temp, "%s", browser.dir);
        test = strtok(temp, "/");
        while (test != NULL) {
            size = strlen(test);
            test = strtok(NULL, "/");
        }

        /* remove last subdirectory name */
        size = strlen(browser.dir) - size - 1;
        browser.dir[size] = 0;

        return 1;
    }/* Open a directory */
	else {
		/* test new directory namelength */
		if ((strlen(browser.dir) + 1 + strlen(browserList[browser.selIndex].filename)) < MAXPATHLEN) {
			/* update current directory name */
			sprintf(browser.dir, "%s/%s", browser.dir, browserList[browser.selIndex].filename);
			return 1;
		} else {
			return -1;
		}
	}
}

/****************************************************************************
 * FileSortCallback
 *
 * Quick sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/
int FileSortCallback(const void *f1, const void *f2) {
	/* Special case for implicit directories */
	if (((BROWSERENTRY *) f1)->filename[0] == '.' || ((BROWSERENTRY *) f2)->filename[0] == '.') {
		if (strcmp(((BROWSERENTRY *) f1)->filename, ".") == 0) {
			return -1;
		}
		if (strcmp(((BROWSERENTRY *) f2)->filename, ".") == 0) {
			return 1;
		}
		if (strcmp(((BROWSERENTRY *) f1)->filename, "..") == 0) {
			return -1;
		}
		if (strcmp(((BROWSERENTRY *) f2)->filename, "..") == 0) {
			return 1;
		}
	}

	/* If one is a file and one is a directory the directory is first. */
	if (((BROWSERENTRY *) f1)->isdir && !(((BROWSERENTRY *) f2)->isdir)) return -1;
	if (!(((BROWSERENTRY *) f1)->isdir) && ((BROWSERENTRY *) f2)->isdir) return 1;
	
	//Ascending Name
	if (XMPlayerCfg.sort_order == 0) {
		return stricmp(((BROWSERENTRY *) f1)->filename, ((BROWSERENTRY *) f2)->filename);
	}
	//Descending Name
	else if (XMPlayerCfg.sort_order == 1) {
		return stricmp(((BROWSERENTRY *) f2)->filename, ((BROWSERENTRY *) f1)->filename);
	}
	//Date Ascending
	else if (XMPlayerCfg.sort_order == 2) {
		if ( ((BROWSERENTRY *) f2)->date == ((BROWSERENTRY *) f1)->date) { //if date is the same order by filename
			return stricmp(((BROWSERENTRY *) f2)->filename, ((BROWSERENTRY *) f1)->filename);
		} else {
			return ((BROWSERENTRY *) f1)->date - ((BROWSERENTRY *) f2)->date;
		}
	}
	//Date Descending
	else if (XMPlayerCfg.sort_order == 3) {
		if ( ((BROWSERENTRY *) f2)->date == ((BROWSERENTRY *) f1)->date) { //if date is the same order by filename
			return stricmp(((BROWSERENTRY *) f1)->filename, ((BROWSERENTRY *) f2)->filename);
		} else {
			return ((BROWSERENTRY *) f2)->date - ((BROWSERENTRY *) f1)->date;
		}
	}
}

VID_EXT video_extensions[VID_EXT_SIZE] = {
	{ ".mkv" }, 
	{ ".mov" }, 
	{ ".mp4" }, 
	{ ".mp4v" },
	{ ".divx" },
	{ ".avi" }, 
	{ ".asf" }, 
	{ ".wmv" }, 
	{ ".vc1" },
	{ ".tivo" },
	{ ".mpg" }, 
	{ ".mpeg" }, 
	{ ".mpeg2" }, 
	{ ".ts" }, 
	{ ".m2ts" },
	{ ".ogv" }, 
	{ ".ogx" }, 
	{ ".vp3" }, 
	{ ".vp6" },
	{ ".vp7" },
	{ ".flv" },
	{ ".264" }, 
	{ ".x264" },
        { ".h264" },
	{ ".3gp" }, 
	{ ".3g2" }, 
	{ ".rar" },
	{ ".rmvb" }
};

AUD_EXT audio_extensions[AUD_EXT_SIZE] = {
	{ ".mp3" }, 
	{ ".mp4" }, 
	{ ".ogg" }, 
	{ ".aac" },
	{ ".ac3" }
};


PIC_EXT picture_extensions[PIC_EXT_SIZE] = {
	{ ".jpg" }, 
	{ ".png" }, 
	{ ".jpeg" }, 
	{ ".bmp" }
};

int extIsValidVideoExt(std::string ext) {
	if (!ext.empty() && ext.size() > 1) {
		for (int i = 0; i < VID_EXT_SIZE; i++) {
			if (ext == video_extensions[i].vid_ext)
				return 1;
		}
	}
	return 0;
}

int extIsValidAudioExt(std::string ext) {
	if (!ext.empty() && ext.size() > 1) {
		for (int i = 0; i < AUD_EXT_SIZE; i++) {
			if (ext == audio_extensions[i].aud_ext)
				return 1;
		}
	}
	return 0;
}

int extIsValidPictureExt(std::string ext) {
	if (!ext.empty() && ext.size() > 1) {
		for (int i = 0; i < PIC_EXT_SIZE; i++) {
			if (ext == picture_extensions[i].pic_ext)
				return 1;
		}
	}
	return 0;
}

// get filetype based on extention

BROWSER_TYPE file_type(char * filename) {
	std::string ext(strrchr(filename, '.'));
	if (!ext.empty() && ext.size() > 1) {
		if (extIsValidVideoExt(ext))
			return BROWSER_TYPE_VIDEO;
		else if (extIsValidAudioExt(ext))
			return BROWSER_TYPE_AUDIO;
		else if (extIsValidPictureExt(ext))
			return BROWSER_TYPE_PICTURE;
		if (!ext.empty() && ext.size() > 2) {
			if (ext == ".elf")
				return BROWSER_TYPE_ELF;
			else if (ext == ".bin")
				return BROWSER_TYPE_NAND;
		}
	}

	return BROWSER_TYPE_UNKNOW;
}

int extAlwaysValid(std::string ext) {
	return 1;
}

int (*extValid)(std::string ext);

#define getFormatDate() "%d/%m/%Y"

static void getDate(time_t time, char * out) {
	struct tm* ts;
	if (time) {
		ts = localtime(&time);
		strftime(out, 20, "%d/%m/%Y", ts);
	} else {
		strcpy(out, "00/00/0000");
	}
}

static std::string getStrExt(const char * str) {
	if (strlen(str) > 0) {
		std::string file(str);
		std::string ext = file.substr(file.find_last_of("."));
		if (!ext.empty()) {
			return ext;	
		} else {
			return "";
		}
	} else {
		return "";
	}
}

/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
int ParseDirectory() {
	DIR *dir = NULL;
	char fulldir[MAXPATHLEN];
	char file_path[MAXPATHLEN];
	struct dirent *entry;
	TR;
	// reset browser
	ResetBrowser();

	// open the directory
	sprintf(fulldir, "%s%s", rootdir, browser.dir); // add currentDevice to path
	dir = opendir(fulldir);

	// if we can't open the dir, try opening the root dir
	if (dir == NULL) {
		sprintf(browser.dir, "/");
		dir = opendir(rootdir);
		if (dir == NULL) {
			return -1;
		}
	}

	// index files/folders
	int entryNum = 0;
	TR;
	// always add an .. entry
	if (strcmp(browser.dir, "/")) {

		BROWSERENTRY * newBrowserList = (BROWSERENTRY *) realloc(browserList, (entryNum + 1) * sizeof (BROWSERENTRY));

		browserList = newBrowserList;

		memset(&(browserList[entryNum]), 0, sizeof (BROWSERENTRY)); // clear the new entry

		strncpy(browserList[entryNum].filename, "..", MAXJOLIET);

		sprintf(browserList[entryNum].displayname, "..");
		browserList[entryNum].isdir = 1; // flag this as a dir
		entryNum++;
	}

	while ((entry = readdir(dir))) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
			continue;

		BROWSERENTRY * newBrowserList = (BROWSERENTRY *) realloc(browserList, (entryNum + 1) * sizeof (BROWSERENTRY));

		if (!newBrowserList) // failed to allocate required memory
		{
			ResetBrowser();
			entryNum = -1;
			break;
		} else {
			browserList = newBrowserList;
		}
		memset(&(browserList[entryNum]), 0, sizeof (BROWSERENTRY)); // clear the new entry

		strncpy(browserList[entryNum].filename, entry->d_name, MAXJOLIET);

		sprintf(file_path, "%s/%s", fulldir, entry->d_name);
		
		getDate(entry->d_mtime, browserList[entryNum].moddate);
		browserList[entryNum].date = entry->d_mtime;
		TR;
		if (extValid(getStrExt(entry->d_name)) || entry->d_type == DT_DIR) {
		TR;
			if (entry->d_type != DT_DIR)
				browserList[entryNum].type = file_type(entry->d_name);

			strncpy(browserList[entryNum].displayname, entry->d_name, MAXDISPLAY); // crop name for display

			if (entry->d_type == DT_DIR)
				browserList[entryNum].isdir = 1; // flag this as a dir

		} else {
			continue;
		}
		TR;
		entryNum++;
	}

	// close directory
	closedir(dir);

	// Sort the file list
	qsort(browserList, entryNum, sizeof (BROWSERENTRY), FileSortCallback);
	TR;
	browser.numEntries = entryNum;
	return entryNum;
}


void BrowserSortList() {
	qsort(browserList, browser.numEntries, sizeof (BROWSERENTRY), FileSortCallback);
}

/****************************************************************************
 * BrowserChangeFolder
 *
 * Update current directory and set new entry list if directory has changed
 ***************************************************************************/
int BrowserChangeFolder() {
	if (!UpdateDirName())
		return -1;

	ParseDirectory();

	return browser.numEntries;
}

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected device
 ***************************************************************************/
int BrowseDevice() {
	sprintf(browser.dir, "/");
	//sprintf(rootdir, "usb:/");
	ParseDirectory(); // Parse root directory
	return browser.numEntries;
}

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected device
 ***************************************************************************/
int BrowseDevice(std::string dir, std::string root) {
	sprintf(browser.dir, dir.c_str());
	sprintf(rootdir, root.c_str());
	ParseDirectory(); // Parse root directory
	return browser.numEntries;
}
