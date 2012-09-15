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

	if (!XMPlayerCfg.subcolor) {
		sprintf(XMPlayerCfg.subcolor, "FFFFFF00");
	}
	if (!XMPlayerCfg.sub_bcolor) {
		sprintf(XMPlayerCfg.sub_bcolor, "00000000");		
	}
	if (!XMPlayerCfg.subcp) {
		sprintf(XMPlayerCfg.subcp, "ISO-8859-1");
	}
	if (!XMPlayerCfg.subcp_desc) {
		sprintf(XMPlayerCfg.subcp_desc, "Western European");
	}
	if (!XMPlayerCfg.sublang) {
		sprintf(XMPlayerCfg.sublang, "en");
	}
	if (!XMPlayerCfg.sublang_desc) {
		sprintf(XMPlayerCfg.sublang_desc, "English");
	}	
}

/****************************************************************************
 * DefaultSettings
 *
 * Sets all the defaults!
 ***************************************************************************/
static void DefaultSettings() {
		XMPlayerCfg.language = 0;
		XMPlayerCfg.exit_action = 0;
		XMPlayerCfg.sort_order = 0;
		sprintf(XMPlayerCfg.subcolor, "FFFFFF00");
		sprintf(XMPlayerCfg.sub_bcolor, "00000000");		
		sprintf(XMPlayerCfg.subcp, "ISO-8859-1");
		sprintf(XMPlayerCfg.subcp_desc, "Western European");
		sprintf(XMPlayerCfg.sublang, "en");
		sprintf(XMPlayerCfg.sublang_desc, "English");
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
	//Global
        TiXmlElement* global = new TiXmlElement("global");
	settings->LinkEndChild(global);
	
	TiXmlElement* exit = new TiXmlElement("exit");
	global->LinkEndChild(exit);
	exit->SetAttribute("value", toStr(XMPlayerCfg.exit_action));
	
	TiXmlElement* language = new TiXmlElement("language");
	global->LinkEndChild(language);
        language->SetAttribute("value", toStr(XMPlayerCfg.language));
	//File browser		
	TiXmlElement* filebrowser = new TiXmlElement("filebrowser");
	settings->LinkEndChild(filebrowser);
	
	TiXmlElement* sort = new TiXmlElement("sort");
	filebrowser->LinkEndChild(sort);
	sort->SetAttribute("value", toStr(XMPlayerCfg.sort_order));
	//Subtitles
	TiXmlElement* subtitles = new TiXmlElement("subtitles");
	settings->LinkEndChild(subtitles);
	
	TiXmlElement* subcolor = new TiXmlElement("sub_color");
	subtitles->LinkEndChild(subcolor);
	subcolor->SetAttribute("value", XMPlayerCfg.subcolor);

	TiXmlElement* sub_bcolor = new TiXmlElement("border_color");
	subtitles->LinkEndChild(sub_bcolor);
	sub_bcolor->SetAttribute("value", XMPlayerCfg.sub_bcolor);

	TiXmlElement* subcp = new TiXmlElement("codepage");
	subtitles->LinkEndChild(subcp);
	subcp->SetAttribute("value", XMPlayerCfg.subcp);
	subcp->SetAttribute("desc", XMPlayerCfg.subcp_desc);
	
	TiXmlElement* sublang = new TiXmlElement("language");
	subtitles->LinkEndChild(sublang);
	sublang->SetAttribute("value", XMPlayerCfg.sublang);
	sublang->SetAttribute("desc", XMPlayerCfg.sublang_desc);
	
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
	        TiXmlHandle docu(&doc);
                TiXmlElement* settings = docu.FirstChildElement().Element();
                if (settings == NULL) {
                       goto noheader;
                }
                TiXmlHandle handle(0);
                TiXmlElement* elem;
                handle = TiXmlHandle(settings);
                elem = handle.FirstChild("global").FirstChild().Element();
                for(elem; elem; elem = elem->NextSiblingElement()) {
                       const char* elemName = elem->Value();
                       if (strcmp(elemName, "exit") == 0) {
                             XMPlayerCfg.exit_action = atoi(elem->Attribute("value"));
                       } else if (strcmp(elemName, "language") == 0) {
                             XMPlayerCfg.language = atoi(elem->Attribute("value"));
               	       }
               	}
               	elem = handle.FirstChild("filebrowser").FirstChild().Element();
                for(elem; elem; elem = elem->NextSiblingElement()) {
                       const char* elemName = elem->Value();
                       if (strcmp(elemName, "sort") == 0) { 		
                             XMPlayerCfg.sort_order = atoi(elem->Attribute("value"));
                       }
                }
               	elem = handle.FirstChild("subtitles").FirstChild().Element();                
                for(elem; elem; elem = elem->NextSiblingElement()) {
                       const char* elemName = elem->Value();
			if (strcmp(elemName, "sub_color") == 0) {
                             sprintf(XMPlayerCfg.subcolor, elem->Attribute("value"));
                       } else if (strcmp(elemName, "border_color") == 0) {
                             sprintf(XMPlayerCfg.sub_bcolor, elem->Attribute("value"));
                       } else if (strcmp(elemName, "codepage") == 0) {
                             sprintf(XMPlayerCfg.subcp, elem->Attribute("value"));
                             sprintf(XMPlayerCfg.subcp_desc, elem->Attribute("desc"));
                       } else if (strcmp(elemName, "language") == 0) {
                             sprintf(XMPlayerCfg.sublang, elem->Attribute("value"));
                             sprintf(XMPlayerCfg.sublang_desc, elem->Attribute("desc"));
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
