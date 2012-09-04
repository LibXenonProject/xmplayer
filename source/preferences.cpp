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
#include "tinyxml/tinyxml.h"
#include "mplayer_cfg.h"

#define MAXPATHLEN 256
extern char MPLAYER_DATADIR[100];
XMPlayerCfg_t XMPlayerCfg;
static char temp[200];

static const char * toStr(int i) {
	sprintf(temp, "%d", i);
	return temp;
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
static void DefaultSettings() {
	memset(&XMPlayerCfg, 0, sizeof (XMPlayerCfg_t));
}

/****************************************************************************
 * Save Preferences
 ***************************************************************************/
bool SavePrefs(bool silent) {
	char filepath[MAXPATHLEN];

	FixInvalidSettings();

	sprintf(filepath, "%s/%s", MPLAYER_DATADIR, PREF_FILE_NAME);
	
        TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
        doc.LinkEndChild(decl);
        
        TiXmlElement* settings = new TiXmlElement("settings");
	doc.LinkEndChild(settings);
	
        TiXmlElement* file = new TiXmlElement("file");
        settings->LinkEndChild(file);
        
        file->SetAttribute("app", APPNAME);
        file->SetAttribute("version", APPVERSION);
	
        TiXmlElement* menu = new TiXmlElement("menu");
	settings->LinkEndChild(menu);
	
	menu->SetAttribute("exit_action", toStr(XMPlayerCfg.exit_action));
        menu->SetAttribute("language", toStr(XMPlayerCfg.language));
		
	TiXmlElement* filebrowser = new TiXmlElement("filebrowser");
	settings->LinkEndChild(filebrowser);
	
	filebrowser->SetAttribute("sort_order", toStr(XMPlayerCfg.sort_order));
	
	bool success = doc.SaveFile(filepath);
	
    doc.Clear();
    
    if(success)
        return true;
    else
        return false;
}

/****************************************************************************
 * Load Preferences
 * Checks sources consecutively until we find a preference file
 ***************************************************************************/
static bool prefLoaded = false;

bool LoadPrefs() {
	if (prefLoaded) // already attempted loading
		return true;

        char filepath[MAXPATHLEN];
	sprintf(filepath, "%s/%s", MPLAYER_DATADIR, PREF_FILE_NAME);
        
        TiXmlDocument doc;
        
        bool loadOkay = doc.LoadFile(filepath);
	if (loadOkay) {
                FixInvalidSettings();
                TiXmlElement* settings = doc.FirstChildElement();
                if (settings == NULL) {
                       goto noheader;
                }
                for(TiXmlElement* elem = settings->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement()) {
                       const char* elemName = elem->Value();
                       const char* attr;
                       if (strcmp(elemName, "menu") == 0) {
                             attr = elem->Attribute("exit_action");
                             XMPlayerCfg.exit_action = atoi(attr);
                             attr = elem->Attribute("language");
                             XMPlayerCfg.language = atoi(attr);
                       } else if (strcmp(elemName, "filebrowser") == 0) { 		
                             attr = elem->Attribute("sort_order");
                             XMPlayerCfg.sort_order = atoi(attr);
                       }
                }
		doc.Clear();
                prefLoaded = true;
                
                printf("[Preferences] Sucessfully loaded xmplayer.xml \n");
                return true;
                
	} else {
 noheader:      DefaultSettings();
		printf("[Preferences] Failed to load xmplayer.xml - Loading default settings \n");
                return false;                       
	}
        
}