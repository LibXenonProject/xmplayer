/*
 * File:   newmain.c
 * Author: cc
 *
 * Created on 27 avril 2012, 19:58
 */

#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <libfat/fat.h>
#include <libext2/ext2.h>
#include <libntfs/ntfs.h>
#include <usb/usbmain.h>
#include <diskio/ata.h>
#include <xenon_soc/xenon_power.h>
#include <xenon_smc/xenon_smc.h> 
#include <sys/iosupport.h>
#include <ppc/atomic.h>
//#include <network/network.h>
#include <time/time.h>
#include <elf/elf.h>

//#include "http_client.h"

#include "gettext.h"
#include "preferences.h"

#include "libwiigui/gui.h"
#include "libwiigui/gui_list.h"
#include "libwiigui/gui_tab.h"

#include "filebrowser.h"

#include "mplayer_func.h"
#include "mplayer_common.h"

// list of all data files
#include "datalist.h"

#include "menu.h"

#define _(x)	gettext(x)


static int need_gui = 1;
extern "C" void mount_all_devices();

//**************************************************************************
// D-Pad direction for OSD Settings
//**************************************************************************
extern int osd_pad_right; //from gui_optionbrowser.cpp
extern int osd_pad_left; //from gui_optionbrowser.cpp
extern int osd_level;

//**************************************************************************
// Device information
//**************************************************************************
static char * root_dev = NULL;
static int device_list_size = 0;
static char device_list[STD_MAX][10];


//**************************************************************************
// Loading thread
//**************************************************************************
static int loading_thread_finished = 0;
static int end_loading_thread = 0;
static unsigned char thread_stack[6][0x10000] __attribute__((aligned(256)));
static unsigned int loadingThreadLock = 0;


//**************************************************************************
// Gui
//**************************************************************************

GuiImage * bgImg = NULL;
GuiWindow * mainWindow = NULL;
GuiTrigger * trigA;

// used at loading
static XenosSurface * logo = NULL;
static XenosSurface * loading[4] = {NULL};

//browser sorting
static GuiText * browser_sortText = NULL;
static GuiImage * browser_sort_up = NULL;
static GuiImage * browser_sort_down = NULL;

// Browser
static GuiFileBrowser * gui_browser = NULL;
static GuiImage * browser_top_bg = NULL;
static GuiText * browser_headline = NULL;
static GuiText * browser_pagecounter = NULL;
static GuiText * browser_subheadline = NULL;

static GuiImageData * browser_photo_icon = NULL;
static GuiImageData * browser_video_icon = NULL;
static GuiImageData * browser_music_icon = NULL;

static GuiImageData * browser_photo_folder_icon = NULL;
static GuiImageData * browser_video_folder_icon = NULL;
static GuiImageData * browser_music_folder_icon = NULL;

static GuiImageData * browser_selector = NULL;

static GuiImage * browser_up_icon = NULL;
static GuiImage * browser_down_icon = NULL;

// no image data, pointer to image
static GuiImageData * browser_folder_icon = NULL;
static GuiImageData * browser_files_icon[BROWSER_TYPE_MAX] = {NULL};

// Home screen
static GuiList * home_list_v = NULL;
static GuiList * home_list_h = NULL;
static GuiImage * home_list_h_selector = NULL;
static GuiText * home_curitem = NULL;

static GuiImage * home_left = NULL;
static GuiImage * home_main_function_frame_bg = NULL;

static GuiImage * home_hdd_icon[STD_MAX];
static GuiImage * home_no_hdd_icon[STD_MAX];
static GuiButton * home_device_btn[STD_MAX];

static GuiButton * home_video_btn = NULL;
static GuiButton * home_all_btn = NULL;
static GuiButton * home_music_btn = NULL;
static GuiButton * home_photo_btn = NULL;
static GuiButton * home_setting_btn = NULL;
static GuiButton * home_restart_btn = NULL;
static GuiButton * home_shutdown_btn = NULL;

static GuiImage * home_video_img = NULL;
static GuiImage * home_music_img = NULL;
static GuiImage * home_photo_img = NULL;
static GuiImage * home_setting_img = NULL;

static GuiText* home_video_txt = NULL;
static GuiText * home_all_txt = NULL;
static GuiText * home_music_txt = NULL;
static GuiText * home_photo_txt = NULL;
static GuiText * home_setting_txt = NULL;
static GuiText * home_restart_txt = NULL;
static GuiText * home_shutdown_txt = NULL;

//**************************************************************************
// Mplayer variables
//**************************************************************************
static char mplayer_filename[2048];

//Saved seek
static char seek_filename[2048];

// ass color buffer
// since we pass directly color to mplayer we need to allocate them or made them point to a aligned buffer
static char _ass_color[10];
static char _ass_border_color[10];

//**************************************************************************
// Browser variables
//**************************************************************************
static const char * exited_root = "";
static char exited_dir[MAXPATHLEN];
static char exited_dir_array[64][MAXPATHLEN];
static int exited_item[64];
static int exited_page[64];
static int exited_i[64];

//Audio player
static int last_menu;
static int current_menu = HOME_PAGE;

static void update()
{
	UpdatePads();
	mainWindow->Draw();
	Menu_Render();
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
}

static void resetController()
{
	int i = 0;
	for (i = 0; i < 4; i++) {
		struct controller_data_s ctrl_zero = {};
		set_controller_data(i, &ctrl_zero);
	}
}

static int GetALangIndex()
{
	for (int i = 0; i < LANGUAGE_SIZE; i++)
		if (strcmp(XMPlayerCfg.alang, languages[i].abbrev2) == 0)
			return i;
	return 0;
}

static int GetCodepageIndex()
{
	for (int i = 0; i < CODEPAGE_SIZE; i++)
		if (strcmp(XMPlayerCfg.subcp, codepages[i].cpname) == 0)
			return i;
	return 0;
}

static int GetLangIndex()
{
	for (int i = 0; i < LANGUAGE_SIZE; i++)
		if (strcmp(XMPlayerCfg.sublang, languages[i].abbrev) == 0)
			return i;
	return 0;
}

/**
 * Load ressources
 **/
static void loadHomeRessources()
{
	home_left = new GuiImage(new GuiImageData(home_left_png));

	home_main_function_frame_bg = new GuiImage(new GuiImageData(home_main_function_frame_bg_png));
	home_main_function_frame_bg->SetPosition(0, 341);

	home_list_v = new GuiList(200, 440);
	home_list_v->SetPosition(290, 140);
	home_list_v->SetCount(4);
	home_list_v->SetCenter(3);
	home_list_v->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	home_list_h = new GuiList(465, 110, 'H');
	home_list_h->SetPosition(560, 360);
	home_list_h->SetCount(3);
	home_list_h->SetCenter(2);
	home_list_h_selector = new GuiImage(new GuiImageData(home_horizontal_select_bar_png));
	home_list_h_selector->SetPosition(715, 350);

	home_curitem = new GuiText("@@curitem", 32, 0xffffffff);
	home_curitem->SetPosition(550, 610);
	home_curitem->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	home_curitem->SetStyle(FTGX_JUSTIFY_LEFT);

	for (int i = 0; i < device_list_size; i++) {
		home_hdd_icon[i] = new GuiImage(new GuiImageData(home_hdd_sub_icon_n_png));
		home_no_hdd_icon[i] = new GuiImage(new GuiImageData(home_nohdd_sub_icon_n_png));
		home_device_btn[i] = new GuiButton(home_hdd_icon[i]->GetWidth(), home_hdd_icon[i]->GetHeight());
		home_device_btn[i]->SetIcon(home_hdd_icon[i]);
		home_device_btn[i]->SetEffectGrow();
	}


	home_video_img = new GuiImage(new GuiImageData(home_video_sm_icon_n_png));
	home_music_img = new GuiImage(new GuiImageData(home_music_sm_icon_n_png));
	home_photo_img = new GuiImage(new GuiImageData(home_photo_sm_icon_n_png));
	home_setting_img = new GuiImage(new GuiImageData(home_settings_sm_icon_n_png));

	home_video_txt = new GuiText("Videos", 48, 0xFFFFFFFF);
	home_all_txt = new GuiText("All", 48, 0xFFFFFFFF);
	home_music_txt = new GuiText("Music", 48, 0xFFFFFFFF);
	home_photo_txt = new GuiText("Photos", 48, 0xFFFFFFFF);
	home_setting_txt = new GuiText("Settings", 48, 0xFFFFFFFF);
	home_restart_txt = new GuiText("Restart", 48, 0xFFFFFFFF);
	home_shutdown_txt = new GuiText("Shutdown", 48, 0xFFFFFFFF);

	home_video_btn = new GuiButton(home_video_img->GetWidth(), home_video_img->GetHeight());
	home_all_btn = new GuiButton(home_video_img->GetWidth(), home_video_img->GetHeight());
	home_music_btn = new GuiButton(home_music_img->GetWidth(), home_music_img->GetHeight());
	home_photo_btn = new GuiButton(home_photo_img->GetWidth(), home_photo_img->GetHeight());
	home_setting_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());
	home_restart_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());
	home_shutdown_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());

	//	home_video_btn->SetIcon(home_video_img);
	//	home_music_btn->SetIcon(home_music_img);
	//	home_photo_btn->SetIcon(home_photo_img);
	//	home_setting_btn->SetIcon(home_setting_img);

	home_video_btn->SetLabel(home_video_txt);
	home_all_btn->SetLabel(home_all_txt);
	home_music_btn->SetLabel(home_music_txt);
	home_photo_btn->SetLabel(home_photo_txt);
	home_setting_btn->SetLabel(home_setting_txt);
	home_restart_btn->SetLabel(home_restart_txt);
	home_shutdown_btn->SetLabel(home_shutdown_txt);
}

static void loadBrowserRessources()
{

	// Browser
	browser_photo_icon = new GuiImageData(browser_photo_icon_f_png);
	browser_video_icon = new GuiImageData(browser_video_icon_f_png);
	browser_music_icon = new GuiImageData(browser_music_icon_f_png);

	browser_photo_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_video_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_music_folder_icon = new GuiImageData(browser_folder_icon_f_png);

	browser_selector = new GuiImageData(browser_list_btn_png);

	browser_pagecounter = new GuiText("@@pagecounter", 24, 0xFFFFFFFF);
	browser_pagecounter->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_pagecounter->SetPosition(980, 54);
	browser_pagecounter->SetMaxWidth(200);
	browser_pagecounter->SetStyle(FTGX_JUSTIFY_RIGHT);

	browser_subheadline = new GuiText("@@subheadline", 24, 0xFFFFFFFF);
	browser_subheadline->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_subheadline->SetPosition(100, 70);

	browser_headline = new GuiText("@@headline", 32, 0xfffa9600);
	browser_headline->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_headline->SetPosition(100, 40);
	browser_headline->SetEffectGrow();

	browser_sortText = new GuiText("@@sorttext", 24, 0xFFFFFFFF);
	browser_sortText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_sortText->SetPosition(550, 40);
	browser_sortText->SetEffectGrow();

	browser_sort_up = new GuiImage(new GuiImageData(browser_list_arrow_up_png));
	browser_sort_down = new GuiImage(new GuiImageData(browser_list_arrow_down_png));

	browser_sort_up->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_sort_down->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_sort_up->SetPosition(710, 45);
	browser_sort_down->SetPosition(710, 45);

	browser_up_icon = new GuiImage(new GuiImageData(browser_list_arrow_up_png));
	browser_down_icon = new GuiImage(new GuiImageData(browser_list_arrow_down_png));

	browser_up_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_down_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_up_icon->SetPosition(120, 640);
	browser_down_icon->SetPosition(1141, 640);

	browser_top_bg = new GuiImage(new GuiImageData(browser_top_png));
	browser_top_bg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_top_bg->SetPosition(0, 0);

	// no image data, pointer to image
	browser_folder_icon = browser_video_folder_icon;

	browser_files_icon[BROWSER_TYPE_UNKNOW] = new GuiImageData(browser_file_icon_f_png);
	browser_files_icon[BROWSER_TYPE_VIDEO] = new GuiImageData(browser_video_icon_f_png);
	browser_files_icon[BROWSER_TYPE_AUDIO] = new GuiImageData(browser_music_icon_f_png);
	browser_files_icon[BROWSER_TYPE_PICTURE] = new GuiImageData(browser_photo_icon_f_png);
	browser_files_icon[BROWSER_TYPE_ELF] = new GuiImageData(browser_elf_icon_f_png);
	browser_files_icon[BROWSER_TYPE_NAND] = new GuiImageData(browser_file_icon_f_png);

	gui_browser = new GuiFileBrowser(980, 500, browser_selector, browser_folder_icon, browser_files_icon);
	gui_browser->SetPosition(150, 131);
	gui_browser->SetFontSize(20);
	gui_browser->SetSelectedFontSize(24);
	gui_browser->SetPageSize(10);
}

static void ExitMplayer()
{
	// save settings
	SavePrefs(true);
	// restart to xell or shutdown, it will always restart to xell if mplayer cannot initiate
	if ((XMPlayerCfg.exit_action == 0) || (strlen(MPLAYER_CONFDIR) <= 0)) {
		exit(0);
	} else {
		xenon_smc_power_shutdown();
	}
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label)
{
	int choice = -1;

	GuiWindow promptWindow(448, 288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt(title, 26, (XeColor)
	{
		{ 255, 255, 255, 255}});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0, 14);

	GuiText msgTxt(msg, 26, (XeColor)
	{
		{ 255, 255, 255, 255}});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0, -20);
	msgTxt.SetWrap(true, 430);

	GuiText btn1Txt(btn1Label, 22, (XeColor)
	{
		{255, 255, 255, 255}});
	GuiImage btn1Img(&btnOutline);
	GuiImage btn1ImgOver(&btnOutlineOver);
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

	if (btn2Label) {
		btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn1.SetPosition(20, -25);
	} else {
		btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn1.SetPosition(0, -25);
	}

	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetImageOver(&btn1ImgOver);
	btn1.SetTrigger(trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 22, (XeColor)
	{
		{255, 0, 0, 0}});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn2.SetPosition(-20, -25);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	btn2.SetTrigger(trigA);
	btn2.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&btn1);

	if (btn2Label)
		promptWindow.Append(&btn2);

	//promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);

	if (btn2Label) {
		btn1.ResetState();
		btn1.SetState(STATE_SELECTED);
	}

	while (choice == -1) {
		update();
		if (btn1.GetState() == STATE_CLICKED)
			choice = 1;
		else if (btn2.GetState() == STATE_CLICKED)
			choice = 0;
	}

	/*promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0) {
		update();
	}*/
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	return choice;
}

extern "C" void cErrorPrompt(const char *msg)
{
	WindowPrompt("Error", msg, "Quit", NULL);
	ExitMplayer();
}

/** to do **/
static void loadRessources()
{
	loadHomeRessources();
	loadBrowserRessources();
	loadOsdRessources();
}

static void common_setup()
{
	trigA = new GuiTrigger();

	trigA->SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

	GuiImageData * background = new GuiImageData(welcome_background_bg_png);
	mainWindow = new GuiWindow(screenwidth, screenheight);
	bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);

	loadRessources();
}

static void Browser(const char * title, const char * root)
{
	int _working_menu = current_menu;
	// apply correct icon
	switch (current_menu) {
	case BROWSE_AUDIO:
		browser_folder_icon = browser_music_folder_icon;
		extValid = extIsValidAudioExt;
		break;
	case BROWSE_VIDEO:
		browser_folder_icon = browser_video_folder_icon;
		extValid = extIsValidVideoExt;
		break;
	case BROWSE_PICTURE:
		browser_folder_icon = browser_photo_folder_icon;
		extValid = extIsValidPictureExt;
		break;
	default:
		extValid = extAlwaysValid;
		break;
	}
	ResetBrowser();
	if ((strlen(exited_dir_array[current_menu]) != 0) && (exited_root == root)) {
		BrowseDevice(exited_dir_array[current_menu], root);
		gui_browser->ResetState();
		browser.selIndex = exited_item[current_menu];
		browser.pageIndex = exited_page[current_menu];
		gui_browser->fileList[exited_i[current_menu]]->SetState(STATE_SELECTED);
	} else {
		BrowseDevice("/", root);
		gui_browser->ResetState();
		gui_browser->fileList[0]->SetState(STATE_SELECTED);
	}
	gui_browser->TriggerUpdate();

	mainWindow->Append(gui_browser);

	GuiTrigger bMenu;
	bMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_B);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(50, -35);
	bBtn.SetTrigger(&bMenu);
	bBtn.SetEffectGrow();

	GuiTrigger backMenu;
	backMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_BACK);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(10, -35);
	backBtn.SetTrigger(&backMenu);
	backBtn.SetEffectGrow();

	GuiTrigger sortMenu;
	sortMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_X);

	GuiButton browser_sortBtn(20, 20);
	browser_sortBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	browser_sortBtn.SetPosition(30, -35);
	browser_sortBtn.SetTrigger(&sortMenu);
	browser_sortBtn.SetEffectGrow();

	mainWindow->Append(&bBtn);
	mainWindow->Append(&backBtn);
	browser_headline->SetText(title);
	browser_subheadline->SetText(rootdir);

	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_headline);
	mainWindow->Append(browser_subheadline);
	mainWindow->Append(browser_pagecounter);
	mainWindow->Append(&browser_sortBtn);
	mainWindow->Append(browser_sortText);

	mainWindow->Append(browser_up_icon);
	mainWindow->Append(browser_down_icon);

	mainWindow->Append(browser_sort_up);
	mainWindow->Append(browser_sort_down);

	last_menu = current_menu;
	int last_sel_item = -1;
	int last_sort = -1;
	int browser_exit = 0;
	char tmp[256];

	while (current_menu == last_menu) {
		if (last_sel_item != browser.selIndex) {
			sprintf(tmp, "%d/%d", browser.selIndex + 1, browser.numEntries);
			browser_pagecounter->SetText(tmp);
		}

		// filebrowser sort icons
		if (last_sort != XMPlayerCfg.sort_order) {
			if (XMPlayerCfg.sort_order == 0) {
				browser_sortText->SetText("Sort by: Name");
				browser_sort_up->SetVisible(true);
				browser_sort_down->SetVisible(false);
			} else if (XMPlayerCfg.sort_order == 1) {
				browser_sortText->SetText("Sort by: Name");
				browser_sort_down->SetVisible(true);
				browser_sort_up->SetVisible(false);
			} else if (XMPlayerCfg.sort_order == 2) {
				browser_sortText->SetText("Sort by: Date");
				browser_sort_up->SetVisible(true);
				browser_sort_down->SetVisible(false);
			} else if (XMPlayerCfg.sort_order == 3) {
				browser_sortText->SetText("Sort by: Date");
				browser_sort_down->SetVisible(true);
				browser_sort_up->SetVisible(false);
			}
		}

		last_sort = XMPlayerCfg.sort_order;
		last_sel_item = browser.selIndex;

		if (browser.pageIndex) {
			// draw prev
			browser_up_icon->SetVisible(true);
		} else {
			browser_up_icon->SetVisible(false);
		}
		if ((browser.numEntries > 9) && (browser.selIndex + 3 < browser.numEntries)) {
			browser_down_icon->SetVisible(true);
		} else {
			browser_down_icon->SetVisible(false);
		}


		exited_item[current_menu] = browser.selIndex;
		exited_page[current_menu] = browser.pageIndex;

		// update file browser based on arrow xenon_buttons
		// set MENU_EXIT if A xenon_button pressed on a file
		for (int i = 0; i < gui_browser->GetPageSize(); i++) {
			if (gui_browser->fileList[i]->GetState() == STATE_SELECTED) {
				exited_i[current_menu] = i;
			}
			if (gui_browser->fileList[i]->GetState() == STATE_CLICKED) {
				gui_browser->fileList[i]->ResetState();
				// check corresponding browser entry 
				if (browserList[browser.selIndex].isdir) {
					if (BrowserChangeFolder()) {
						gui_browser->ResetState();
						gui_browser->fileList[0]->SetState(STATE_SELECTED);
						gui_browser->TriggerUpdate();
						last_sel_item = -1;
					} else {
						break;
					}
				} else {
					sprintf(mplayer_filename, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);
					CleanupPath(mplayer_filename);
					sprintf(seek_filename, "%s/cache/elapsed/%s.bin", MPLAYER_CONFDIR, browserList[browser.selIndex].filename);
					ShutoffRumble();
					gui_browser->ResetState();
					if (file_type(mplayer_filename) == BROWSER_TYPE_ELF) {
						current_menu = MENU_ELF;
						/*} else if (file_type(mplayer_filename) == BROWSER_TYPE_AUDIO) {
							audio_gui = 1;
							current_menu = MENU_MPLAYER;*/
					} else {
						current_menu = MENU_MPLAYER;
						strcpy(mplayer_seek_time, "seek 0 2");
						if (file_exists(seek_filename)) {
							double seek_time = playerSeekPrompt(seek_filename);
							sprintf(mplayer_seek_time, "seek %f 2", seek_time);
							remove(seek_filename);
						}
					}
				}
			}
		}
		//Sort button selection
		if (browser_sortBtn.GetState() == STATE_CLICKED) {
			browser_sortBtn.ResetState();
			gui_browser->TriggerUpdate();
			XMPlayerCfg.sort_order++;
			if (XMPlayerCfg.sort_order > 3) {
				XMPlayerCfg.sort_order = 0;
			}
			last_sort = -1;
			BrowserSortList();

			// save pref
			SavePrefs(true);
		}
		if (bBtn.GetState() == STATE_CLICKED) {
			if (strcmp(browserList[0].filename, "..") == 0) {
				bBtn.ResetState();
				browser.selIndex = 0;
				last_sel_item = -1;
				BrowserChangeFolder();

				gui_browser->ResetState();
				gui_browser->fileList[0]->SetState(STATE_SELECTED);
				gui_browser->TriggerUpdate();
			} else {
				current_menu = MENU_BACK;
			}
		}
		if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = MENU_BACK;
		}
		update();
	}

	// exit dir						
	sprintf(exited_dir, "%s/", browser.dir);
	CleanupPath(exited_dir);
	strncpy(exited_dir_array[_working_menu], exited_dir, 2048);
	exited_root = root;

	mainWindow->Remove(browser_up_icon);
	mainWindow->Remove(browser_down_icon);

	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_headline);
	mainWindow->Remove(browser_subheadline);
	mainWindow->Remove(browser_pagecounter);
	mainWindow->Remove(gui_browser);
	mainWindow->Remove(&bBtn);
	mainWindow->Remove(&backBtn);
	mainWindow->Remove(&browser_sortBtn);
	mainWindow->Remove(browser_sortText);
	mainWindow->Remove(browser_sort_up);
	mainWindow->Remove(browser_sort_down);
}

static void HomePage()
{
	static int last_selected_value = 0; // Video

	mainWindow->Append(home_left);
	mainWindow->Append(home_main_function_frame_bg);

	home_video_txt ->SetText("Videos");
	home_all_txt ->SetText("All");
	home_music_txt ->SetText("Music");
	home_photo_txt ->SetText("Photos");
	home_setting_txt ->SetText("Settings");
	home_restart_txt ->SetText("Restart");
	home_shutdown_txt ->SetText("Shutdown");

	home_list_v->Append(home_all_btn);
	home_list_v->Append(home_video_btn);
	home_list_v->Append(home_music_btn);
	home_list_v->Append(home_photo_btn);
	home_list_v->Append(home_setting_btn);
	home_list_v->Append(home_restart_btn);
	home_list_v->Append(home_shutdown_btn);

	home_list_v->SetSelected(last_selected_value);

	mainWindow->Append(home_list_v);

	for (int i = 0; i < device_list_size; i++) {
		home_list_h->Append(home_device_btn[i]);
	}

	home_list_h->SetFocus(1);
	home_list_h->SetSelector(home_list_h_selector);

	mainWindow->Append(home_list_h);
	mainWindow->Append(home_curitem);


	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_A);

	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();

	mainWindow->Append(&menuBtn);

	static int last_device = -1;

	while (current_menu == HOME_PAGE) {

		int h_val = home_list_h->GetValue();

		if (last_device != h_val) {
			home_curitem->SetText(device_list[h_val]);
		}

		last_device = h_val;

		if (menuBtn.GetState() == STATE_CLICKED) {
			switch (home_list_v->GetValue()) {
			case 0:
				current_menu = BROWSE_ALL;
				break;
			case 1:
				current_menu = BROWSE_VIDEO;
				break;
				//				case 2:
				//					current_menu = BROWSE_AUDIO;
				//					break;
				//				case 3:
				//					current_menu = BROWSE_PICTURE;
				//					break;
			case 4:
				current_menu = SETTINGS;
				break;
			case 5:
				xenon_smc_power_reboot();
				break;
			case 6:
				ExitMplayer();
				break;
			default:
				WindowPrompt("Warning", "Not implemented yet", "Ok", NULL);
				break;
			}

			last_selected_value = home_list_v->GetValue();
		}
		update();
	}

	root_dev = device_list[home_list_h->GetValue()];

	mainWindow->Remove(&menuBtn);
	mainWindow->Remove(home_list_h);
	mainWindow->Remove(home_list_v);
	mainWindow->Remove(home_left);
	mainWindow->Remove(home_main_function_frame_bg);

	for (int i = 0; i < device_list_size; i++) {
		home_list_h->Remove(home_device_btn[i]);
	}

	home_list_v->Remove(home_video_btn);
	home_list_v->Remove(home_music_btn);
	home_list_v->Remove(home_photo_btn);
	home_list_v->Remove(home_setting_btn);
	home_list_v->Remove(home_restart_btn);
	home_list_v->Remove(home_shutdown_btn);
	mainWindow->Remove(home_curitem);
}

static void GlobalSettings()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Exit Action");
	sprintf(options.name[i++], "Language");
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Global Settings", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiText backBtnTxt("Back", 22, 0xFFFFFFFF);
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_GLOBAL) {
		update();

		ret = optionBrowser.GetClickedOption();

		switch (ret) {
		case 0:
			XMPlayerCfg.exit_action++;
			if (XMPlayerCfg.exit_action > 1)
				XMPlayerCfg.exit_action = 0;
			break;
		case 1:
			XMPlayerCfg.language++;

			if (XMPlayerCfg.language >= LANG_LENGTH)
				XMPlayerCfg.language = 0;

			break;
		}

		if (ret >= 0 || firstRun) {
			firstRun = false;

			if (XMPlayerCfg.exit_action > 1)
				XMPlayerCfg.exit_action = 0;
			if (XMPlayerCfg.exit_action == 0)
				sprintf(options.value[0], "Return to Xell");
			else
				sprintf(options.value[0], "Shutdown");

			switch (XMPlayerCfg.language) {
			case LANG_ENGLISH: sprintf(options.value[1], "English");
				gettextCleanUp();
				break;
			case LANG_FRENCH: sprintf(options.value[1], "French");
				LoadLanguage((char*) fr_lang, fr_lang_size);
				break;
			}

			optionBrowser.TriggerUpdate();
		}

		if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);

	// save settings
	SavePrefs(true);
}

static void AudioSettings()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Language");
	sprintf(options.name[i++], "Volume");
	sprintf(options.name[i++], "Soft Volume");
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;


	options.v[0].curr = GetALangIndex();
	options.v[1].curr = XMPlayerCfg.volume;
	options.v[2].curr = XMPlayerCfg.softvol;

	options.v[0].max = LANGUAGE_SIZE;
	options.v[1].max = 100;
	options.v[2].max = 100;

	GuiText titleTxt("Audio Settings", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiText backBtnTxt("Back", 22, 0xFFFFFFFF);
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_AUDIO) {
		update();

		int option_value = optionBrowser.GetClickedValueOption();
		ret = optionBrowser.GetClickedOption();

		if ((ret >= 0) && (option_value >= 0)) {
			switch (ret) {
			case 0:
			{
				sprintf(XMPlayerCfg.alang, languages[option_value].abbrev2);
				sprintf(XMPlayerCfg.alang_desc, languages[option_value].language);
				audio_lang = XMPlayerCfg.alang;
				break;
			}
			case 1:
			{
				start_volume = XMPlayerCfg.volume = options.v[1].curr;
				break;
			}
			case 2:
			{
				soft_vol_max = XMPlayerCfg.softvol = options.v[2].curr;
				break;
			}
			}
		}
		if (ret >= 0 || firstRun) {
			firstRun = false;
			sprintf(options.value[0], XMPlayerCfg.alang_desc);
			sprintf(options.value[1], "%d", XMPlayerCfg.volume);
			sprintf(options.value[2], "%d", XMPlayerCfg.softvol);
			optionBrowser.TriggerUpdate();
		}

		if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);

	// save settings
	SavePrefs(true);
}

static void VideoSettings()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	char framedrop[100] = {};
	OptionList options;

	sprintf(options.name[i++], "Frame Dropping");
	sprintf(options.name[i++], "Vsync");
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Video Settings", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiText backBtnTxt("Back", 22, 0xFFFFFFFF);
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_VIDEO) {
		update();

		ret = optionBrowser.GetClickedOption();

		switch (ret) {
		case 0:
		{
			XMPlayerCfg.framedrop++;
			if (XMPlayerCfg.framedrop > 2) {
				XMPlayerCfg.framedrop = 0;
			}
			frame_dropping = XMPlayerCfg.framedrop;
			break;
		}
		case 1:
		{
			XMPlayerCfg.vsync = !XMPlayerCfg.vsync;
			vo_vsync = XMPlayerCfg.vsync;
			break;
		}
		}
		if (ret >= 0 || firstRun) {
			firstRun = false;
			if (XMPlayerCfg.framedrop == 2) {
				strcpy(framedrop, "Hard");
			} else if (XMPlayerCfg.framedrop == 1) {
				strcpy(framedrop, "Enabled");
			} else {
				strcpy(framedrop, "Disabled");
			}
			sprintf(options.value[0], framedrop);
			sprintf(options.value[1], "%s", XMPlayerCfg.vsync == 1 ? "Enabled" : "Disabled");
		}
		optionBrowser.TriggerUpdate();
		if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);

	// save settings
	SavePrefs(true);
}

typedef struct
{
	//RRGGBB00
	unsigned int hex;
	char * string;
} color;

// tmp buffer
char unknow_color[10] = {};
color colors[] = {
	{ 0xFFFFFF00, "White"},
	{ 0x00000000, "Black"},
	{ 0xFFFF0000, "Yellow"},
	{ 0xFF000000, "Red"},
};

char * getColorFromHex(unsigned int hex, color * pColor, int max)
{
	char * dest = NULL;
	for (int i = 0; i < max; i++) {
		if (pColor[i].hex == hex) {
			dest = pColor[i].string;
			break;
		}
	}
	if (dest == NULL) {
		// set it to temp buffer
		dest = unknow_color;
		// set it to the hexadecimal value of the color
		sprintf(dest, "%08x", hex);
	}
	return dest;
}

unsigned int getColorFromString(char * str, color * pColor, int max)
{
	unsigned int dest = 0;
	for (int i = 0; i < max; i++) {
		if (strcmp(str, pColor[i].string) == 0) {
			dest = pColor[i].hex;
			break;
		}
	}
	return dest;
}

/*
int getColorIndex(char *str, color * pColor, int max) {
	for (int i=0; i<max; i++)
	{
		if (strcmp(str, pColor[i].string) == 0) {
			return i;
		}
	}	
	return -1;
}
 * */
int getColorIndex(unsigned int hex, color * pColor, int max)
{
	for (int i = 0; i < max; i++) {
		if (pColor[i].hex == hex) {
			return i;
		}
	}
	return -1;
}

static void SubtitleSettings()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	int nbColors = sizeof (colors) / sizeof (color);
	printf("nbColors : %d\n", nbColors);
	sprintf(options.name[i++], "Color");
	sprintf(options.name[i++], "Border Color");
	sprintf(options.name[i++], "Code Page");
	sprintf(options.name[i++], "Language");
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	options.v[0].curr = getColorIndex(XMPlayerCfg.subcolor, colors, sizeof (colors));
	options.v[1].curr = getColorIndex(XMPlayerCfg.border_color, colors, sizeof (colors));
	options.v[2].curr = GetCodepageIndex();
	options.v[3].curr = GetLangIndex();

	options.v[0].max = nbColors;
	options.v[1].max = nbColors;
	options.v[2].max = CODEPAGE_SIZE;
	options.v[3].max = LANGUAGE_SIZE;

	GuiText titleTxt("Subtitle Settings", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiText backBtnTxt("Back", 22, 0xFFFFFFFF);
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_SUBTITLES) {
		update();
		int option_value = optionBrowser.GetClickedValueOption();
		ret = optionBrowser.GetClickedOption();
		if (ret >= 0) {
			// option_value should always been >= 0
			printf("option_value : %d\n", option_value);
			if (option_value >= 0) {
				switch (ret) {
				case 0:
				{
					sprintf(_ass_color, "%08x", colors[option_value].hex);
					XMPlayerCfg.subcolor = colors[option_value].hex;
					break;
				}
				case 1:
				{
					sprintf(_ass_border_color, "%08x", colors[option_value].hex);
					XMPlayerCfg.border_color = colors[option_value].hex;
					break;
				}
				case 2:
				{
					strcpy(XMPlayerCfg.subcp, codepages[option_value].cpname);
					strcpy(XMPlayerCfg.subcp_desc, codepages[option_value].language);
					sub_cp = XMPlayerCfg.subcp;
					break;
				}
				case 3:
				{
					strcpy(XMPlayerCfg.sublang, languages[option_value].abbrev);
					strcpy(XMPlayerCfg.sublang_desc, languages[option_value].language);
					dvdsub_lang = XMPlayerCfg.sublang;
					break;
				}
				}
			}
		}
		if (ret >= 0 || firstRun) {
			firstRun = false;

			strcpy(options.value[0], getColorFromHex(XMPlayerCfg.subcolor, colors, sizeof (colors)));
			strcpy(options.value[1], getColorFromHex(XMPlayerCfg.border_color, colors, sizeof (colors)));

			strcpy(options.value[2], XMPlayerCfg.subcp_desc);
			strcpy(options.value[3], XMPlayerCfg.sublang_desc);
			optionBrowser.TriggerUpdate();
		}

		if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		}
	}
	ass_force_reload = 1;
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);

	// save settings
	SavePrefs(true);
}

//SETTINGS MENU

static void XMPSettings()
{
	int ret;
	int i = 0;
	OptionList options;

	sprintf(options.name[i++], "Global");
	sprintf(options.name[i++], "Audio");
	sprintf(options.name[i++], "Video");
	sprintf(options.name[i++], "Subtitles");
	sprintf(options.name[i++], "Network");
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Settings", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiText backBtnTxt("Back", 22, 0xFFFFFFFF);
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetTrigger(trigA);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS) {
		update();

		ret = optionBrowser.GetClickedOption();

		switch (ret) {
		case 0:
			current_menu = SETTINGS_GLOBAL;
			break;
		case 1:
			current_menu = SETTINGS_AUDIO;
			break;
		case 2:
			current_menu = SETTINGS_VIDEO;
			break;
		case 3:
			current_menu = SETTINGS_SUBTITLES;
			break;
			/*		case 4:
						current_menu = SETTINGS_NETWORK;
						break;  					*/
		}

		if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
}

static void init_mplayer_settings(void)
{
	ass_color = _ass_color;
	ass_border_color = _ass_border_color;
	sub_cp = XMPlayerCfg.subcp;
	dvdsub_lang = XMPlayerCfg.sublang;
	audio_lang = XMPlayerCfg.alang;
}

static void do_mplayer(char * filename)
{
	static int mplayer_need_init = 1;
	if (mplayer_need_init) {
		char vsync[50] = "-novsync";
		char framedrop[50] = "-noframedrop";

		if (XMPlayerCfg.vsync == 1)
			strcpy(vsync, "-vsync");
		else
			strcpy(vsync, "-vsync");

		if (XMPlayerCfg.framedrop == 1)
			strcpy(framedrop, "-framedrop");
		else if (XMPlayerCfg.framedrop == 2)
			strcpy(framedrop, "-hardframedrop");

		char * argv[] = {
			"mplayer.xenon",
			vsync,
			framedrop,
			"-lavdopts", "skiploopfilter=all:threads=5",
			filename,
		};
		mplayer_need_init = 0;
		int argc = sizeof (argv) / sizeof (char *);

		mplayer_main(argc, argv);
		// will never be here !!!
	} else {
		mplayer_load(filename);
		mplayer_return_to_player();
	}
}

void MenuMplayer()
{
	//sprintf(foldername, "%s/", browser.dir);
	printf("filename:%s\r\n", mplayer_filename);
	do_mplayer(mplayer_filename);
}

void ElfLoader()
{
	printf("Load Elf:%s\r\n", mplayer_filename);
	char * argv[] = {
		mplayer_filename,
	};
	int argc = sizeof (argv) / sizeof (char *);

	elf_setArgcArgv(argc, argv);
	elf_runFromDisk(mplayer_filename);
}

static void gui_loop()
{
	while (need_gui) {
		if (current_menu == HOME_PAGE) {
			HomePage();
		} else if (current_menu == BROWSE_VIDEO) {
			Browser("Videos", root_dev);
		} else if (current_menu == BROWSE_AUDIO) {
			Browser("Music", root_dev);
		} else if (current_menu == BROWSE_PICTURE) {
			Browser("Photos", root_dev);
		} else if (current_menu == BROWSE_ALL) {
			Browser("All", root_dev);
		} else if (current_menu == MENU_MPLAYER) {
			MenuMplayer();
		} else if (current_menu == MENU_BACK) {
			current_menu = HOME_PAGE;
		} else if (current_menu == MENU_ELF) {
			ElfLoader();
		} else if (current_menu == SETTINGS) {
			XMPSettings();
		} else if (current_menu == SETTINGS_GLOBAL) {
			GlobalSettings();
		} else if (current_menu == SETTINGS_SUBTITLES) {
			SubtitleSettings();
		} else if (current_menu == SETTINGS_AUDIO) {
			AudioSettings();
		} else if (current_menu == SETTINGS_VIDEO) {
			VideoSettings();
		} /*else if (current_menu == SETTINGS_NETWORK) {
			NetworkSettings();
		} */
	}
}

static void findDevices()
{
	for (int i = 3; i < STD_MAX; i++) {
		if (devoptab_list[i]->structSize) {
			//strcpy(device_list[device_list_size],devoptab_list[i]->name);
			sprintf(device_list[device_list_size], "%s:/", devoptab_list[i]->name);
			printf("findDevices : %s\r\n", device_list[device_list_size]);
			device_list_size++;
		}
	}

	root_dev = device_list[0];
}

void loadingThread()
{
	int i = 0;
	logo = loadPNGFromMemory((unsigned char*) logo_png);
	loading[0] = loadPNGFromMemory((unsigned char*) loading_0_png);
	loading[1] = loadPNGFromMemory((unsigned char*) loading_1_png);
	loading[2] = loadPNGFromMemory((unsigned char*) loading_2_png);
	loading[3] = loadPNGFromMemory((unsigned char*) loading_3_png);

	while (end_loading_thread == 0) {
		lock(&loadingThreadLock);
		Xe_SetClearColor(g_pVideoDevice, 0xFFFFFFFF);
		Menu_DrawImg(0, 0, 1280, 720, logo, 0, 1, 1, 0xff);
		Menu_DrawImg(640 - 55, 500, 110, 110, loading[i], 0, 1, 1, 0xff);
		Menu_Render();
		unlock(&loadingThreadLock);

		mdelay(60);

		i++;
		if (i >= 4)
			i = 0;
	}
	lock(&loadingThreadLock);
	loading_thread_finished = 1;
	unlock(&loadingThreadLock);
}

int main(int argc, char** argv)
{
	xenon_make_it_faster(XENON_SPEED_FULL);
	//	
	// Init Video
	InitVideo();

	/** loading **/
	end_loading_thread = 0;

	// run in a thread ....
	xenon_run_thread_task(2, thread_stack[2], (void*) loadingThread);

	// Init devices
	usb_init();
	xenon_ata_init();
	xenon_atapi_init();
	usb_do_poll();

	// fs
	mount_all_devices();
	findDevices();

	InitFreeType((u8*) font_ttf, font_ttf_size);
	SetupPads();
	ChangeFontSize(26);
	common_setup();

	// signal end of loading thread
	lock(&loadingThreadLock);
	end_loading_thread = 1;
	unlock(&loadingThreadLock);
	while (loading_thread_finished == 0) {
		lock(&loadingThreadLock);
		udelay(25);
		unlock(&loadingThreadLock);
	}

	// init mplayer
	init_mplayer();
	init_mplayer_settings();
	
	// preference
	if (LoadPrefs() == false)
		SavePrefs(true);

	// Init gui
	switch (XMPlayerCfg.language) {
	case LANG_FRENCH:
		LoadLanguage((char*) fr_lang, fr_lang_size);
		break;
	}

	current_menu = HOME_PAGE;

	while (1) {
		// never exit !!
		need_gui = 1;
		gui_loop();
	}

	return (EXIT_SUCCESS);
}

/**
 * return to gui - doesn't exit mplayer process
 */
extern "C" void mplayer_return_to_gui()
{
	need_gui = 1;

	// always sync
	Xe_Sync(g_pVideoDevice);
	while (!Xe_IsVBlank(g_pVideoDevice));
	Xe_InvalidateState(g_pVideoDevice);

	// make sur to leave the gui
	mplayer_osd_close();
	resetController(); //resets buttons, so pushes from mplayer doesn't get reconized in browser
	current_menu = last_menu;

	gui_loop();

}

/**
 * return to mplayer
 */
extern "C" void mplayer_return_to_player()
{
	need_gui = 0;
}
