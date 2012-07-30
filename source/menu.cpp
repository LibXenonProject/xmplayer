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
#include <xenon_smc/xenon_smc.h> /*siz - included to add restart and shutdown buttons: 15/07/2012 */
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

#include "../build/logo_png.h"
#include "../build/loading_0_png.h"
#include "../build/loading_1_png.h"
#include "../build/loading_2_png.h"
#include "../build/loading_3_png.h"

#include "../build/home_left_png.h"
#include "../build/home_main_function_frame_bg_png.h"
#include "../build/welcome_background_bg_png.h"
#include "../build/home_music_sm_icon_n_png.h"
#include "../build/home_photo_sm_icon_n_png.h"
#include "../build/home_video_sm_icon_n_png.h"
#include "../build/home_settings_sm_icon_n_png.h"
#include "../build/home_hdd_sub_icon_n_png.h"
#include "../build/home_nohdd_sub_icon_n_png.h"
#include "../build/home_horizontal_select_bar_png.h"
#include "../build/font_ttf.h"

#include "../build/browser_list_btn_png.h"
#include "../build/browser_folder_icon_f_png.h"
#include "../build/options_menu_btn_png.h"
#include "../build/browser_list_arrow_down_png.h"
#include "../build/browser_list_arrow_up_png.h"

// osd
#include "../build/video_control_frame_bg_png.h"
#include "../build/video_control_time_played_line_bg_png.h"
#include "../build/video_player_time_played_line_n_01_png.h"
#include "../build/video_control_fastforward_btn_png.h"
#include "../build/video_control_frame_bg_png.h"
#include "../build/video_control_next_btn_png.h"
#include "../build/video_control_pause_btn_png.h"
#include "../build/video_control_play_btn_png.h"
#include "../build/video_control_previous_btn_png.h"
#include "../build/video_control_rewind_btn_png.h"
#include "../build/video_control_stop_btn_png.h"
#include "../build/video_control_time_played_line_bg_png.h"

#include "../build/browser_photo_icon_f_png.h"
#include "../build/browser_video_icon_f_png.h"
#include "../build/browser_music_icon_f_png.h"
#include "../build/browser_elf_icon_f_png.h"
#include "../build/browser_file_icon_f_png.h"
#include "../build/browser_top_png.h"

#include "../build/folder_music_icon_png.h"
#include "../build/folder_photo_icon_png.h"
#include "../build/browser_video_icon_f_png.h"

// osd 3
#include "../build/options_menu_audio_channel_icon_f_png.h"
#include "../build/options_menu_audio_channel_icon_n_png.h"
#include "../build/options_menu_bg_x5_png.h"
#include "../build/options_menu_bg_x6_png.h"
#include "../build/options_menu_bg_x7_png.h"
#include "../build/options_menu_info_icon_f_png.h"
#include "../build/options_menu_info_icon_n_png.h"
#include "../build/options_menu_pan_icon_d_png.h"
#include "../build/options_menu_pan_icon_f_png.h"
#include "../build/options_menu_pan_icon_n_png.h"
#include "../build/options_menu_repeat_icon_d_png.h"
#include "../build/options_menu_repeat_icon_f_png.h"
#include "../build/options_menu_repeat_icon_n_png.h"
#include "../build/options_menu_subtitle_icon_d_png.h"
#include "../build/options_menu_subtitle_icon_f_png.h"
#include "../build/options_menu_subtitle_icon_n_png.h"
#include "../build/options_menu_zoomin_icon_f_png.h"
#include "../build/options_menu_zoomin_icon_n_png.h"
#include "../build/options_menu_zoomout_icon_f_png.h"
#include "../build/options_menu_zoomout_icon_n_png.h"
#include "../build/options_menu_gridview_icon_n_png.h"
#include "../build/options_menu_gridview_icon_f_png.h"

#include "../build/button_blue_png.h"
#include "../build/button_red_png.h"
#include "../build/button_green_png.h"

#include "../build/options_menu_bg_x5_png.h"
#include "../build/options_menu_bg_x6_png.h"
#include "../build/options_menu_bg_x7_png.h"

#include "../build/button_p_seek_png.h" //siz
#include "../build/button_p_seek_select_png.h" //siz
#include "../build/p_seek_bg_png.h" //siz

#include "../build/video_info_bg_png.h"

#include "../build/fr_lang.h"

#include "filebrowser.h"

#include "mplayer_func.h"
#include "folder_video_icon_png.h"

#define _(x)	gettext(x)

char * root_dev = NULL;
static int device_list_size = 0;
static char device_list[STD_MAX][10];

enum {
	MENU_BACK = -1,
	HOME_PAGE = 1,
	MENU_MPLAYER,
	MENU_ELF,
	BROWSE = 0x10,
	BROWSE_VIDEO,
	BROWSE_AUDIO,
	BROWSE_PICTURE,
	BROWSE_ALL,
	SETTINGS,
	OSD = 0x20,
};
/**
 * used at loading
 **/
static XenosSurface * logo = NULL;
static XenosSurface * loading[4] = {NULL};

static GuiImage * bgImg = NULL;
static GuiWindow * mainWindow = NULL;
static GuiTrigger * trigA;

/**
 * video osd
 **/
static GuiImage * video_osd_progress_bar_front = NULL;
static GuiImage * video_osd_progress_bar_back = NULL;
static GuiImage * video_osd_bg = NULL;

static GuiText * video_osd_info_filename = NULL;
static GuiText * video_osd_info_cur_time = NULL;
static GuiText * video_osd_info_duration = NULL;

static GuiWindow * video_osd_infobar = NULL;
static GuiImage * video_osd_infobar_bg = NULL;
static GuiText * video_osd_infobar_text_filename = NULL;
static GuiText * video_osd_infobar_text_trackinfo = NULL;
static GuiText * video_osd_infobar_text_resolution = NULL;
static GuiText * video_osd_infobar_text_bitrate = NULL;

static GuiImage * video_osd_play = NULL;
static GuiImage * video_osd_pause = NULL;
static GuiImage * video_osd_stop = NULL;
static GuiImage * video_osd_next = NULL;
static GuiImage * video_osd_prev = NULL;
static GuiImage * video_osd_rewind = NULL;
static GuiImage * video_osd_forward = NULL;

/**
 * Browser
 **/
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

/**
 * Home screen
 **/
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
static GuiButton * home_restart_btn = NULL; /*siz - added Restart: 15/07/2012 */
static GuiButton * home_shutdown_btn = NULL; /*siz - added Shutdown: 15/07/2012 */

static GuiImage * home_video_img = NULL;
static GuiImage * home_music_img = NULL;
static GuiImage * home_photo_img = NULL;
static GuiImage * home_setting_img = NULL;

static GuiText* home_video_txt = NULL;
static GuiText * home_all_txt = NULL;
static GuiText * home_music_txt = NULL;
static GuiText * home_photo_txt = NULL;
static GuiText * home_setting_txt = NULL;
static GuiText * home_restart_txt = NULL; /*siz - added Restart: 15/07/2012 */
static GuiText * home_shutdown_txt = NULL; /*siz - added Shutdown: 15/07/2012 */

/**
 * Not used yet
 **/
static GuiImage * decoration_state = NULL;
static GuiImage * decoration_keyicon = NULL;
static GuiImage * decoration_keyicon_ex = NULL;
static GuiImage * decoration_wrongkeyicon = NULL;

/**
 * Osd option menu
 **/
static GuiTab * osd_options_window = NULL;
static GuiImage * osd_options_background = NULL;

static GuiImage * osd_options_bg_x5 = NULL;
static GuiImage * osd_options_bg_x6 = NULL;
static GuiImage * osd_options_bg_x7 = NULL;

static GuiImage * osd_options_menu_audio_channel_icon_f = NULL;
static GuiImage * osd_options_menu_audio_channel_icon_n = NULL;

static GuiImage * options_menu_icon_over[8] = {NULL};

static GuiImage * options_menu_info_icon_f = NULL;
static GuiImage * options_menu_info_icon_n = NULL;

static GuiImage * options_menu_pan_icon_f = NULL;
static GuiImage * options_menu_pan_icon_n = NULL;

static GuiImage * options_menu_repeat_icon_d = NULL;
static GuiImage * options_menu_repeat_icon_f = NULL;
static GuiImage * options_menu_repeat_icon_n = NULL;

static GuiImage * options_menu_subtitle_icon_d = NULL;
static GuiImage * options_menu_subtitle_icon_f = NULL;
static GuiImage * options_menu_subtitle_icon_n = NULL;

static GuiImage * options_menu_zoomin_icon_f = NULL;
static GuiImage * options_menu_zoomin_icon_n = NULL;

static GuiImage * options_menu_zoomout_icon_f = NULL;
static GuiImage * options_menu_zoomout_icon_n = NULL;

static GuiImage * options_menu_return_to_gui_f = NULL;
static GuiImage * options_menu_return_to_gui_n = NULL;

static GuiButton * osd_options_menu_audio_channel_btn = NULL;
static GuiButton * osd_options_menu_info_btn = NULL;
static GuiButton * osd_options_menu_pan_btn = NULL;
static GuiButton * osd_options_menu_repeat_btn = NULL;
static GuiButton * osd_options_menu_next_btn = NULL;
static GuiButton * osd_options_menu_subtitle_btn = NULL;
static GuiButton * osd_options_menu_zoomin_btn = NULL;
static GuiButton * osd_options_menu_zoomout_btn = NULL;

/**
 * Osd option subtitles
 */
static GuiImageData * osd_options_bg_entry = NULL;
static GuiWindow * osd_options_subtitle_window = NULL;
static GuiOptionBrowser * osd_options_subtitle = NULL;
static OptionList subtitle_option_list;

static OptionList audio_option_list;

static char mplayer_filename[2048];
static char exited_dir[2048]; /*siz - added: save exit path, for SmartMenu: 20/07/2012 */
static char exited_dir_array[64][2048]; /*siz - added: save exit path for a specific menu, for SmartMenu: 20/07/2012 */
static int exited_item[64]; /*siz - added: save exit item, for SmartMenu: 20/07/2012 */
static char seek_filename[2048]; /*siz - added: path for playback-resume cache file: 29/07/2012 */
static char * playerSeekTime = ""; /*siz - added: time for playback-resume: 29/07/2012 */
static char * playerStopFile = ""; /*siz - added: file for playback-resume: 29/07/2012 */
static int playerSeekChoice = 0; /*siz - added: choice for playback-resume: 29/07/2012 */

static int last_menu;

static int current_menu = HOME_PAGE;

static void update() {
	UpdatePads();
	mainWindow->Draw();
	Menu_Render();
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
}

/**
 * Callback for osd option bar
 **/
static int osd_display_info = 0;
static int osd_display_option_audio = 0;
static int osd_display_option_subtitle = 0;

static void osd_option_default_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_pan_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		playerSwitchFullscreen();
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_vsync_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		playerSwitchVsync();
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_next_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		button->ResetState();
		playerTurnOffSubtitle(); /*siz added: turns sub_visibility off before quit -> sub doesn't stay on screen on a new video - 30/07/2012 */
		playerGuiAsked(playerStopFile);/*siz added: for playback-resume, it gives file to exit func. which saves last postion to file.txt - 29/07/2012 */
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_loop_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_info_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		osd_display_info = !osd_display_info;
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_audio_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		playerSwitchAudio();
		//osd_display_option_audio = 1; 
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_sub_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		playerSwitchSubtitle();
		//		osd_display_option_subtitle = 1;
		//		osd_options_subtitle_window->SetVisible(true);
		//		osd_options_subtitle_window->SetFocus(1);
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

/**
 * Load ressources
 **/
static void loadHomeRessources() {

	//	<image image="image/home_left.png" x="0" y="0" w="522" h="720" bg="1"/>
	//	<image image="image/home_main_function_frame_bg.png" x="0" y="341" w="1280" h="148" bg="1"/>
	home_left = new GuiImage(new GuiImageData(home_left_png));
	home_main_function_frame_bg = new GuiImage(new GuiImageData(home_main_function_frame_bg_png));

	home_main_function_frame_bg->SetPosition(0, 341);

	// <wgt_mlist name="VERT_MLIST" image="" x="305" y="140" w="200" h="440" fontsize="26" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="vert" itemcount="4" extra_index="3" extra_len="0"/>
	home_list_v = new GuiList(200, 440);

	home_list_v->SetPosition(290, 140);
	home_list_v->SetCount(4);
	home_list_v->SetCenter(3);


	home_list_v->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	// <wgt_mlist name="HORI_MLIST" x="560" y="360" w="465" h="110" fontsize="20" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="hori" itemcount="3"/>
	home_list_h = new GuiList(465, 110, 'H');

	home_list_h->SetPosition(560, 360);
	home_list_h->SetCount(3);
	home_list_h->SetCenter(2);

	home_list_h_selector = new GuiImage(new GuiImageData(home_horizontal_select_bar_png));
	home_list_h_selector->SetPosition(715, 350);

	// <text text="@@curitem" x="550" y="610" w="650" h="40" align="right" fontsize="32" textcolor="0xffffff"/>
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
	home_music_txt = new GuiText("Music", 48, 0xFFFFFFFF);  // siz edit: Musics to Music
	home_photo_txt = new GuiText("Photos", 48, 0xFFFFFFFF);
	home_setting_txt = new GuiText("Settings", 48, 0xFFFFFFFF);
	home_restart_txt = new GuiText("Restart", 48, 0xFFFFFFFF); /*siz - added Restart: 15/07/2012 */
	home_shutdown_txt = new GuiText("Shutdown", 48, 0xFFFFFFFF); /*siz - added Shutdown: 15/07/2012 */

	home_video_btn = new GuiButton(home_video_img->GetWidth(), home_video_img->GetHeight());
	home_all_btn = new GuiButton(home_video_img->GetWidth(), home_video_img->GetHeight());
	home_music_btn = new GuiButton(home_music_img->GetWidth(), home_music_img->GetHeight());
	home_photo_btn = new GuiButton(home_photo_img->GetWidth(), home_photo_img->GetHeight());
	home_setting_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());
	home_restart_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight()); /*siz - added Restart: 15/07/2012 */
	home_shutdown_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight()); /*siz - added Shutdown: 15/07/2012 */

	//	home_video_btn->SetIcon(home_video_img);
	//	home_music_btn->SetIcon(home_music_img);
	//	home_photo_btn->SetIcon(home_photo_img);
	//	home_setting_btn->SetIcon(home_setting_img);

	home_video_btn->SetLabel(home_video_txt);
	home_all_btn->SetLabel(home_all_txt);
	home_music_btn->SetLabel(home_music_txt);
	home_photo_btn->SetLabel(home_photo_txt);
	home_setting_btn->SetLabel(home_setting_txt);
	home_restart_btn->SetLabel(home_restart_txt); /*siz - added Restart: 15/07/2012 */
	home_shutdown_btn->SetLabel(home_shutdown_txt); /*siz - added Shutdown: 15/07/2012 */
}

static void loadBrowserRessources() {

	// Browser
	browser_photo_icon = new GuiImageData(browser_photo_icon_f_png);
	browser_video_icon = new GuiImageData(browser_video_icon_f_png);
	browser_music_icon = new GuiImageData(browser_music_icon_f_png);

	browser_photo_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_video_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_music_folder_icon = new GuiImageData(browser_folder_icon_f_png);

	browser_selector = new GuiImageData(browser_list_btn_png);

	//	<item id="headline" x="100" y="44" w="300" h="40" align="left"/>
	//	<item id="subheadline" x="100" y="70" w="300" h="30" fontsize="24" align="left"/>
	//	<item id="pagecounter" x="980" y="54" w="200" h="26" fontsize="24" align="right" textcolor="0xffffff"/>

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

	browser_up_icon = new GuiImage(new GuiImageData(browser_list_arrow_up_png));
	browser_down_icon = new GuiImage(new GuiImageData(browser_list_arrow_down_png));
	;

	// <image image="@@moreprev" x="150" y="650" w="19" h="13"/>
	// <image image="@@morenext" x="1111" y="650" w="19" h="13"/>
	browser_up_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_down_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_up_icon->SetPosition(120, 650);
	browser_down_icon->SetPosition(1141, 650);

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

	//	browser_file_icon = browser_video_icon;

	gui_browser = new GuiFileBrowser(980, 500, browser_selector, browser_folder_icon, browser_files_icon);
	gui_browser->SetPosition(150, 131);
	gui_browser->SetFontSize(20);
	gui_browser->SetSelectedFontSize(26);
	gui_browser->SetPageSize(10);
}

static void loadOsdRessources() { // OSD
	video_osd_progress_bar_front = new GuiImage(new GuiImageData(video_player_time_played_line_n_01_png));
	video_osd_progress_bar_back = new GuiImage(new GuiImageData(video_control_time_played_line_bg_png));
	video_osd_bg = new GuiImage(new GuiImageData(video_control_frame_bg_png));

	//<text text="@@info_filename" x="264" y="621" w="644" h="24" fontsize="22" textcolor="0xffffff" speed="1" delay="2" align="left" auto_translate="1"/>
	//<text text="@@info_cur_time" x="943" y="625" w="80" h="20" fontsize="18" textcolor="0xffffff" align="left"/>
	//<text text="@@info_duration" x="1023" y="625" w="95" h="20" fontsize="18" textcolor="0x0096fa" align="left"/>

	//<image x="202" y="613" w="40" h="40" image="@@speed_state"/>
	//<image image="@@play_state" x="202" y="613" w="40" h="40"/>

	video_osd_info_filename = new GuiText("@@info_filename", 22, 0xffffffff);
	video_osd_info_cur_time = new GuiText("@@info_cur_time", 18, 0xffffffff);
	video_osd_info_duration = new GuiText("@@info_duration", 18, 0xfffa9600);

	/** video infobar **/
	video_osd_infobar_text_filename = new GuiText("Filename", 22, 0xffffffff);
	video_osd_infobar_text_trackinfo = new GuiText("Codec", 22, 0xffffffff);
	video_osd_infobar_text_resolution = new GuiText("Resolution", 22, 0xffffffff);
	video_osd_infobar_text_bitrate = new GuiText("Bitrate", 22, 0xffffffff);

	video_osd_infobar_bg = new GuiImage(new GuiImageData(video_info_bg_png));

	video_osd_infobar_bg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	//video_osd_infobar_bg->SetPosition(68,240);


	video_osd_infobar = new GuiWindow(1144, 240);
	//video_osd_infobar->SetPosition(110, 350);
	video_osd_infobar->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	video_osd_infobar->Append(video_osd_infobar_bg);

	video_osd_infobar->Append(video_osd_infobar_text_filename);
	video_osd_infobar->Append(video_osd_infobar_text_trackinfo);
	video_osd_infobar->Append(video_osd_infobar_text_resolution);
	video_osd_infobar->Append(video_osd_infobar_text_bitrate);

	video_osd_infobar_text_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_trackinfo->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_resolution->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_bitrate->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	video_osd_infobar_text_filename->SetPosition(20, 20);
	video_osd_infobar_text_trackinfo->SetPosition(20, 50);
	video_osd_infobar_text_resolution->SetPosition(20, 80);
	video_osd_infobar_text_bitrate->SetPosition(20, 110);

	/** play state **/
	video_osd_play = new GuiImage(new GuiImageData(video_control_play_btn_png));
	video_osd_pause = new GuiImage(new GuiImageData(video_control_pause_btn_png));
	video_osd_stop = new GuiImage(new GuiImageData(video_control_stop_btn_png));
	video_osd_next = new GuiImage(new GuiImageData(video_control_next_btn_png));
	video_osd_prev = new GuiImage(new GuiImageData(video_control_previous_btn_png));
	video_osd_rewind = new GuiImage(new GuiImageData(video_control_rewind_btn_png));
	video_osd_forward = new GuiImage(new GuiImageData(video_control_fastforward_btn_png));


	/** a ranger **/
	// <image image="image/video_control_frame_bg.png" x="149" y="597" w="983" h="73" disable="@@progress_disable" bg="1"/>
	video_osd_bg->SetPosition(149, 597);

	// <progress_bar name="video_time_bar" x="262" y="652" w="858" h="4" pb_back_img="image/video_control_time_played_line_bg.png" pb_front_img="image/Video_Player_time_played_line_n_01.png" pb_point_img="image/slideshow_player_time_played_line_mark.png" pb_delay="500" disable="@@progress_disable"/>
	video_osd_progress_bar_front->SetPosition(262, 652);
	//video_osd_progress_bar_back->SetPosition(262,652);
	video_osd_progress_bar_back->SetPosition(262, 652);

	//<text text="@@info_filename" x="264" y="621" w="644" h="24" fontsize="22" textcolor="0xffffff" speed="1" delay="2" align="left" auto_translate="1"/>
	//<text text="@@info_cur_time" x="943" y="625" w="80" h="20" fontsize="18" textcolor="0xffffff" align="left"/>
	//<text text="@@info_duration" x="1023" y="625" w="95" h="20" fontsize="18" textcolor="0x0096fa" align="left"/>

	video_osd_info_filename->SetPosition(264, 621);
	video_osd_info_cur_time->SetPosition(943, 625);
	video_osd_info_duration->SetPosition(1023, 625);

	video_osd_info_filename->SetMaxWidth(644);
	video_osd_info_filename->SetScroll(SCROLL_HORIZONTAL);
	//
	video_osd_info_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_info_cur_time->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_info_duration->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	//<image x="202" y="613" w="40" h="40" image="@@speed_state"/>
	//<image image="@@play_state" x="202" y="613" w="40" h="40"/>

	video_osd_play->SetPosition(202, 613);
	video_osd_pause->SetPosition(202, 613);
	video_osd_stop->SetPosition(202, 613);
	video_osd_next->SetPosition(202, 613);
	video_osd_prev->SetPosition(202, 613);
	video_osd_rewind->SetPosition(202, 613);
	video_osd_forward->SetPosition(202, 613);


	/** osd level 3**/
	// <table x="435" y="30" h="50" w="410" cols="7" align="hcenter" disable="@@disable-options_bg">
	//osd_options_window = new GuiTab(520, 91);
	osd_options_window = new GuiTab(340, 50);
	osd_options_window->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	osd_options_window->SetPosition(480, 30);

	osd_options_window->setRow(1);
	osd_options_window->setCol(5);


	// <image image="image/slideshow_options_menu_bg_x7.png" x="410" y="0" w="460" h="91" bg="1" disable="@@disable-options_bg"/>
	osd_options_bg_x5 = new GuiImage(new GuiImageData(options_menu_bg_x5_png));
	osd_options_bg_x6 = new GuiImage(new GuiImageData(options_menu_bg_x6_png));
	osd_options_bg_x7 = new GuiImage(new GuiImageData(options_menu_bg_x7_png));

	osd_options_bg_x7->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	osd_options_bg_x7->SetPosition(410, 0);

	osd_options_bg_x5->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	osd_options_bg_x5->SetPosition(470, 0);

	osd_options_menu_audio_channel_icon_f = new GuiImage(new GuiImageData(options_menu_audio_channel_icon_f_png));
	osd_options_menu_audio_channel_icon_n = new GuiImage(new GuiImageData(options_menu_audio_channel_icon_n_png));

	options_menu_info_icon_f = new GuiImage(new GuiImageData(options_menu_info_icon_f_png));
	options_menu_info_icon_n = new GuiImage(new GuiImageData(options_menu_info_icon_n_png));

	options_menu_pan_icon_f = new GuiImage(new GuiImageData(options_menu_pan_icon_f_png));
	options_menu_pan_icon_n = new GuiImage(new GuiImageData(options_menu_pan_icon_n_png));

	options_menu_repeat_icon_d = new GuiImage(new GuiImageData(options_menu_repeat_icon_d_png));
	options_menu_repeat_icon_f = new GuiImage(new GuiImageData(options_menu_repeat_icon_f_png));
	options_menu_repeat_icon_n = new GuiImage(new GuiImageData(options_menu_repeat_icon_n_png));

	options_menu_subtitle_icon_d = new GuiImage(new GuiImageData(options_menu_subtitle_icon_d_png));
	options_menu_subtitle_icon_f = new GuiImage(new GuiImageData(options_menu_subtitle_icon_f_png));
	options_menu_subtitle_icon_n = new GuiImage(new GuiImageData(options_menu_subtitle_icon_n_png));

	options_menu_zoomin_icon_f = new GuiImage(new GuiImageData(options_menu_zoomin_icon_f_png));
	options_menu_zoomin_icon_n = new GuiImage(new GuiImageData(options_menu_zoomin_icon_n_png));

	options_menu_zoomout_icon_f = new GuiImage(new GuiImageData(options_menu_zoomout_icon_f_png));
	options_menu_zoomout_icon_n = new GuiImage(new GuiImageData(options_menu_zoomout_icon_n_png));

	options_menu_return_to_gui_f = new GuiImage(new GuiImageData(options_menu_gridview_icon_f_png));
	options_menu_return_to_gui_n = new GuiImage(new GuiImageData(options_menu_gridview_icon_n_png));

	for (int i = 0; i < 8; i++) {
		options_menu_icon_over[i] = new GuiImage(new GuiImageData(options_menu_btn_png));
		options_menu_icon_over[i]->SetPosition(0, -30);
	}

	osd_options_menu_audio_channel_btn = new GuiButton(osd_options_menu_audio_channel_icon_f->GetWidth(), osd_options_menu_audio_channel_icon_f->GetHeight());
	osd_options_menu_info_btn = new GuiButton(options_menu_info_icon_f->GetWidth(), options_menu_info_icon_f->GetHeight());
	osd_options_menu_pan_btn = new GuiButton(options_menu_pan_icon_f->GetWidth(), options_menu_pan_icon_f->GetHeight());
	osd_options_menu_repeat_btn = new GuiButton(options_menu_repeat_icon_d->GetWidth(), options_menu_repeat_icon_d->GetHeight());
	osd_options_menu_subtitle_btn = new GuiButton(options_menu_subtitle_icon_f->GetWidth(), options_menu_subtitle_icon_f->GetHeight());
	osd_options_menu_zoomin_btn = new GuiButton(options_menu_zoomin_icon_f->GetWidth(), options_menu_zoomin_icon_f->GetHeight());
	osd_options_menu_zoomout_btn = new GuiButton(options_menu_zoomout_icon_f->GetWidth(), options_menu_zoomout_icon_f->GetHeight());
	osd_options_menu_next_btn = new GuiButton(options_menu_return_to_gui_f->GetWidth(), options_menu_return_to_gui_f->GetHeight());

	// image
	osd_options_menu_audio_channel_btn->SetImage(osd_options_menu_audio_channel_icon_n);
	osd_options_menu_audio_channel_btn->SetImageOver(osd_options_menu_audio_channel_icon_f);

	osd_options_menu_info_btn->SetImage(options_menu_info_icon_n);
	osd_options_menu_info_btn->SetImageOver(options_menu_info_icon_f);

	osd_options_menu_pan_btn->SetImage(options_menu_pan_icon_n);
	osd_options_menu_pan_btn->SetImageOver(options_menu_pan_icon_f);

	osd_options_menu_repeat_btn->SetImage(options_menu_repeat_icon_n);
	osd_options_menu_repeat_btn->SetImageOver(options_menu_repeat_icon_f);

	osd_options_menu_subtitle_btn->SetImage(options_menu_subtitle_icon_n);
	osd_options_menu_subtitle_btn->SetImageOver(options_menu_subtitle_icon_f);

	osd_options_menu_zoomin_btn->SetImage(options_menu_zoomin_icon_n);
	osd_options_menu_zoomin_btn->SetImageOver(options_menu_zoomin_icon_f);

	osd_options_menu_zoomout_btn->SetImage(options_menu_zoomout_icon_n);
	osd_options_menu_zoomout_btn->SetImageOver(options_menu_zoomout_icon_f);

	osd_options_menu_next_btn->SetImage(options_menu_return_to_gui_n);
	osd_options_menu_next_btn->SetImageOver(options_menu_return_to_gui_f);

	osd_options_menu_audio_channel_btn->SetIconOver(options_menu_icon_over[0]);
	osd_options_menu_info_btn->SetIconOver(options_menu_icon_over[1]);
	osd_options_menu_pan_btn->SetIconOver(options_menu_icon_over[2]);
	osd_options_menu_subtitle_btn->SetIconOver(options_menu_icon_over[3]);
	osd_options_menu_zoomin_btn->SetIconOver(options_menu_icon_over[4]);
	osd_options_menu_zoomout_btn->SetIconOver(options_menu_icon_over[5]);
	osd_options_menu_next_btn->SetIconOver(options_menu_icon_over[6]);
	osd_options_menu_repeat_btn->SetIconOver(options_menu_icon_over[7]);

	// callback*
	osd_options_menu_audio_channel_btn->SetUpdateCallback(osd_options_audio_callback);
	osd_options_menu_info_btn->SetUpdateCallback(osd_options_info_callback);
	osd_options_menu_pan_btn->SetUpdateCallback(osd_options_pan_callback);
	osd_options_menu_next_btn->SetUpdateCallback(osd_options_next_callback);
	osd_options_menu_repeat_btn->SetUpdateCallback(osd_options_loop_callback);
	osd_options_menu_subtitle_btn->SetUpdateCallback(osd_options_sub_callback);
	osd_options_menu_zoomin_btn->SetUpdateCallback(osd_option_default_callback);
	osd_options_menu_zoomout_btn->SetUpdateCallback(osd_option_default_callback);

	GuiText * btn_audio_text = new GuiText("Audio", 18, 0xffffffff);
	GuiText * btn_info_text = new GuiText("Information", 18, 0xffffffff);
	GuiText * btn_fullscreen_text = new GuiText("Fullscreen", 18, 0xffffffff);
	GuiText * btn_subtitle_text = new GuiText("Subtitle", 18, 0xffffffff);
	GuiText * btn_return_text = new GuiText("Return to gui", 18, 0xffffffff);

	btn_audio_text->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	btn_info_text->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	btn_fullscreen_text->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	btn_subtitle_text->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	btn_return_text->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	btn_audio_text->SetPosition(0, -70);
	btn_info_text->SetPosition(0, -70);
	btn_fullscreen_text->SetPosition(0, -70);
	btn_subtitle_text->SetPosition(0, -70);
	btn_return_text->SetPosition(0, -70);

	//	osd_options_menu_audio_channel_btn->SetLabelOver(btn_audio_text);
	//	osd_options_menu_info_btn->SetLabelOver(btn_info_text);
	//	osd_options_menu_pan_btn->SetLabelOver(btn_fullscreen_text);
	//	osd_options_menu_subtitle_btn->SetLabelOver(btn_subtitle_text);
	//	osd_options_menu_next_btn->SetLabelOver(btn_return_text);

	//osd_options_window->SetBackground(osd_options_bg_x7);
	osd_options_window->SetBackground(osd_options_bg_x5);
	osd_options_window->Append(osd_options_menu_audio_channel_btn);
	osd_options_window->Append(osd_options_menu_info_btn);
	osd_options_window->Append(osd_options_menu_pan_btn);
	osd_options_window->Append(osd_options_menu_subtitle_btn);
	osd_options_window->Append(osd_options_menu_next_btn);
	//	osd_options_window->Append(osd_options_menu_repeat_btn);
	//	osd_options_window->Append(osd_options_menu_zoomin_btn);
	//	osd_options_window->Append(osd_options_menu_zoomout_btn);

	osd_options_menu_audio_channel_btn->SetFocus(1);

	// position
	for (u32 i = 1; i < osd_options_window->GetSize(); i++) {
		osd_options_window->GetGuiElementAt(i)->SetTrigger(trigA);
	}


	/**
	 * Osd Subtiles window
	 */
	int i = 0;
	sprintf(subtitle_option_list.name[i++], "Subtitles");
	sprintf(subtitle_option_list.name[i++], "Visibility");
	sprintf(subtitle_option_list.name[i++], "Position");
	sprintf(subtitle_option_list.name[i++], "Delay");
	sprintf(subtitle_option_list.name[i++], "Scale");
	subtitle_option_list.length = i;

	for (i = 0; i < subtitle_option_list.length; i++)
		subtitle_option_list.value[i][0] = 0;

	osd_options_subtitle_window = new GuiWindow(1142, 240);
	osd_options_subtitle = new GuiOptionBrowser(1122, 220, new GuiImageData(browser_list_btn_png), &subtitle_option_list);
	osd_options_subtitle->SetPosition(20, 20);

	osd_options_subtitle_window->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);


	osd_options_subtitle_window->Append(new GuiImage(new GuiImageData(video_info_bg_png)));
	osd_options_subtitle_window->Append(osd_options_subtitle);
}


extern "C" int sub_pos;
extern "C" int sub_visibility;
extern "C" float sub_delay;

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label) {
	int choice = -1;

	GuiWindow promptWindow(448, 288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	//	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	//	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt(title, 26, (XeColor) {
		255, 255, 255, 255
	});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0, 14);

	GuiText msgTxt(msg, 26, (XeColor) {
		255, 255, 255, 255
	});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0, -20);
	msgTxt.SetWrap(true, 430);

	GuiText btn1Txt(btn1Label, 22, (XeColor) {
		255, 255, 255, 255
	});
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
	//	btn1.SetSoundOver(&btnSoundOver);
	//	btn1.SetSoundClick(&btnSoundClick);
	btn1.SetTrigger(trigA);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 22, (XeColor) {
		255, 0, 0, 0
	});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn2.SetPosition(-20, -25);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	//	btn2.SetSoundOver(&btnSoundOver);
	//	btn2.SetSoundClick(&btnSoundClick);
	btn2.SetTrigger(trigA);
	btn2.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&btn1);

	if (btn2Label)
		promptWindow.Append(&btn2);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);

	if (btn2Label) {
		btn1.ResetState();
		btn2.SetState(STATE_SELECTED);
	}

	while (choice == -1) {
		update();
		if (btn1.GetState() == STATE_CLICKED)
			choice = 1;
		else if (btn2.GetState() == STATE_CLICKED)
			choice = 0;
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while (promptWindow.GetEffect() > 0) {
		update();
	}
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	return choice;
}
static char* playerSeekFormatTime(char * dest, double time) {
	div_t hrmin, minsec;
	minsec = div(time, 60);
	hrmin = div(minsec.quot, 60);
	if (time < 3600) { 
	asprintf(&dest, "Resume from %02d:%02d", hrmin.rem, minsec.rem);
	} else {
	asprintf(&dest, "Resume from %d:%02d:%02d", hrmin.quot, hrmin.rem, minsec.rem);
	}
	char * destfile = dest;
	return destfile;
	free(dest);	
}
/* siz added: File exists function - 25/07/2012 */
bool file_exists(const char * filename) {
   FILE * fd = fopen(filename, "rb");
   if (fd != NULL) {
      fclose(fd);
      return true;
   }
   return false;
}
/* siz added: PlayerSeekOpen function - 28/07/2012 */
char * playerSeekOpen(char * file) {
	char* string = (char*)malloc(7);
	FILE * fd = fopen(file, "r");	
	fgets(string, strlen(string), fd);
	if (string[strlen(string)-1] == '\n') {
	string[strlen(string)-1] = '\0';
	}
        fclose(fd);
	return string;
}
/* siz added: Prompts if a file has been played before, and you wish to resume - Start - 25/07/2012 */
int playerSeekPrompt(char * seekfile) {
	int choice = -2;
	char *seektime = playerSeekOpen(seekfile);
	double time = atof(seektime);
	seektime = playerSeekFormatTime(seektime, time); 
	GuiWindow promptWindow(300, 72);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, 0);
	GuiImageData btnOutline(button_p_seek_png);
	GuiImageData btnOutlineOver(button_p_seek_select_png);

	GuiImageData dialogBox(p_seek_bg_png);
	GuiImage dialogBoxImg(&dialogBox);
	dialogBoxImg.SetPosition(0, 20);
	dialogBoxImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	
	GuiText btn1Txt(seektime, 22, (XeColor) {
		255, 255, 255, 255
	});
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
	btn1.SetEffectGrow();

	GuiText btn2Txt("Start from beginning", 22, (XeColor) {
		255, 255, 255, 255
	});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	btn2.SetPosition(0, 22);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	btn2.SetTrigger(trigA);
	btn2.SetEffectGrow();

	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_B);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(100, 100);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();
	menuBtn.SetSelectable(false);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&btn1);
	promptWindow.Append(&btn2);
	promptWindow.Append(&menuBtn);

	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);

	btn1.SetState(STATE_SELECTED);
	btn2.ResetState();
	while (choice == -2) {
		update();
		if (menuBtn.GetState() == STATE_CLICKED) { 
			choice = -1;
		} else {	
			if (btn1.GetState() == STATE_CLICKED) {
				choice = 1;
				playerSeekChoice = 1;
			}
			else if (btn2.GetState() == STATE_CLICKED) {
				choice = 0;
				playerSeekChoice = 0;
			}
		}
	}
	while (promptWindow.GetEffect() > 0) {
		update();
	}
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	return choice;
}
/* siz added: Prompts if a file has been played before, and you wish to resume - End - 25/07/2012 */
/** to do **/
static void loadRessources() {
	loadHomeRessources();
	loadBrowserRessources();
	loadOsdRessources();
}

static void common_setup() {
	trigA = new GuiTrigger();

	trigA->SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

	GuiImageData * background = new GuiImageData(welcome_background_bg_png);
	mainWindow = new GuiWindow(screenwidth, screenheight);
	bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);

	loadRessources();
}

static int osd_duration_bar_width;
static int osd_show = 0;
static char osd_duration[10];
static char osd_cur_time[10];
static int got_metadata = 0;

static int last_level = 0;

static char tmpbuff[1024];

extern "C" void mplayer_osd_open() {
	got_metadata = 0;

	if (osd_show == 0) {
		/** video infobar **/
		sprintf(tmpbuff, "%s %s", _("Filename:"), playetGetMetaData(META_NAME));
		video_osd_infobar_text_filename->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Codec:"), playetGetMetaData(META_VIDEO_CODEC));
		video_osd_infobar_text_trackinfo->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Resolution:"), playetGetMetaData(META_VIDEO_RESOLUTION));
		video_osd_infobar_text_resolution->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Bitrate:"), playetGetMetaData(META_VIDEO_BITRATE));
		video_osd_infobar_text_bitrate->SetText(tmpbuff);

		video_osd_infobar->SetVisible(false);

		video_osd_play->SetVisible(false);
		video_osd_pause->SetVisible(false);
		video_osd_stop->SetVisible(false);
		video_osd_next->SetVisible(false);
		video_osd_prev->SetVisible(false);
		video_osd_rewind->SetVisible(false);
		video_osd_forward->SetVisible(false);

		// order
		mainWindow->Append(video_osd_bg);
		mainWindow->Append(video_osd_progress_bar_back);
		mainWindow->Append(video_osd_progress_bar_front);

		mainWindow->Append(video_osd_info_filename);
		mainWindow->Append(video_osd_info_cur_time);
		mainWindow->Append(video_osd_info_duration);

		//
		mainWindow->Append(video_osd_infobar);

		//
		mainWindow->Append(video_osd_play);
		mainWindow->Append(video_osd_pause);
		mainWindow->Append(video_osd_stop);
		mainWindow->Append(video_osd_next);
		mainWindow->Append(video_osd_prev);
		mainWindow->Append(video_osd_rewind);
		mainWindow->Append(video_osd_forward);

		//
		mainWindow->Append(osd_options_window);

		mainWindow->Append(osd_options_subtitle_window);
		osd_options_subtitle_window->SetVisible(false);

		// remove bg
		mainWindow->Remove(bgImg);

		osd_options_window->SetFocus(1);

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		osd_duration_bar_width = img->width;
	}
	osd_show = 1;
	last_level = -1;
}

extern "C" void mplayer_osd_close() {
	if (osd_show) {

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		img->width = osd_duration_bar_width;

		mainWindow->Remove(video_osd_bg);
		mainWindow->Remove(video_osd_progress_bar_front);
		mainWindow->Remove(video_osd_progress_bar_back);


		mainWindow->Remove(video_osd_info_filename);
		mainWindow->Remove(video_osd_info_cur_time);
		mainWindow->Remove(video_osd_info_duration);

		mainWindow->Remove(video_osd_play);
		mainWindow->Remove(video_osd_pause);
		mainWindow->Remove(video_osd_stop);
		mainWindow->Remove(video_osd_next);
		mainWindow->Remove(video_osd_prev);
		mainWindow->Remove(video_osd_rewind);
		mainWindow->Remove(video_osd_forward);

		//
		mainWindow->Remove(video_osd_infobar);

		mainWindow->Remove(osd_options_window);

		// reapply bg
		mainWindow->Append(bgImg);
	}
	osd_display_info = 0;
	osd_show = 0;
	last_level = -1;
}
static void format_time(char * dest, double time) {
	div_t hrmin, minsec;
	minsec = div(time, 60);
	hrmin = div(minsec.quot, 60);

	sprintf(dest, "%d:%02d:%02d", hrmin.quot, hrmin.rem, minsec.rem);
}
extern "C" void mplayer_osd_draw(int level) {
	if (osd_show) {

		double duration = playerGetDuration();
		double elapsed = playerGetElapsed();

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		float pourcents = (float) (elapsed * 100) / (float) duration;
		float width = (float) osd_duration_bar_width * (pourcents / 100.0);
		img->width = width;

		format_time(osd_duration, duration);
		format_time(osd_cur_time, elapsed);

		video_osd_info_cur_time->SetText(osd_cur_time);
		video_osd_info_duration->SetText(osd_duration);

		if (last_level != level) {
			video_osd_info_filename->SetText(playerGetFilename());
			video_osd_info_filename->SetMaxWidth(644);
			video_osd_info_filename->SetScroll(SCROLL_HORIZONTAL);
		}

		video_osd_progress_bar_front->SetImage(img, img->width, img->height);

		video_osd_play->SetVisible(false);
		video_osd_pause->SetVisible(false);
		video_osd_stop->SetVisible(false);
		video_osd_rewind->SetVisible(false);
		video_osd_forward->SetVisible(false);
		video_osd_next->SetVisible(false);
		video_osd_prev->SetVisible(false);

		switch (playerGetStatus()) {
			case 1:
				video_osd_play->SetVisible(true);
				break;
			case 2:
				video_osd_pause->SetVisible(true);
				break;
			case 3:
				video_osd_stop->SetVisible(true);
				break;
			case 4:
				video_osd_rewind->SetVisible(true);
				break;
			case 5:
				video_osd_forward->SetVisible(true);
				break;
			case 6:
				video_osd_next->SetVisible(true);
				break;
			case 7:
				video_osd_prev->SetVisible(true);
				break;

			default:
				break;
		}

		if (osd_display_option_subtitle) {
			TR;
			int ret = osd_options_subtitle->GetClickedOption();
			switch (ret) {

			}
		}

		// show file info
		if (osd_display_info) {
			if (!video_osd_infobar->IsVisible())
				video_osd_infobar->SetVisible(true);
		} else {
			if (video_osd_infobar->IsVisible())
				video_osd_infobar->SetVisible(false);
		}

		if (level == 3) {
			if (!osd_options_window->IsVisible())
				osd_options_window->SetVisible(true);
		} else {
			if (osd_options_window->IsVisible())
				osd_options_window->SetVisible(false);
			osd_display_info = 0;
		}
	} else {
		osd_display_info = 0;
	}

	last_level = level;

	UpdatePads();
	Menu_Frame();
	mainWindow->Draw();
	//Menu_Render();
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
}

static void Browser(const char * title, const char * root) {
	// apply correct icon
	switch (current_menu) {
		case BROWSE_AUDIO:
			browser_folder_icon = browser_music_folder_icon;
			//browser_file_icon = browser_music_icon;
			break;
		case BROWSE_VIDEO:
			browser_folder_icon = browser_video_folder_icon;
			//browser_file_icon = browser_video_icon;
			extValid = extIsValidVideoExt;
			break;
		case BROWSE_PICTURE:
			browser_folder_icon = browser_photo_folder_icon;
			//browser_file_icon = browser_photo_icon;
			break;
		default:
			extValid = extAlwaysValid;
			break;
	}
	ResetBrowser();
	/* siz added: accesses the stored exited path, for SmartMenu, instead of root when entering a menu again 24/07/2012 - Start */
	if (strlen(exited_dir_array[current_menu]) != 0) {
		BrowseDevice(exited_dir_array[current_menu], root);
		gui_browser->ResetState();	
			if (exited_item[current_menu] >= gui_browser->GetPageSize()) {
			browser.pageIndex = (exited_item[current_menu] + 1 - gui_browser->GetPageSize()); 
			/* siz comment: filelist is only from 0 to 9 (pagesize is set to 10), so if pageindex is 1, item 1 is now 0 - 24/07/2012  */
			gui_browser->fileList[9]->SetState(STATE_SELECTED); 
		} else {
		gui_browser->fileList[exited_item[current_menu]]->SetState(STATE_SELECTED);		
		}
		gui_browser->TriggerUpdate();
	} else {
		BrowseDevice("/", root);
		gui_browser->ResetState();
		gui_browser->fileList[0]->SetState(STATE_SELECTED);
		gui_browser->TriggerUpdate();
	}				
	/* siz added: accesses the stored exited path, for SmartMenu, instead of root when entering a menu again 24/07/2012 - End */

	//mainWindow->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
	mainWindow->Append(gui_browser);

	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_B);

	//GuiText menuBtnTxt("B", 18, 0xffffffff);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	//menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();

	mainWindow->Append(&menuBtn);

	browser_headline->SetText(title);
	browser_subheadline->SetText(rootdir);

	mainWindow->Append(browser_top_bg);
	mainWindow->Append(browser_headline);
	mainWindow->Append(browser_subheadline);
	mainWindow->Append(browser_pagecounter);

	mainWindow->Append(browser_up_icon);
	mainWindow->Append(browser_down_icon);

	last_menu = current_menu;

	int last_sel_item = -1;

	char tmp[256];

	while (current_menu == last_menu) {
		if (last_sel_item != browser.selIndex) {
			sprintf(tmp, "%d/%d", browser.selIndex + 1, browser.numEntries);
			browser_pagecounter->SetText(tmp);
		}
	
	last_sel_item = browser.selIndex;

		if (browser.pageIndex) {
			// draw prev
			browser_up_icon->SetVisible(true);
		} else {
			browser_up_icon->SetVisible(false);
		}
//		if (browser.pageIndex + browser.selIndex + 3 < browser.numEntries) {
		if ((browser.numEntries > 9) && (browser.selIndex + 3 < browser.numEntries)) { //siz changed it
			browser_down_icon->SetVisible(true);
		} else {
			browser_down_icon->SetVisible(false);
		}
	exited_item[current_menu] = browser.selIndex; /*siz - added Save selected item, for SmartMenu: 21/07/2012 */
		// update file browser based on arrow xenon_buttons
		// set MENU_EXIT if A xenon_button pressed on a file
		for (int i = 0; i < gui_browser->GetPageSize(); i++) {
			if (gui_browser->fileList[i]->GetState() == STATE_CLICKED) {
				gui_browser->fileList[i]->ResetState();
				// check corresponding browser entry
				if (browserList[browser.selIndex].isdir) {
					if (BrowserChangeFolder()) {
						gui_browser->ResetState();
						gui_browser->fileList[0]->SetState(STATE_SELECTED);
						gui_browser->TriggerUpdate();
						sprintf(tmp, "%d/%d", 1, browser.numEntries); /*siz - fixed: not updating sel/num when entering folder 26/07/2012 */
						browser_pagecounter->SetText(tmp); /*siz - fixed: not updating sel/num when entering folder 26/07/2012 */	
					} else {
						break;
					}
				} else {
					sprintf(mplayer_filename, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);
					sprintf(exited_dir, "%s/", browser.dir); /*siz - added Save exit path, for SmartMenu: 20/07/2012 */		
					sprintf(seek_filename, "%smplayer/cache/elapsed/%s%s", device_list[0], browserList[browser.selIndex].filename, ".txt");					
					CleanupPath(mplayer_filename);								
					CleanupPath(exited_dir); /*siz - added Save exit path, for SmartMenu: 20/07/2012 */		
					strncpy(exited_dir_array[current_menu], exited_dir, 2048); /*siz - added Save exit path, for SmartMenu: 20/07/2012 */
					playerStopFile = browserList[browser.selIndex].filename;					
					ShutoffRumble();
					gui_browser->ResetState();		
					if (file_type(mplayer_filename) == BROWSER_TYPE_ELF) {
						current_menu = MENU_ELF;
					} else {
					/*siz - added for resume-playback function: 29/07/2012 - Start */	
						if ((file_exists(seek_filename)) && (playerSeekPrompt(seek_filename) == -1)) { //if B is pressed, everything is reset
						gui_browser->fileList[i]->ResetState();
							if (browser.selIndex > 9) {
								browser.pageIndex = (browser.selIndex + 1 - gui_browser->GetPageSize()); 
								gui_browser->fileList[9]->SetState(STATE_SELECTED);
							} else {
								gui_browser->fileList[browser.selIndex]->SetState(STATE_SELECTED);
							}						
								gui_browser->TriggerUpdate();
						} else if ((file_exists(seek_filename)) && (playerSeekChoice == 1)) { //if resume is pressed, play from last location
							char* seek_time = playerSeekOpen(seek_filename);
							asprintf(&playerSeekTime, "seek %s 2", seek_time);
							remove(seek_filename);
							current_menu = MENU_MPLAYER;
						} else {
							playerSeekTime = "seek 0 2";
							remove(seek_filename);
							current_menu = MENU_MPLAYER;
						}
					/*siz - added for resume-playback function: 29/07/2012 - End */	
					}
				}
			}
		}

		if (menuBtn.GetState() == STATE_CLICKED) {
		/*siz - added Save exit path, for SmartMenu: 20/07/2012 - Start */
		sprintf(exited_dir, "%s/", browser.dir); 
		CleanupPath(exited_dir);
		strncpy(exited_dir_array[current_menu], exited_dir, 2048);	
		/*siz - added Save exit path, for SmartMenu: 20/07/2012 - End */
		current_menu = MENU_BACK;
		}
		update();
	}

	mainWindow->Remove(browser_up_icon);
	mainWindow->Remove(browser_down_icon);

	mainWindow->Remove(browser_top_bg);
	mainWindow->Remove(browser_headline);
	mainWindow->Remove(browser_subheadline);
	mainWindow->Remove(browser_pagecounter);
	mainWindow->Remove(gui_browser);
	mainWindow->Remove(&menuBtn);
}

static void HomePage() {
	static int last_selected_value = 0; // Video

	mainWindow->Append(home_left);
	mainWindow->Append(home_main_function_frame_bg);
	
	home_video_txt ->SetText("Videos");
	home_all_txt ->SetText("All");
	home_music_txt ->SetText("Music"); // siz edit: Musics to Music
	home_photo_txt ->SetText("Photos");
	home_setting_txt ->SetText("Settings");
	home_restart_txt ->SetText("Restart"); /*siz - added Restart: 15/07/2012 */
	home_shutdown_txt ->SetText("Shutdown"); /*siz - added Shutdown: 15/07/2012 */

	home_list_v->Append(home_all_btn);
	home_list_v->Append(home_video_btn);
	home_list_v->Append(home_music_btn);
	home_list_v->Append(home_photo_btn);
	home_list_v->Append(home_setting_btn);
	home_list_v->Append(home_restart_btn); /*siz - added Restart: 15/07/2012 */
	home_list_v->Append(home_shutdown_btn); /*siz - added Shutdown: 15/07/2012 */

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

	//GuiText menuBtnTxt("A", 18, 0xffffffff);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	//menuBtn.SetLabel(&menuBtnTxt);
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
					xenon_smc_power_reboot(); /*siz - added Restart: 15/07/2012 */
					break;
				case 6:
					xenon_smc_power_shutdown(); /*siz - added Shutdown: 15/07/2012 */
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
	home_list_v->Remove(home_restart_btn); /*siz - added Restart: 15/07/2012 */
	home_list_v->Remove(home_shutdown_btn); /*siz - added Shutdown: 15/07/2012 */
	mainWindow->Remove(home_curitem);
}
/*siz - added file_exists: used in Subtitles Size 17/07/2012 - Start */

//Paths to subtitles
char *sub_spath = "uda0:/mplayer/font/18font.desc";
char *sub_npath = "uda0:/mplayer/font/24font.desc";
char *sub_bpath = "uda0:/mplayer/font/28font.desc";
char *sub_dpath = "uda0:/mplayer/font/font.desc";
/*siz - added file_exists: used in Subtitles Size 17/07/2012 - End */

//SETTINGS MENU
static int XMPSettings() {

	int menu = SETTINGS;
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Exit Action");
	sprintf(options.name[i++], "Language");
        sprintf(options.name[i++], "Subtitle Size"); /*siz - added subtitle size: 14/07/2012 */
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Settings - Menu", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

	//	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	//	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_blue_png);
	GuiImageData btnOutlineOver(button_green_png);

	GuiText backBtnTxt("Go Back", 22, 0xFFFFFFFF);
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	//	backBtn.SetSoundOver(&btnSoundOver);
	//	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	//backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(980, 426, new GuiImageData(browser_list_btn_png), &options);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	optionBrowser.SetCol2Position(275);

	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);

	while (menu == SETTINGS) {
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
			/*siz - added subtitle size: 14/07/2012 - Start */
			case 2: 
				XMPlayerCfg.subtitle_size++;
				if (XMPlayerCfg.subtitle_size >= SUB_LENGTH)
					XMPlayerCfg.subtitle_size = 0;
				break;
			/*siz - added subtitle size: 14/07/2012 - End */
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
			/*siz - added subtitle size: 17/07/2012 - Start */
			switch (XMPlayerCfg.subtitle_size) {
				case SUB_SMALL:{ 
				sprintf(options.value[2], "Small"); 
					if ((file_exists(sub_spath)) && (file_exists(sub_npath))) {
					rename(sub_dpath, sub_bpath);
					rename(sub_spath, sub_dpath);
				    	}
					break;
				}
				case SUB_NORMAL: {
				sprintf(options.value[2], "Normal");					
					if ((file_exists(sub_npath)) && (file_exists(sub_bpath))) {
					rename(sub_dpath, sub_spath);
					rename(sub_npath, sub_dpath);
					}
					break;
				}
				case SUB_BIG: {
				sprintf(options.value[2], "Big");
					if ((file_exists(sub_bpath)) && (file_exists(sub_spath))) {
					rename(sub_dpath, sub_npath);
					rename(sub_bpath, sub_dpath);
					}
					break;
				}
			}
			/*siz - added subtitle size: 17/07/2012 - End */

			optionBrowser.TriggerUpdate();
		}

		if (backBtn.GetState() == STATE_CLICKED) {
			menu = HOME_PAGE;
		}
	}
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);

	// save settings
	SavePrefs(true);

	//back to home page
	return menu;
}

static void do_mplayer(char * filename) {
	static int mplayer_need_init = 1;
	if (mplayer_need_init) {
		char * argv[] = {
			"mplayer.xenon",
			//"-really-quiet",
			//"-demuxer","mkv",
			"-menu",
			"-lavdopts", "skiploopfilter=all:threads=5",
			"-vsync", //enabled by siz - 16/07/2012
			filename,
		};
		mplayer_need_init = 0;
		int argc = sizeof (argv) / sizeof (char *);

		mplayer_main(argc, argv);
		// will never be here !!!
	} else {
		mplayer_load(filename);
		mplayer_return_to_player();		
		playerSeekPos(playerSeekTime); //siz
	}
}

void MenuMplayer() {
	//sprintf(foldername, "%s/", browser.dir);
	printf("filename:%s\r\n", mplayer_filename);
	do_mplayer(mplayer_filename);
}

void ElfLoader() {
	printf("Load Elf:%s\r\n", mplayer_filename);
	char * argv[] = {
		mplayer_filename,
	};
	int argc = sizeof (argv) / sizeof (char *);

	elf_setArgcArgv(argc, argv);
	elf_runFromDisk(mplayer_filename);
}

static int need_gui = 1;

static void gui_loop() {
	while (need_gui) {
		if (current_menu == HOME_PAGE) {
			HomePage();
		} else if (current_menu == BROWSE_VIDEO) {
			Browser("Videos", root_dev);
		} else if (current_menu == BROWSE_AUDIO) {
			Browser("Audios", root_dev);
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
			current_menu = XMPSettings();
		}
	}
}

static void findDevices() {
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

extern "C" void mount_all_devices();


static int loading_thread_finished = 0;
static int end_loading_thread = 0;
static unsigned char thread_stack[6][0x10000] __attribute__ ((aligned (256)));
static unsigned int loadingThreadLock = 0;

void loadingThread() {
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

int main(int argc, char** argv) {
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
	init_mplayer();
	findDevices();

	// preference
	if (LoadPrefs() == false)
		SavePrefs(true);

	// Init gui
	// french langage
	printf("XMPlayerCfg.language : %d\n",XMPlayerCfg.language);
	switch (XMPlayerCfg.language) {
		case LANG_FRENCH:
			LoadLanguage((char*) fr_lang, fr_lang_size);
			break;
	}
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
extern "C" void mplayer_return_to_gui() {
	need_gui = 1;
	
	TR;
	// always sync
	Xe_Sync(g_pVideoDevice);
	TR;
	while (!Xe_IsVBlank(g_pVideoDevice));
	Xe_InvalidateState(g_pVideoDevice);

	TR;
	// make sur to leave the gui
	mplayer_osd_close();

	current_menu = last_menu;

	TR;
	gui_loop();
}

/**
 * return to mplayer
 */
extern "C" void mplayer_return_to_player() {
	need_gui = 0;
}
