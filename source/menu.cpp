/*
 * File:   newmain.c
 * Author: cc
 *
 * Created on 27 avril 2012, 19:58
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
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
// Device information
//**************************************************************************
static std::string root_dev;
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
GuiTrigger * trigB;
GuiTrigger * trigBk;

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
static GuiImage * browser_bottom_bg = NULL;

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

/**
 * Xbox 360 buttons
 **/
static GuiText * btn_a_text = NULL;
static GuiText * btn_b_text = NULL;
static GuiText * btn_x_text = NULL;
static GuiText * btn_y_text = NULL;
static GuiText * btn_bk_text = NULL;

static GuiImage * btn_a = NULL;
static GuiImage * btn_b = NULL;
static GuiImage * btn_x = NULL;
static GuiImage * btn_y = NULL;
static GuiImage * btn_bk = NULL;

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
static std::string exited_root;
static char exited_dir[MAXPATHLEN];
static char exited_dir_array[64][MAXPATHLEN];
static int exited_item[64];
static int exited_page[64];
static int exited_i[64];

//network
static int shareId = -1;

static int last_menu;
static int current_menu = HOME_PAGE;

static void Update()
{
	UpdatePads();
	mainWindow->Draw();
	Menu_Render();
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
}

static void ResetController()
{
	int i = 0;
	for (i = 0; i < 4; i++) {
		struct controller_data_s ctrl_zero = {};
		set_controller_data(i, &ctrl_zero);
	}
}

/**
 * Load ressources
 **/
static void LoadHomeRessources()
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
	//home_music_img = new GuiImage(new GuiImageData(home_music_sm_icon_n_png));
	//home_photo_img = new GuiImage(new GuiImageData(home_photo_sm_icon_n_png));
	home_setting_img = new GuiImage(new GuiImageData(home_settings_sm_icon_n_png));

	home_video_txt = new GuiText("Videos", 48, 0xFFFFFFFF);
	home_all_txt = new GuiText("All", 48, 0xFFFFFFFF);
	//home_music_txt = new GuiText("Music", 48, 0xFFFFFFFF);
	//home_photo_txt = new GuiText("Photos", 48, 0xFFFFFFFF);
	home_setting_txt = new GuiText("Settings", 48, 0xFFFFFFFF);
	home_restart_txt = new GuiText("Restart", 48, 0xFFFFFFFF);
	home_shutdown_txt = new GuiText("Shutdown", 48, 0xFFFFFFFF);

	home_video_btn = new GuiButton(home_video_img->GetWidth(), home_video_img->GetHeight());
	home_all_btn = new GuiButton(home_video_img->GetWidth(), home_video_img->GetHeight());
	//home_music_btn = new GuiButton(home_music_img->GetWidth(), home_music_img->GetHeight());
	//home_photo_btn = new GuiButton(home_photo_img->GetWidth(), home_photo_img->GetHeight());
	home_setting_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());
	home_restart_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());
	home_shutdown_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());

	//	home_video_btn->SetIcon(home_video_img);
	//	home_music_btn->SetIcon(home_music_img);
	//	home_photo_btn->SetIcon(home_photo_img);
	//	home_setting_btn->SetIcon(home_setting_img);

	home_video_btn->SetLabel(home_video_txt);
	home_all_btn->SetLabel(home_all_txt);
	//home_music_btn->SetLabel(home_music_txt);
	//home_photo_btn->SetLabel(home_photo_txt);
	home_setting_btn->SetLabel(home_setting_txt);
	home_restart_btn->SetLabel(home_restart_txt);
	home_shutdown_btn->SetLabel(home_shutdown_txt);
}

static void LoadBrowserRessources()
{

	// Browser
	browser_photo_icon = new GuiImageData(browser_photo_icon_f_png);
	browser_video_icon = new GuiImageData(browser_video_icon_f_png);
	browser_music_icon = new GuiImageData(browser_music_icon_f_png);

	browser_photo_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_video_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_music_folder_icon = new GuiImageData(browser_folder_icon_f_png);

	browser_selector = new GuiImageData(browser_list_btn_png);

	browser_pagecounter = new GuiText("@@pagecounter", 18, 0xFFFFFFFF);
	browser_pagecounter->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_pagecounter->SetPosition(1080, 643);
	browser_pagecounter->SetMaxWidth(200);
//	browser_pagecounter->SetStyle(FTGX_JUSTIFY_RIGHT);

	browser_subheadline = new GuiText("@@subheadline", 18, 0xFFFFFFFF);
	browser_subheadline->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_subheadline->SetPosition(150, 60);

	browser_headline = new GuiText("@@headline", 24, 0xfffa9600);
	browser_headline->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_headline->SetPosition(150, 35);
	browser_headline->SetEffectGrow();

	browser_sortText = new GuiText("@@sorttext", 18, 0xFFFFFFFF);
	browser_sortText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_sortText->SetPosition(1080, 60);
	browser_sortText->SetEffectGrow();

	browser_sort_up = new GuiImage(new GuiImageData(browser_list_arrow_up_png));
	browser_sort_down = new GuiImage(new GuiImageData(browser_list_arrow_down_png));

	browser_sort_up->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_sort_down->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_sort_up->SetPosition(1133, 62);
	browser_sort_down->SetPosition(1133, 62);

	browser_up_icon = new GuiImage(new GuiImageData(browser_list_arrow_up_png));
	browser_down_icon = new GuiImage(new GuiImageData(browser_list_arrow_down_png));

	browser_up_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_down_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_up_icon->SetPosition(120, 620);
	browser_down_icon->SetPosition(1142, 620);

	browser_top_bg = new GuiImage(new GuiImageData(browser_top_png));
	browser_top_bg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_top_bg->SetPosition(0, 0);
	
	browser_bottom_bg = new GuiImage(new GuiImageData(browser_bottom_png));
	browser_bottom_bg->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	browser_bottom_bg->SetPosition(0, 0);

	// no image data, pointer to image
	browser_folder_icon = browser_video_folder_icon;

	browser_files_icon[BROWSER_TYPE_UNKNOW] = new GuiImageData(browser_file_icon_f_png);
	browser_files_icon[BROWSER_TYPE_VIDEO] = new GuiImageData(browser_video_icon_f_png);
	browser_files_icon[BROWSER_TYPE_AUDIO] = new GuiImageData(browser_music_icon_f_png);
	browser_files_icon[BROWSER_TYPE_PICTURE] = new GuiImageData(browser_photo_icon_f_png);
	browser_files_icon[BROWSER_TYPE_ELF] = new GuiImageData(browser_elf_icon_f_png);
	browser_files_icon[BROWSER_TYPE_NAND] = new GuiImageData(browser_file_icon_f_png);

	gui_browser = new GuiFileBrowser(980, 500, browser_selector, browser_folder_icon, browser_files_icon);
	gui_browser->SetPosition(150, 120);
	gui_browser->SetFontSize(20);
	gui_browser->SetSelectedFontSize(24);
	gui_browser->SetPageSize(15);
}

static void LoadXboxButtons()
{
	btn_a = new GuiImage(new GuiImageData(btn_a_png));
	btn_a->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_a->SetPosition(150, 641);
	
	btn_a_text = new GuiText("Select", 18, 0xFFFFFFFF);
	btn_a_text->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_a_text->SetPosition(185, 643);
	btn_a_text->SetEffectGrow();	

	btn_b = new GuiImage(new GuiImageData(btn_b_png));
	btn_b->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_b->SetPosition(275, 641);

	btn_b_text = new GuiText("Back", 18, 0xFFFFFFFF);
	btn_b_text->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_b_text->SetPosition(310, 643);
	btn_b_text->SetEffectGrow();		
	
	btn_x = new GuiImage(new GuiImageData(btn_x_png));
	btn_x->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_x->SetPosition(400, 641);

	btn_x_text = new GuiText("Sort", 18, 0xFFFFFFFF);
	btn_x_text->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_x_text->SetPosition(435, 643);
	btn_x_text->SetEffectGrow();		

	btn_bk = new GuiImage(new GuiImageData(btn_bk_png));
	btn_bk->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_bk->SetPosition(525, 641);			
	
	btn_bk_text = new GuiText("Home", 18, 0xFFFFFFFF);
	btn_bk_text->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	btn_bk_text->SetPosition(560, 643);
	btn_bk_text->SetEffectGrow();	
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
int WindowPrompt(const std::string title, const std::string msg, const std::string btn1Label, const std::string btn2Label)
{
	int choice = -1;

	GuiWindow promptWindow(448, 288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt(title.c_str(), 26, (XeColor)
	{
		{ 255, 255, 255, 255}});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0, 14);

	GuiText msgTxt(msg.c_str(), 26, (XeColor)
	{
		{ 255, 255, 255, 255}});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0, -20);
	msgTxt.SetWrap(true, 430);

	GuiText btn1Txt(btn1Label.c_str(), 22, (XeColor)
	{
		{255, 255, 255, 255}});
	GuiImage btn1Img(&btnOutline);
	GuiImage btn1ImgOver(&btnOutlineOver);
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

	if (!btn2Label.empty()) {
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

	GuiText btn2Txt(btn2Label.c_str(), 22, (XeColor)
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

	if (!btn2Label.empty())
		promptWindow.Append(&btn2);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);

	if (!btn2Label.empty()) {
		btn1.ResetState();
		btn1.SetState(STATE_SELECTED);
	}

	while (choice == -1) {
		Update();
		if (btn1.GetState() == STATE_CLICKED)
			choice = 1;
		else if (btn2.GetState() == STATE_CLICKED)
			choice = 0;
	}

	/*promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0) {
		Update();
	}*/
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	return choice;
}

int SmallWindowPrompt(const std::string btn1Label, const std::string btn2Label)
{
	int choice = -1;
	GuiWindow promptWindow(300, 72);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, 0);
	GuiImageData btnOutline(button_p_seek_png);
	GuiImageData btnOutlineOver(button_p_seek_select_png);

	GuiImageData dialogBox(p_seek_bg_png);
	GuiImage dialogBoxImg(&dialogBox);
	dialogBoxImg.SetPosition(0, 20);
	dialogBoxImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	GuiText btn1Txt(btn1Label.c_str(), 22, (XeColor) {{255, 255, 255, 255 }});
	GuiImage btn1Img(&btnOutline);
	GuiImage btn1ImgOver(&btnOutlineOver);
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	btn1.SetPosition(0, -22);
	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetImageOver(&btn1ImgOver);
	btn1.SetTrigger(trigA);
	btn1.SetState(STATE_SELECTED);

	GuiText btn2Txt(btn2Label.c_str(), 22, (XeColor){{255, 255, 255, 255}});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	btn2.SetPosition(0, 22);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	btn2.SetTrigger(trigA);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&btn1);
	promptWindow.Append(&btn2);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);

	btn1.SetState(STATE_SELECTED);
	btn2.ResetState();
	while (choice == -1) {
		Update();		
		if (btn1.GetState() == STATE_CLICKED)
			choice = 1;
		else if (btn2.GetState() == STATE_CLICKED) 
			choice = 0;
	}
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	return choice;
}
/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
static void OnScreenKeyboard(char * var, u32 maxlen) {
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiText okBtnTxt("OK", 22, (XeColor) {
		0, 0, 0, 255
	});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetTrigger(trigA);
	okBtn.SetTrigger(trigB);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (XeColor) {
		0, 0, 0, 255
	});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetTrigger(trigA);
	cancelBtn.SetTrigger(trigB);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	while (save == -1) {
		if (okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if (cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if (save) {
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
}

extern "C" void error_prompt(std::string msg)
{
	WindowPrompt("Error", msg, "Quit", NULL);
	ExitMplayer();
}

/** to do **/
static void LoadRessources()
{
	LoadXboxButtons();
	LoadHomeRessources();
	LoadBrowserRessources();
	LoadOsdRessources();
}

static void CommonSetup()
{
	trigA = new GuiTrigger();
	trigA->SetSimpleTrigger(-1, 0, PAD_BUTTON_A);
	
	trigB = new GuiTrigger();
	trigB->SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_B);	

	trigBk = new GuiTrigger();
	trigBk->SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_BACK);	

	GuiImageData * background = new GuiImageData(welcome_background_bg_png);
	mainWindow = new GuiWindow(screenwidth, screenheight);
	bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);

	LoadRessources();
}


static void StringRemplaceAll(std::string & src, const std::string find, const std::string replace) 
{
	size_t pos = 0;
	while((pos = src.find(find, pos)) != std::string::npos)
	{
		src.replace(pos, find.length(), replace);
		pos += replace.length();
	}
}

static void Browser(const std::string title, const std::string root)
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
		BrowseDevice(exited_dir_array[current_menu], root.c_str());
		gui_browser->ResetState();
		browser.selIndex = exited_item[current_menu];
		browser.pageIndex = exited_page[current_menu];
		gui_browser->fileList[exited_i[current_menu]]->SetState(STATE_SELECTED);
	} else {
		BrowseDevice("/", root.c_str());
		gui_browser->ResetState();
		gui_browser->fileList[0]->SetState(STATE_SELECTED);
	}
	gui_browser->TriggerUpdate();

	mainWindow->Append(gui_browser);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(50, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(10, -35);
	backBtn.SetTrigger(trigBk);

	GuiTrigger sortMenu;
	sortMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_X);

	GuiButton browser_sortBtn(20, 20);
	browser_sortBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	browser_sortBtn.SetPosition(30, -35);
	browser_sortBtn.SetTrigger(&sortMenu);

	mainWindow->Append(&bBtn);
	mainWindow->Append(&backBtn);
	browser_headline->SetText(title.c_str());
	browser_subheadline->SetText(rootdir);

	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_headline);
	mainWindow->Append(browser_subheadline);
	mainWindow->Append(&browser_sortBtn);
	mainWindow->Append(browser_sortText);
	mainWindow->Append(browser_up_icon);
	mainWindow->Append(browser_down_icon);
	mainWindow->Append(browser_sort_up);
	mainWindow->Append(browser_sort_down);

	btn_bk->SetPosition(530, 641);
	btn_bk_text->SetPosition(565, 643);

	mainWindow->Append(browser_bottom_bg);
	mainWindow->Append(browser_pagecounter);
	mainWindow->Append(btn_a);
	mainWindow->Append(btn_b);
	mainWindow->Append(btn_x);
	mainWindow->Append(btn_bk);					
	mainWindow->Append(btn_a_text);
	mainWindow->Append(btn_b_text);
	mainWindow->Append(btn_x_text);
	mainWindow->Append(btn_bk_text);							
	last_menu = current_menu;
	int last_sel_item = -1;
	int last_sort = -1;
	char tmp[256];
	char* ext = NULL;
	while (current_menu == last_menu) {
		if (last_sel_item != browser.selIndex) {
			sprintf(tmp, "%d/%d", browser.selIndex + 1, browser.numEntries);
			browser_pagecounter->SetText(tmp);
			std::string dir;
			char _dir[MAXPATHLEN];
						
			sprintf(_dir, "%s/%s", rootdir, browser.dir);
			CleanupPath(_dir);
			if (strlen(_dir) > 100) {
				_dir[99] = '\0';
				strcat(_dir, "...");
			}	
			dir = _dir;
						
			StringRemplaceAll(dir, "/", " > ");
			StringRemplaceAll(dir, ":", "");	
			browser_subheadline->SetText(dir.c_str());					
		}

		// filebrowser sort icons
		if (last_sort != XMPlayerCfg.sort_order) {
			if (XMPlayerCfg.sort_order == 0) {
				browser_sortText->SetText("Name");
				browser_sort_up->SetVisible(true);
				browser_sort_down->SetVisible(false);
			} else if (XMPlayerCfg.sort_order == 1) {
				browser_sortText->SetText("Name");
				browser_sort_down->SetVisible(true);
				browser_sort_up->SetVisible(false);
			} else if (XMPlayerCfg.sort_order == 2) {
				browser_sortText->SetText("Date");
				browser_sort_up->SetVisible(true);
				browser_sort_down->SetVisible(false);
			} else if (XMPlayerCfg.sort_order == 3) {
				browser_sortText->SetText("Date");
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
						current_menu = MENU_MPLAYER; */
					} else if (file_type(mplayer_filename) == BROWSER_TYPE_VIDEO) {	
					    	ext = strrchr(mplayer_filename, '.');
						if (strcmp(ext, ".rar") == 0) { 	
							ext = playerGetRarExt(mplayer_filename);
							if (strcmp(ext, "stop") == 0) { 
								printf("[browser] Unsupported .rar file(s) \n");
								goto rar_skip;
							}
						}
						current_menu = MENU_MPLAYER;
						strcpy(mplayer_seek_time, "seek 0 2");
						if (file_exists(seek_filename)) {
							double seek_time = playerSeekPrompt(seek_filename);
							sprintf(mplayer_seek_time, "seek %f 2", seek_time);
							remove(seek_filename);
						}
					} else {
rar_skip:						gui_browser->fileList[exited_i[current_menu]]->SetState(STATE_SELECTED);	
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
		Update();
	}
	delete ext; 
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
	mainWindow->Remove(btn_a);
	mainWindow->Remove(btn_b);
	mainWindow->Remove(btn_x);
	mainWindow->Remove(btn_bk);					
	mainWindow->Remove(btn_a_text);
	mainWindow->Remove(btn_b_text);
	mainWindow->Remove(btn_x_text);
	mainWindow->Remove(btn_bk_text);
	mainWindow->Remove(browser_bottom_bg);								
}

static void HomePage()
{
	static int last_selected_value = 0; // Video

	mainWindow->Append(home_left);
	mainWindow->Append(home_main_function_frame_bg);

	home_video_txt ->SetText("Videos");
	home_all_txt ->SetText("All");
	//home_music_txt ->SetText("Music");
	//home_photo_txt ->SetText("Photos");
	home_setting_txt ->SetText("Settings");
	home_restart_txt ->SetText("Restart");
	home_shutdown_txt ->SetText("Shutdown");

	home_list_v->Append(home_all_btn);
	home_list_v->Append(home_video_btn);
	//home_list_v->Append(home_music_btn);
	//home_list_v->Append(home_photo_btn);
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
			case 2:
				current_menu = SETTINGS;
				break;
			case 3:
				xenon_smc_power_reboot();
				break;
			case 4:
				ExitMplayer();
				break;
			default:
				WindowPrompt("Warning", "Not implemented yet", "Ok", NULL);
				break;
			}

			last_selected_value = home_list_v->GetValue();
		}
		Update();
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
	//home_list_v->Remove(home_music_btn);
	//home_list_v->Remove(home_photo_btn);
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

	GuiText titleTxt("Home > Settings > Global", 24, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(150, 35);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(10, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetTrigger(trigBk);

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	btn_bk->SetPosition(400, 641);
	btn_bk_text->SetPosition(435, 643);
	w.Append(&bBtn);
	w.Append(&backBtn);
	w.Append(btn_a);
	w.Append(btn_a_text);	
	w.Append(btn_b);
	w.Append(btn_b_text);
	w.Append(btn_bk);
	w.Append(btn_bk_text);	
	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_bottom_bg);	
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_GLOBAL) {
		Update();

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

		if (bBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		} else if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_bottom_bg);
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

	options.v[0].curr = GetAudioLangIndex();
	options.v[1].curr = XMPlayerCfg.volume;
	options.v[2].curr = XMPlayerCfg.softvol;

	options.v[0].max = LANGUAGE_SIZE;
	options.v[1].max = 101;
	options.v[2].max = 1001;

	GuiText titleTxt("Home > Settings > Audio", 24, 0xfffa9600);

	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(150, 35);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(10, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetTrigger(trigBk);

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	btn_bk->SetPosition(400, 641);
	btn_bk_text->SetPosition(435, 643);
	w.Append(&bBtn);
	w.Append(&backBtn);
	w.Append(btn_a);
	w.Append(btn_a_text);	
	w.Append(btn_b);
	w.Append(btn_b_text);
	w.Append(btn_bk);
	w.Append(btn_bk_text);	
	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_bottom_bg);	
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_AUDIO) {
		Update();

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

		if (bBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		} else if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_bottom_bg);
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

	GuiText titleTxt("Home > Settings >  Video", 24, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(150, 35);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(10, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetTrigger(trigBk);

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	btn_bk->SetPosition(400, 641);
	btn_bk_text->SetPosition(435, 643);
	w.Append(&bBtn);
	w.Append(&backBtn);
	w.Append(btn_a);
	w.Append(btn_a_text);	
	w.Append(btn_b);
	w.Append(btn_b_text);
	w.Append(btn_bk);
	w.Append(btn_bk_text);
	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_bottom_bg);		
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_VIDEO) {
		Update();

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
			optionBrowser.TriggerUpdate();
		}
		
		if (bBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		} else if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_bottom_bg);
	// save settings
	SavePrefs(true);
}

static void SubtitleSettings()
{
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

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
	options.v[3].curr = GetSubLangIndex();

	options.v[0].max = NB_COLOR;
	options.v[1].max = NB_COLOR;
	options.v[2].max = CODEPAGE_SIZE;
	options.v[3].max = LANGUAGE_SIZE;

	GuiText titleTxt("Home > Settings > Subtitle", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(150, 35);

	GuiText subNote("Note: Color and Border Color needs ass=yes (unstable)!", 16, 0xfffa9600);
	subNote.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	subNote.SetPosition(160, 290);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(10, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetTrigger(trigBk);

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	btn_bk->SetPosition(400, 641);
	btn_bk_text->SetPosition(435, 643);								
	w.Append(&bBtn);
	w.Append(&backBtn);
	w.Append(btn_a);
	w.Append(btn_a_text);	
	w.Append(btn_b);
	w.Append(btn_b_text);
	w.Append(btn_bk);
	w.Append(btn_bk_text);	
	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_bottom_bg);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	mainWindow->Append(&subNote);

	while (current_menu == SETTINGS_SUBTITLES) {
		Update();
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
					ass_color = _ass_color;
					break;
				}
				case 1:
				{
					sprintf(_ass_border_color, "%08x", colors[option_value].hex);
					XMPlayerCfg.border_color = colors[option_value].hex;
					ass_border_color = _ass_border_color;					
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

		if (bBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		} else if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = HOME_PAGE;
		}
	}
	ass_force_reload = 1;
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(&subNote);
	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_bottom_bg);
	// save settings
	SavePrefs(true);
}

static void NetworkSettingsSMB() {
	int ret;
	int i = 0;
	bool firstRun = true;
	char sharename[100];
	OptionList options;
	sprintf(options.name[i++], "Display Name");
	sprintf(options.name[i++], "Share IP");
	sprintf(options.name[i++], "Share Name");
	sprintf(options.name[i++], "Username");
	sprintf(options.name[i++], "Password");	
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	if (shareId < 0) {
		sprintf(sharename, "New Share");
		for (int j = 0; j < MAX_SHARES; j++) { //find id for new share
			if (XMPlayerCfg.smb[j].share[0] == 0) {
				shareId = j;
				break;
			}
		}
	} else if (strlen(XMPlayerCfg.smb[shareId].name) > 0) {
		sprintf(sharename, XMPlayerCfg.smb[shareId].name);
	} else {
		sprintf(sharename, XMPlayerCfg.smb[shareId].share);
	}
	printf("[Network Settings] SMB ShareId: %d \n", shareId);
		
	GuiText titleTxt("Home > Settings > Network > SMB", 24, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(150, 35);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(10, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetTrigger(trigBk);

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	btn_bk->SetPosition(400, 641);
	btn_bk_text->SetPosition(435, 643);								
	w.Append(&bBtn);
	w.Append(&backBtn);
	w.Append(btn_a);
	w.Append(btn_a_text);	
	w.Append(btn_b);
	w.Append(btn_b_text);
	w.Append(btn_bk);
	w.Append(btn_bk_text);	
	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_bottom_bg);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_NETWORK_SMB) {
		Update();

		ret = optionBrowser.GetClickedOption();

		switch (ret) {
			case 0: {
				OnScreenKeyboard(XMPlayerCfg.smb[shareId].name, 40);
				break;
				}
			case 1: {
				OnScreenKeyboard(XMPlayerCfg.smb[shareId].ip, 80);
				break;
				}
			case 2: {	
				OnScreenKeyboard(XMPlayerCfg.smb[shareId].share, 80);
				break;
				} 
			case 3:	{
				OnScreenKeyboard(XMPlayerCfg.smb[shareId].user, 25);			
				break;
				}
			case 4:	{
				OnScreenKeyboard(XMPlayerCfg.smb[shareId].pass, 25);			
				break;
				}				
		}		
		if (ret >= 0 || firstRun) {
			firstRun = false;
			sprintf(options.value[0], sharename);
			sprintf(options.value[1], XMPlayerCfg.smb[shareId].ip);
			sprintf(options.value[2], XMPlayerCfg.smb[shareId].share);
			sprintf(options.value[3], XMPlayerCfg.smb[shareId].user);
			sprintf(options.value[4], XMPlayerCfg.smb[shareId].pass);
			optionBrowser.TriggerUpdate();
		}

		if (bBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS_NETWORK;
		} else if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_bottom_bg);
	//save settings
	//SavePrefs(true);
}

static void NetworkSettings() {
	int ret;
	int i = 0;
	OptionList options;
	for (int j = 0; j < MAX_SHARES; j++) {
		if (strlen(XMPlayerCfg.smb[j].share) > 0) {
			if (strlen(XMPlayerCfg.smb[j].name) > 0) {
				sprintf(options.name[i], XMPlayerCfg.smb[j].name);
			} else {
				sprintf(options.name[i], XMPlayerCfg.smb[j].share);
			}
		i++;	
		}
	}
	sprintf(options.name[i++], "Add");
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Home > Settings > Network", 24, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(150, 35);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(10, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetTrigger(trigBk);

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	btn_bk->SetPosition(400, 641);
	btn_bk_text->SetPosition(435, 643);								
	w.Append(&bBtn);
	w.Append(&backBtn);
	w.Append(btn_a);
	w.Append(btn_a_text);	
	w.Append(btn_b);
	w.Append(btn_b_text);
	w.Append(btn_bk);
	w.Append(btn_bk_text);	
	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_bottom_bg);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (current_menu == SETTINGS_NETWORK) {
		Update();
		ret = optionBrowser.GetClickedOption();
		if (ret >= 0) {
			if (ret == (options.length-1)) {
				shareId = -1;				
			} else {
				shareId = ret;
			}
			current_menu = SETTINGS_NETWORK_SMB;
		}
		if (bBtn.GetState() == STATE_CLICKED) {
			current_menu = SETTINGS;
		} else if (backBtn.GetState() == STATE_CLICKED) {
			current_menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_bottom_bg);
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

	GuiText titleTxt("Home > Settings", 24, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(150, 35);

	GuiButton bBtn(20, 20);
	bBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	bBtn.SetPosition(10, -35);
	bBtn.SetTrigger(trigB);

	GuiButton backBtn(20, 20);
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetTrigger(trigBk);	

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);
	GuiWindow w(screenwidth, screenheight);
	btn_bk->SetPosition(400, 641);
	btn_bk_text->SetPosition(435, 643);							
	w.Append(&bBtn);
	w.Append(&backBtn);
	w.Append(btn_a);
	w.Append(btn_a_text);	
	w.Append(btn_b);
	w.Append(btn_b_text);
	w.Append(btn_bk);
	w.Append(btn_bk_text);															
	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_bottom_bg);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);	

	while (current_menu == SETTINGS) {
		Update();

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
		case 4:
			WindowPrompt("Warning", "Not implemented yet", "Ok", NULL);
			//current_menu = SETTINGS_NETWORK;
			break;  				
		}

		if ((bBtn.GetState() == STATE_CLICKED) || (backBtn.GetState() == STATE_CLICKED)) {
			current_menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_bottom_bg);	
}

static void InitMplayerSettings(void)
{
	sprintf(_ass_color, "%08x", XMPlayerCfg.subcolor);
	sprintf(_ass_border_color, "%08x", XMPlayerCfg.border_color);
	ass_color = _ass_color;
	ass_border_color = _ass_border_color;
	sub_cp = XMPlayerCfg.subcp;
	dvdsub_lang = XMPlayerCfg.sublang;
	audio_lang = XMPlayerCfg.alang;
}

void MenuMplayer()
{
	printf("filename:%s\r\n", mplayer_filename);
	static int mplayer_need_init = 1;
	if (mplayer_need_init) {
		char * argv[] = {
			"mplayer.xenon",
			"-lavdopts", "skiploopfilter=all:threads=5",
			mplayer_filename,
		};
		mplayer_need_init = 0;
		int argc = sizeof (argv) / sizeof (char *);

		mplayer_main(argc, argv);
		// will never be here !!!
	} else {
		mplayer_load(mplayer_filename);
		mplayer_return_to_player();
	}
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

static void GuiLoop()
{
	while (need_gui) {
		if (current_menu == HOME_PAGE) {
			HomePage();
		} else if (current_menu == BROWSE_VIDEO) {
			Browser("Home > Videos", root_dev);
		} else if (current_menu == BROWSE_AUDIO) {
			Browser("Home > Music", root_dev);
		} else if (current_menu == BROWSE_PICTURE) {
			Browser("Home > Photos", root_dev);
		} else if (current_menu == BROWSE_ALL) {
			Browser("Home > All", root_dev);
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
		} else if (current_menu == SETTINGS_NETWORK) {
			NetworkSettings();
		} else if (current_menu == SETTINGS_NETWORK_SMB) {
			NetworkSettingsSMB();
		}
	}
}

static void FindDevices()
{
	device_list_size = 0;
	for (int i = 3; i < STD_MAX; i++) {
		if (devoptab_list[i]->structSize) {
			sprintf(device_list[device_list_size], "%s:/", devoptab_list[i]->name);
			printf("findDevices : %s\r\n", device_list[device_list_size]);
			device_list_size++;
		}
	}

	root_dev = device_list[0];
}

static void LoadingThread()
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
	delay(2);
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
	xenon_run_thread_task(2, thread_stack[2], (void*) LoadingThread);

	// Init devices
	usb_init();
	xenon_ata_init();
	xenon_atapi_init();
	usb_do_poll();
			
	// fs
	mount_all_devices();
	FindDevices();

	InitFreeType((u8*) font_ttf, font_ttf_size);
	SetupPads();
	ChangeFontSize(26);
	CommonSetup();

	// signal end of loading thread
	lock(&loadingThreadLock);
	end_loading_thread = 1;
	unlock(&loadingThreadLock);
	do  {
		lock(&loadingThreadLock);
		// try to mount undetected devices
	//	usb_do_poll();
	//	mount_all_devices();
		udelay(25);
		unlock(&loadingThreadLock);
	} while(loading_thread_finished == 0);
	
	// recheck devices
	//FindDevices();

	// init mplayer
	init_mplayer();
	
	// preference
	if (LoadPrefs() == false)
		SavePrefs(true);

	// load mplayer settings
	InitMplayerSettings();

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
		GuiLoop();
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
	ResetController(); //resets buttons, so pushes from mplayer doesn't get reconized in browser
	current_menu = last_menu;

	GuiLoop();

}

/**
 * return to mplayer
 */
extern "C" void mplayer_return_to_player()
{
	need_gui = 0;
}
