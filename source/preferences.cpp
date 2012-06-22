/****************************************************************************
 *
 * preferences.cpp
 *
 * Preferences save/load to XML file
 ***************************************************************************/

#include <xetypes.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <mxml.h>
#include "mplayer_cfg.h"

#define MAXPATHLEN 256
#define SAVEBUFFERSIZE (1024 * 512)
extern char MPLAYER_DATADIR[100];
unsigned char *savebuffer;
XMPlayerCfg_t XMPlayerCfg;

/****************************************************************************
 * Prepare Preferences Data
 *
 * This sets up the save buffer for saving.
 ***************************************************************************/
static mxml_node_t *xml = NULL;
static mxml_node_t *data = NULL;
static mxml_node_t *section = NULL;
static mxml_node_t *item = NULL;
static mxml_node_t *elem = NULL;

static char temp[200];

static const char * toStr(int i) {
	sprintf(temp, "%d", i);
	return temp;
}

static const char * FtoStr(float i) {
	sprintf(temp, "%.2f", i);
	return temp;
}

static void createXMLSection(const char * name, const char * description) {
	section = mxmlNewElement(data, "section");
	mxmlElementSetAttr(section, "name", name);
	mxmlElementSetAttr(section, "description", description);
}

static void createXMLSetting(const char * name, const char * description, const char * value) {
	item = mxmlNewElement(section, "setting");
	mxmlElementSetAttr(item, "name", name);
	mxmlElementSetAttr(item, "value", value);
	mxmlElementSetAttr(item, "description", description);
}

static const char * XMLSaveCallback(mxml_node_t *node, int where) {
	const char *name;

	name = node->value.element.name;

	if (where == MXML_WS_BEFORE_CLOSE) {
		if (!strcmp(name, "file") || !strcmp(name, "section"))
			return ("\n");
		else if (!strcmp(name, "controller"))
			return ("\n\t");
	}
	if (where == MXML_WS_BEFORE_OPEN) {
		if (!strcmp(name, "file"))
			return ("\n");
		else if (!strcmp(name, "section"))
			return ("\n\n");
		else if (!strcmp(name, "setting") || !strcmp(name, "controller"))
			return ("\n\t");
		else if (!strcmp(name, "button"))
			return ("\n\t\t");
	}
	return (NULL);
}

static int preparePrefsData() {
	xml = mxmlNewXML("1.0");
	mxmlSetWrapMargin(0); // disable line wrapping

	data = mxmlNewElement(xml, "file");
	mxmlElementSetAttr(data, "app", APPNAME);
	mxmlElementSetAttr(data, "version", APPVERSION);

	createXMLSection("Menu", "Menu Settings");

	createXMLSetting("ExitAction", "Exit Action", toStr(XMPlayerCfg.exit_action));
	createXMLSetting("language", "Language", toStr(XMPlayerCfg.language));

	int datasize = mxmlSaveString(xml, (char *) savebuffer, SAVEBUFFERSIZE, XMLSaveCallback);

	mxmlDelete(xml);
	
	printf("XMPlayerCfg.language : %d\n",XMPlayerCfg.language);

	return datasize;
}

/****************************************************************************
 * loadXMLSetting
 *
 * Load XML elements into variables for an individual variable
 ***************************************************************************/

static void loadXMLSetting(char * var, const char * name, int maxsize) {
	item = mxmlFindElement(xml, xml, "setting", "name", name, MXML_DESCEND);
	if (item) {
		const char * tmp = mxmlElementGetAttr(item, "value");
		if (tmp)
			snprintf(var, maxsize, "%s", tmp);
	}
}

static void loadXMLSetting(int * var, const char * name) {
	item = mxmlFindElement(xml, xml, "setting", "name", name, MXML_DESCEND);
	if (item) {
		const char * tmp = mxmlElementGetAttr(item, "value");
		if (tmp)
			*var = atoi(tmp);
	}
}

static void loadXMLSetting(float * var, const char * name) {
	item = mxmlFindElement(xml, xml, "setting", "name", name, MXML_DESCEND);
	if (item) {
		const char * tmp = mxmlElementGetAttr(item, "value");
		if (tmp)
			*var = atof(tmp);
	}
}

/****************************************************************************
 * decodePrefsData
 *
 * Decodes preferences - parses XML and loads preferences into the variables
 ***************************************************************************/
static bool decodePrefsData() {
	bool result = false;
	
	xml = mxmlLoadString(NULL, (char *) savebuffer, MXML_TEXT_CALLBACK);

	if (xml) {
		// check settings version
		item = mxmlFindElement(xml, xml, "file", "version", NULL, MXML_DESCEND);
		
		result = true;
		
//		if (item) // a version entry exists
//		{
//			const char * version = mxmlElementGetAttr(item, "version");
//
//			if (version && strlen(version) == 5) {
//				// this code assumes version in format X.X.X
//				// XX.X.X, X.XX.X, or X.X.XX will NOT work
//				int verMajor = version[0] - '0';
//				int verMinor = version[2] - '0';
//				int verPoint = version[4] - '0';
//				int curMajor = APPVERSION[0] - '0';
//				int curMinor = APPVERSION[2] - '0';
//				int curPoint = APPVERSION[4] - '0';
//
//				// first we'll check that the versioning is valid
//				if (!(verMajor >= 0 && verMajor <= 9 &&
//						verMinor >= 0 && verMinor <= 9 &&
//						verPoint >= 0 && verPoint <= 9))
//					result = false;
//				else if (verMajor < 4) // less than version 4.0.0
//					result = false; // reset settings
//				else if (verMajor == 4 && verMinor == 0 && verPoint < 2) // anything less than 4.0.2
//					result = false; // reset settings
//				else if ((verMajor * 100 + verMinor * 10 + verPoint) >
//						(curMajor * 100 + curMinor * 10 + curPoint)) // some future version
//					result = false; // reset settings
//				else
//					result = true;
//			}
//		}

		if (result) {
			// Menu Settings
			loadXMLSetting(&XMPlayerCfg.exit_action, "ExitAction");
			loadXMLSetting(&XMPlayerCfg.language, "language");
		}
		mxmlDelete(xml);
		
		printf("XMPlayerCfg.language : %d\n",XMPlayerCfg.language);
	}
	return result;
}

/****************************************************************************
 * FixInvalidSettings
 *
 * Attempts to correct at least some invalid settings - the ones that
 * might cause crashes
 ***************************************************************************/
void FixInvalidSettings() {
	if (XMPlayerCfg.language < 0 || XMPlayerCfg.language >= LANG_LENGTH)
		XMPlayerCfg.language = LANG_ENGLISH;
}

/****************************************************************************
 * DefaultSettings
 *
 * Sets all the defaults!
 ***************************************************************************/
void
DefaultSettings() {
	memset(&XMPlayerCfg, 0, sizeof (XMPlayerCfg_t));
}

/****************************************************************************
 * Save Preferences
 ***************************************************************************/
static char prefpath[MAXPATHLEN] = {0};

/****************************************************************************
 * SaveFile
 * Write buffer to file
 ***************************************************************************/
size_t SaveFile(char * buffer, char *filepath, size_t datasize, bool silent) {
	size_t written = 0;
	size_t writesize, nextwrite;
	FILE *file = NULL;

	if (datasize == 0)
		return 0;

	file = fopen(filepath, "wb");

	if (!file) {
		return written;
	}

	while (written < datasize) {
		if (datasize - written > 4096) nextwrite = 4096;
		else nextwrite = datasize - written;
		writesize = fwrite(buffer + written, 1, nextwrite, file);
		if (writesize != nextwrite) break; // write failure
		written += writesize;
	}
	fclose(file);

	if (written != datasize)
		written = 0;

	return written;
}

size_t SaveFile(char * filepath, size_t datasize, bool silent) {
	return SaveFile((char *) savebuffer, filepath, datasize, silent);
}

/****************************************************************************
 * LoadFile
 ***************************************************************************/
size_t LoadFile(char * rbuffer, char *filepath, size_t length, bool silent) {
	size_t size = 0, offset = 0, readsize = 0;
	FILE *file = NULL;

	file = fopen(filepath, "rb");

	if (!file) {
		return 0;
	}

	if (length > 0 && length <= 2048) // do a partial read (eg: to check file header)
	{
		size = fread(rbuffer, 1, length, file);
	}
	else // load whole file
	{
		fseeko(file, 0, SEEK_END);
		size = ftello(file);
		fseeko(file, 0, SEEK_SET);

		while (!feof(file)) {
			readsize = fread(rbuffer + offset, 1, 4096, file); // read in next chunk

			if (readsize <= 0)
				break; // reading finished (or failed)

			offset += readsize;
		}
		size = offset;

	}
	fclose(file);

	return size;
}

size_t LoadFile(char * filepath, bool silent) {
	return LoadFile((char *) savebuffer, filepath, 0, silent);
}

bool SavePrefs(bool silent) {
	char filepath[MAXPATHLEN];
	int datasize;
	int offset = 0;

	FixInvalidSettings();

	savebuffer = (unsigned char*) malloc(SAVEBUFFERSIZE);

	datasize = preparePrefsData();

	sprintf(filepath, "%s/%s", MPLAYER_DATADIR, PREF_FILE_NAME);
	offset = SaveFile(filepath, datasize, silent);

	free(savebuffer);

	if (offset > 0) {
		return true;
	}
	return false;
}

/****************************************************************************
 * Load Preferences from specified filepath
 ***************************************************************************/
bool LoadPrefsFromMethod(char * path) {
	bool retval = false;
	int size = 0;
	char filepath[MAXPATHLEN];

	sprintf(filepath, "%s/%s", path, PREF_FILE_NAME);

	savebuffer = (unsigned char*) malloc(SAVEBUFFERSIZE);

	size = LoadFile(filepath, 1);

	if (size > 0)
		retval = decodePrefsData();

	free(savebuffer);

	if (retval) {
		strcpy(prefpath, path);
	}
	
	return retval;
}

/****************************************************************************
 * Load Preferences
 * Checks sources consecutively until we find a preference file
 ***************************************************************************/
static bool prefLoaded = false;

bool LoadPrefs() {
	if (prefLoaded) // already attempted loading
		return true;

	bool prefFound = false;

	prefFound = LoadPrefsFromMethod(MPLAYER_DATADIR);

	prefLoaded = true; // attempted to load preferences

	if (prefFound)
		FixInvalidSettings();

	return prefFound;
}


