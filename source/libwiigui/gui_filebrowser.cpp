/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_filebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../filebrowser.h"
#include "../w_input.h"
#include "../gui_debug.h"

//#define FILE_PAGESIZE 20

/**
 * Constructor for the GuiFileBrowser class.
 */
GuiFileBrowser::GuiFileBrowser(int w, int h, GuiImageData * list_bg, GuiImageData * folder_icon, GuiImageData ** file_icon) {
	width = w;
	height = h;
	numEntries = 0;
	selectedItem = 0;
	selectable = true;
	listChanged = true; // trigger an initial list update
	focus = 0; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

	//    btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	//    btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	//    bgFileSelection = new GuiImageData(xenon_filebrowser_png);
	//    bgFileSelectionImg = new GuiImage(bgFileSelection);
	//    bgFileSelectionImg->SetParent(this);
	//    bgFileSelectionImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgFileSelectionEntry = list_bg;
	fileFolder = folder_icon;
	fileIcon = file_icon;
	
	font_size = 20;
	selected_font_size = 26;
}

int GuiFileBrowser::GetPageSize() {
	return file_pagesize;
}

void GuiFileBrowser::SetFontSize(int size){
	font_size = size;
};
void GuiFileBrowser::SetSelectedFontSize(int size){
	selected_font_size = size;
};
void GuiFileBrowser::SetPageSize(int size) {
	file_pagesize = size;

	fileList = new GuiButton*[size];
	fileListText = new GuiText*[size];

	fileListBg = new GuiImage*[size];
	fileListFolder = new GuiImage*[size];
	fileListFile = new GuiImage*[size];
	bgFileSelectionImg = new GuiImage*[size];

	for (int i = 0; i < size; ++i) {

		fileListText[i] = new GuiText(NULL, font_size, (XeColor) {
			0xff, 0xff, 0xff, 0xff
		});
		fileListText[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		fileListText[i]->SetPosition(5, 0);
		fileListText[i]->SetMaxWidth(this->GetWidth()  - 50);

		fileListBg[i] = new GuiImage(bgFileSelectionEntry);
		
		fileListFolder[i] = new GuiImage(fileFolder);		
		fileListFolder[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		
		fileListFile[i] = new GuiImage(fileIcon[BROWSER_TYPE_UNKNOW]);			
		fileListFile[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

		fileList[i] = new GuiButton(this->GetWidth(), bgFileSelectionEntry->GetHeight());
		fileList[i]->SetParent(this);
		fileList[i]->SetLabel(fileListText[i]);
		fileList[i]->SetImageOver(fileListBg[i]);
		fileList[i]->SetPosition(2, bgFileSelectionEntry->GetHeight() * i );
		fileList[i]->SetTrigger(trigA);
		//        fileList[i]->SetTrigger(trig2);
		//        fileList[i]->SetSoundClick(btnSoundClick);
		
	}
}

/**
 * Destructor for the GuiFileBrowser class.
 */
GuiFileBrowser::~GuiFileBrowser() {
	delete bgFileSelectionImg;

	//    delete bgFileSelection;
	//delete bgFileSelectionEntry;
	//delete fileFolder;

	//    delete btnSoundOver;
	//    delete btnSoundClick;
	delete trigA;

	for (int i = 0; i < file_pagesize; i++) {
		delete fileListText[i];
		delete fileList[i];
		delete fileListBg[i];
		delete fileListFolder[i];	
		delete fileListFile[i];
	}
	
	delete fileListText;
	delete fileList;
	delete fileListBg;
	delete fileListFolder;
}

void GuiFileBrowser::SetFocus(int f) {
	focus = f;

	for (int i = 0; i < file_pagesize; i++)
		fileList[i]->ResetState();

	if (f == 1)
		fileList[selectedItem]->SetState(STATE_SELECTED);
}

void GuiFileBrowser::ResetState() {
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for (int i = 0; i < file_pagesize; i++) {
		fileList[i]->ResetState();
	}
}

void GuiFileBrowser::TriggerUpdate() {
	listChanged = true;
}

/**
 * Draw the button on screen
 */
void GuiFileBrowser::Draw() {
	if (!this->IsVisible())
		return;

	//    bgFileSelectionImg->Draw();

	for (u32 i = 0; i < file_pagesize; ++i) {
		fileList[i]->Draw();
	}

	this->UpdateEffects();
}

void GuiFileBrowser::DrawTooltip() {
}

void GuiFileBrowser::Update(GuiTrigger * t) {
	if (state == STATE_DISABLED || !t)
		return;

	// pad/joystick navigation
	if (!focus) {
		goto endNavigation; // skip navigation
		listChanged = false;
	}

	if (t->Right()) {
		if (browser.pageIndex < browser.numEntries && browser.numEntries > file_pagesize) {
			browser.pageIndex += file_pagesize;
			if (browser.pageIndex + file_pagesize >= browser.numEntries)
				browser.pageIndex = browser.numEntries - file_pagesize;
			listChanged = true;
		}
	} else if (t->Left()) {
		if (browser.pageIndex > 0) {
			browser.pageIndex -= file_pagesize;
			if (browser.pageIndex < 0)
				browser.pageIndex = 0;
			listChanged = true;
		}
	} else if (t->Down()) {
		if (browser.pageIndex + selectedItem + 1 < browser.numEntries) {
			if (selectedItem == file_pagesize - 1) {
				// move list down by 1
				++browser.pageIndex;
				listChanged = true;
			} else if (fileList[selectedItem + 1]->IsVisible()) {
				fileList[selectedItem]->ResetState();
				fileList[++selectedItem]->SetState(STATE_SELECTED, t->chan);
			}
		}
	} else if (t->Up()) {
		if (selectedItem == 0 && browser.pageIndex + selectedItem > 0) {
			// move list up by 1
			--browser.pageIndex;
			listChanged = true;
		} else if (selectedItem > 0) {
			fileList[selectedItem]->ResetState();
			fileList[--selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
	}

endNavigation:

	for (int i = 0; i < file_pagesize; ++i) {
		if (listChanged || numEntries != browser.numEntries) {
			if (browser.pageIndex + i < browser.numEntries) {
				if (fileList[i]->GetState() == STATE_DISABLED)
					fileList[i]->SetState(STATE_DEFAULT);

				fileList[i]->SetVisible(true);

				fileListText[i]->SetText(browserList[browser.pageIndex + i].displayname);

				if (browserList[browser.pageIndex + i].isdir) // directory
				{
					fileList[i]->SetIcon(fileListFolder[i]);
					fileListText[i]->SetPosition(fileListFolder[i]->GetWidth(), 0);
				} else {
					fileListFile[i]->SetImage(fileIcon[browserList[browser.pageIndex + i].type]);
					fileList[i]->SetIcon(fileListFile[i]);
					fileListText[i]->SetPosition(fileListFile[i]->GetWidth(), 0);
				}
			} else {
				fileList[i]->SetVisible(false);
				fileList[i]->SetState(STATE_DISABLED);
			}
		}

		if (i != selectedItem && fileList[i]->GetState() == STATE_SELECTED)
		{
			fileList[i]->ResetState();
		}
		else if (focus && i == selectedItem && fileList[i]->GetState() == STATE_DEFAULT)
		{
			fileList[selectedItem]->SetState(STATE_SELECTED, t->chan);
		}

		int currChan = t->chan;

		if (t->wpad->ir.valid && !fileList[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
			t->chan = -1;

		fileList[i]->Update(t);
		t->chan = currChan;

		if (fileList[i]->GetState() == STATE_SELECTED) {
			selectedItem = i;
			browser.selIndex = browser.pageIndex + i;
		}

		if (selectedItem == i){
			fileListText[i]->SetScroll(SCROLL_HORIZONTAL);
			fileListText[i]->SetFontSize(selected_font_size);
		}
		else{
			fileListText[i]->SetScroll(SCROLL_NONE);
			fileListText[i]->SetFontSize(font_size);
		}
	}

	listChanged = false;
	numEntries = browser.numEntries;

	if (updateCB)
		updateCB(this);
}