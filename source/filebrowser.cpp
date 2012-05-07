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

#include "filebrowser.h"

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

extern char * getRootDir();
extern char foldername[1024];
bool MakeFilePath(char filepath[], int type, char * filename, int filenum) {
    char file[512];
    char ext[4];
    char temppath[MAXPATHLEN];
    char foldername[512];

    sprintf(foldername,"states");
    sprintf(ext, "gpz");
    if (filenum >= -1) {
        if (filenum == -1)
            sprintf(file, "%s.%s", filename, ext);
        else if (filenum == 0)
            sprintf(file, "%s Auto.%s", filename, ext);
        else
            sprintf(file, "%s %i.%s", filename, filenum, ext);
    } else {
        sprintf(file, "%s", filename);
    }
    sprintf(temppath, "%s/%s/%s",getRootDir(), foldername, file);

    CleanupPath(temppath); // cleanup path
    snprintf(filepath, MAXPATHLEN, "%s", temppath);
    return true;
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

	return stricmp(((BROWSERENTRY *) f1)->filename, ((BROWSERENTRY *) f2)->filename);
}

int extValid(char * ext) {
	//    if (ext) {
	//        if (stricmp(ext, ".z") == 0) {
	//            return 1;
	//        } else if (stricmp(ext, ".gpz") == 0) {
	//            return 1;
	//        } else if (stricmp(ext, ".cue") == 0) {
	//            return 1;
	//        } else if (stricmp(ext, ".bin") == 0) {
	//            return 1;
	//        } else if (stricmp(ext, ".iso") == 0) {
	//            return 1;
	//        } else if (stricmp(ext, ".nrg") == 0) {
	//            return 1;
	//        }
	//    }
	//
	//    return 0;
	return 1;
}

/***************************************************************************
 * Browse subdirectories
 **************************************************************************/
int
ParseDirectory() {
	DIR *dir = NULL;
	char fulldir[MAXPATHLEN];
	struct dirent *entry;
	char * ext = NULL;

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

	// always add an .. entry
	if(1){
		BROWSERENTRY * newBrowserList = (BROWSERENTRY *) realloc(browserList, (entryNum + 1) * sizeof (BROWSERENTRY));
		
		browserList = newBrowserList;
		
		memset(&(browserList[entryNum]), 0, sizeof (BROWSERENTRY)); // clear the new entry

		strncpy(browserList[entryNum].filename, "..", MAXJOLIET);
		
		sprintf(browserList[entryNum].displayname, "Up One Level");
		browserList[entryNum].isdir = 1; // flag this as a dir
		entryNum++;
	}

	while ((entry = readdir(dir))) {
		if (strcmp(entry->d_name, ".") == 0)
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

		//
		ext = strrchr(entry->d_name, '.');

		if (strcmp(entry->d_name, "..") == 0) {
			
		} else if (extValid(ext) || entry->d_type == DT_DIR) {
			//}else if(1){
			strncpy(browserList[entryNum].displayname, entry->d_name, MAXDISPLAY); // crop name for display

			if (entry->d_type == DT_DIR)
				browserList[entryNum].isdir = 1; // flag this as a dir
		} else {
			continue;
		}

		entryNum++;
	}

	// close directory
	closedir(dir);

	// Sort the file list
	qsort(browserList, entryNum, sizeof (BROWSERENTRY), FileSortCallback);

	browser.numEntries = entryNum;
	return entryNum;
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
	sprintf(rootdir, "usb:/");
	ParseDirectory(); // Parse root directory
	return browser.numEntries;
}

/****************************************************************************
 * BrowseDevice
 * Displays a list of files on the selected device
 ***************************************************************************/
int BrowseDevice(const char * dir, const char * root) {
	sprintf(browser.dir, dir);
	sprintf(rootdir, root);
	ParseDirectory(); // Parse root directory
	return browser.numEntries;
}
