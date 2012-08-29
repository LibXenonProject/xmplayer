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

#include "../build/button_p_seek_png.h" 
#include "../build/button_p_seek_select_png.h"
#include "../build/p_seek_bg_png.h"
#include "../build/osd_options_browser_btn_png.h"
#include "../build/osd_options_browser_bg_png.h"

#include "../build/video_info_bg_png.h"

#include "../build/fr_lang.h"

#include "filebrowser.h"

#include "mplayer_func.h"
#include "mplayer_common.h"
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
static GuiText * video_osd_infobar_text_a_codec = NULL;
static GuiText * video_osd_infobar_text_a_bitrate = NULL;

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

//browser sorting
static GuiText * browser_sortText = NULL;
static GuiImage * browser_sort_up = NULL;
static GuiImage * browser_sort_down = NULL;
//osd subtitle options
static GuiWindow * osd_options_subtitle_window = NULL;
static GuiOptionBrowser * osd_options_subtitle = NULL;
static OptionList subtitle_option_list;
//osd audio options
static GuiWindow * osd_options_audio_window = NULL;
static GuiOptionBrowser * osd_options_audio = NULL;
static OptionList audio_option_list;
//osd video options
static GuiWindow * osd_options_video_window = NULL;
static GuiOptionBrowser * osd_options_video = NULL;
static OptionList video_option_list;

static GuiText * osd_options_headline = NULL;

static char mplayer_filename[2048];
static char exited_dir[2048]; 
static char exited_dir_array[64][2048];
static int exited_item[64];
const char * exited_root = ""; 
static char seek_filename[2048];
char * playerSeekTime = "";
static int playerSeekChoice = 0;

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
static int osd_display_option_video = 0;
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
		osd_display_option_video = !osd_display_option_video;
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
		//playerTurnOffSubtitle(); //turns off subs, atm if not, subs will be freezed on a video without them
		playerGuiAsked();//playback resume
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
		osd_display_option_audio = !osd_display_option_audio;
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_sub_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		osd_display_option_subtitle = !osd_display_option_subtitle;
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}
/**
 * Load ressources
 **/
static void loadHomeRessources() {
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

static void loadBrowserRessources() {

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

static void loadOsdRessources() { // OSD
	video_osd_progress_bar_front = new GuiImage(new GuiImageData(video_player_time_played_line_n_01_png));
	video_osd_progress_bar_back = new GuiImage(new GuiImageData(video_control_time_played_line_bg_png));
	video_osd_bg = new GuiImage(new GuiImageData(video_control_frame_bg_png));

	video_osd_info_filename = new GuiText("@@info_filename", 22, 0xffffffff);
	video_osd_info_cur_time = new GuiText("@@info_cur_time", 18, 0xffffffff);
	video_osd_info_duration = new GuiText("@@info_duration", 18, 0xfffa9600);

	/** video infobar **/
	video_osd_infobar_text_filename = new GuiText("Filename", 20, 0xffffffff);
	video_osd_infobar_text_trackinfo = new GuiText("Video Codec", 20, 0xffffffff);
	video_osd_infobar_text_resolution = new GuiText("Resolution", 20, 0xffffffff);
	video_osd_infobar_text_bitrate = new GuiText("Video Bitrate", 20, 0xffffffff);
	video_osd_infobar_text_a_codec = new GuiText("Audio Codec", 20, 0xffffffff);
	video_osd_infobar_text_a_bitrate = new GuiText("Audio Bitrate", 20, 0xffffffff);

	video_osd_infobar_bg = new GuiImage(new GuiImageData(video_info_bg_png));
	video_osd_infobar_bg->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	video_osd_infobar = new GuiWindow(1144, 240);
	video_osd_infobar->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	video_osd_infobar->Append(video_osd_infobar_bg);
	video_osd_infobar->Append(video_osd_infobar_text_filename);
	video_osd_infobar->Append(video_osd_infobar_text_trackinfo);
	video_osd_infobar->Append(video_osd_infobar_text_resolution);
	video_osd_infobar->Append(video_osd_infobar_text_bitrate);
	video_osd_infobar->Append(video_osd_infobar_text_a_codec);
	video_osd_infobar->Append(video_osd_infobar_text_a_bitrate);

	video_osd_infobar_text_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_trackinfo->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_resolution->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_bitrate->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_a_codec->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_text_a_bitrate->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	video_osd_infobar_text_filename->SetPosition(20, 20);
	video_osd_infobar_text_resolution->SetPosition(20, 50);
	video_osd_infobar_text_trackinfo->SetPosition(20, 80);
	video_osd_infobar_text_bitrate->SetPosition(20, 110);
	video_osd_infobar_text_a_codec->SetPosition(20, 140);
	video_osd_infobar_text_a_bitrate->SetPosition(20, 170);

	/** play state **/
	video_osd_play = new GuiImage(new GuiImageData(video_control_play_btn_png));
	video_osd_pause = new GuiImage(new GuiImageData(video_control_pause_btn_png));
	video_osd_stop = new GuiImage(new GuiImageData(video_control_stop_btn_png));
	video_osd_next = new GuiImage(new GuiImageData(video_control_next_btn_png));
	video_osd_prev = new GuiImage(new GuiImageData(video_control_previous_btn_png));
	video_osd_rewind = new GuiImage(new GuiImageData(video_control_rewind_btn_png));
	video_osd_forward = new GuiImage(new GuiImageData(video_control_fastforward_btn_png));

	/** a ranger **/
	video_osd_bg->SetPosition(149, 597);

	video_osd_progress_bar_front->SetPosition(262, 652);
	video_osd_progress_bar_back->SetPosition(262, 652);

	video_osd_info_filename->SetPosition(264, 621);
	video_osd_info_cur_time->SetPosition(943, 625);
	video_osd_info_duration->SetPosition(1023, 625);

	video_osd_info_filename->SetMaxWidth(644);
	video_osd_info_filename->SetScroll(SCROLL_HORIZONTAL);
	
	video_osd_info_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_info_cur_time->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_info_duration->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

	video_osd_play->SetPosition(202, 613);
	video_osd_pause->SetPosition(202, 613);
	video_osd_stop->SetPosition(202, 613);
	video_osd_next->SetPosition(202, 613);
	video_osd_prev->SetPosition(202, 613);
	video_osd_rewind->SetPosition(202, 613);
	video_osd_forward->SetPosition(202, 613);

	/** osd level 3**/
	osd_options_window = new GuiTab(340, 50);
	osd_options_window->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	osd_options_window->SetPosition(480, 30);

	osd_options_window->setRow(1);
	osd_options_window->setCol(5);

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

	//Order of the options window icons, left to right
	//osd_options_window->SetBackground(osd_options_bg_x7);
	osd_options_window->SetBackground(osd_options_bg_x5);
	osd_options_window->Append(osd_options_menu_audio_channel_btn); //audio menu
	osd_options_window->Append(osd_options_menu_pan_btn); //video menu
	osd_options_window->Append(osd_options_menu_subtitle_btn); //subtitle menu
	osd_options_window->Append(osd_options_menu_info_btn);
	osd_options_window->Append(osd_options_menu_next_btn);
	//	osd_options_window->Append(osd_options_menu_repeat_btn);
	//	osd_options_window->Append(osd_options_menu_zoomin_btn);
	//	osd_options_window->Append(osd_options_menu_zoomout_btn);

	//osd_options_menu_audio_channel_btn->SetFocus(1);

	// position
	for (u32 i = 1; i < osd_options_window->GetSize(); i++) {
		osd_options_window->GetGuiElementAt(i)->SetTrigger(trigA);
	}
 	
	/**
	 * Osd Subtiles window
	 */
	int i = 0;
	sprintf(subtitle_option_list.name[i++], "Subtitle:");
	sprintf(subtitle_option_list.name[i++], "Visibility:");
	sprintf(subtitle_option_list.name[i++], "Position:");
	sprintf(subtitle_option_list.name[i++], "Delay:");
	sprintf(subtitle_option_list.name[i++], "Scale:");
	sprintf(subtitle_option_list.name[i++], "BACK");
	subtitle_option_list.length = i;

	for (i = 0; i < subtitle_option_list.length; i++)
		subtitle_option_list.value[i][0] = 0;

	osd_options_subtitle_window = new GuiWindow(405, 280);
	osd_options_subtitle = new GuiOptionBrowser(400, 275, new GuiImageData(osd_options_browser_btn_png), &subtitle_option_list);
	osd_options_subtitle->SetPosition(0, 40);
	osd_options_subtitle->SetCol2Position(150);	
	osd_options_subtitle_window->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	
	osd_options_subtitle_window->Append(new GuiImage(new GuiImageData(osd_options_browser_bg_png)));
	osd_options_subtitle_window->Append(osd_options_subtitle);
	/**
	 * Osd Audio window
	 */
	int d = 0;
	sprintf(audio_option_list.name[d++], "SWITCH SOURCE");
	sprintf(audio_option_list.name[d++], "Volume:");
	sprintf(audio_option_list.name[d++], "Balance:");
	sprintf(audio_option_list.name[d++], "Mute:");
	sprintf(audio_option_list.name[d++], "Delay:");
	sprintf(audio_option_list.name[d++], "BACK");
	audio_option_list.length = d;

	for (d = 0; d < audio_option_list.length; d++)
		audio_option_list.value[d][0] = 0;

	osd_options_audio_window = new GuiWindow(405, 280);
	osd_options_audio = new GuiOptionBrowser(400, 275, new GuiImageData(osd_options_browser_btn_png), &audio_option_list);
	osd_options_audio->SetPosition(0, 40);
	osd_options_audio->SetCol2Position(150);	
	osd_options_audio_window->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	
	osd_options_audio_window->Append(new GuiImage(new GuiImageData(osd_options_browser_bg_png)));
	osd_options_audio_window->Append(osd_options_audio);
	/**
	 * Osd Audio window
	 */
	int g = 0;
	sprintf(video_option_list.name[g++], "Fullscreen:");
	sprintf(video_option_list.name[g++], "Frame Drop:");
	sprintf(video_option_list.name[g++], "Vsync:");
	sprintf(video_option_list.name[g++], "BACK");
	video_option_list.length = g;

	for (g = 0; g < video_option_list.length; g++)
		video_option_list.value[g][0] = 0;

	osd_options_video_window = new GuiWindow(405, 280);
	osd_options_video = new GuiOptionBrowser(400, 275, new GuiImageData(osd_options_browser_btn_png), &video_option_list);
	osd_options_video->SetPosition(0, 40);
	osd_options_video->SetCol2Position(150);	
	osd_options_video_window->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	
	osd_options_video_window->Append(new GuiImage(new GuiImageData(osd_options_browser_bg_png)));
	osd_options_video_window->Append(osd_options_video);

	//Headline for Osd Options
	osd_options_headline = new GuiText("@@osdheadline", 24, 0xfffa9600);
	osd_options_headline->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	osd_options_headline->SetPosition(0, -120);
	osd_options_headline->SetEffectGrow();
}
//Mplayer's subtitle variables
extern "C" int sub_pos;
extern "C" int sub_visibility;
extern "C" float sub_delay;
extern "C" float text_font_scale_factor;
extern "C" int force_load_font;
extern "C" float ass_font_scale;
extern "C" int ass_enabled;
extern "C" int ass_force_reload;
//Mplayer's audio variables
extern "C" float audio_delay;
//Mplayer's video variables
extern "C" int vo_fs; 
extern "C" int vo_vsync;
extern "C" int vo_dheight;
extern "C" int frame_dropping;
//D-Pad direction for OSD Settings
extern int osd_pad_right; //from gui_optionbrowser.cpp
extern int osd_pad_left; //from gui_optionbrowser.cpp
extern int osd_level; 
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
bool file_exists(const char * filename) {
   FILE * fd = fopen(filename, "rb");
   if (fd != NULL) {
      fclose(fd);
      return true;
   }
   return false;
}
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

		sprintf(tmpbuff, "%s %s", _("Resolution:"), playetGetMetaData(META_VIDEO_RESOLUTION));
		video_osd_infobar_text_resolution->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Video Codec:"), playetGetMetaData(META_VIDEO_CODEC));
		video_osd_infobar_text_trackinfo->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Video Bitrate:"), playetGetMetaData(META_VIDEO_BITRATE));
		video_osd_infobar_text_bitrate->SetText(tmpbuff);
		
		sprintf(tmpbuff, "%s %s", _("Audio Codec:"), playetGetMetaData(META_AUDIO_CODEC));
		video_osd_infobar_text_a_codec->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Audio Bitrate:"), playetGetMetaData(META_AUDIO_BITRATE));
		video_osd_infobar_text_a_bitrate->SetText(tmpbuff);

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

		mainWindow->Append(video_osd_infobar);

		mainWindow->Append(video_osd_play);
		mainWindow->Append(video_osd_pause);
		mainWindow->Append(video_osd_stop);
		mainWindow->Append(video_osd_next);
		mainWindow->Append(video_osd_prev);
		mainWindow->Append(video_osd_rewind);
		mainWindow->Append(video_osd_forward);

		mainWindow->Append(osd_options_window);
		mainWindow->Append(osd_options_subtitle_window);
		mainWindow->Append(osd_options_audio_window);
		mainWindow->Append(osd_options_video_window);
		mainWindow->Append(osd_options_headline);

		// remove bg
		mainWindow->Remove(bgImg);

		osd_options_window->SetFocus(1);

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		osd_duration_bar_width = img->width;
	}
	osd_show = 1;
	last_level = -1;
	osd_pad_right = 0;
	osd_pad_left = 0;
}
static void osdSubtitlesOptions() {
bool firstRun = true;
char *osd_sub_name = "";
float osd_subdelay;
  if ((osd_display_option_audio == 0) && (osd_display_option_video == 0)) { 
if (osd_display_option_subtitle) {
	osd_options_window->SetFocus(0);
	osd_options_headline->SetText("Subtitle Options");
	osd_options_subtitle_window->SetVisible(true);
	osd_options_subtitle_window->SetFocus(1);
	osd_options_subtitle->SetFocus(1);
	Menu_Frame();
	UpdatePads();	
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
	int ret = osd_options_subtitle->GetClickedOption();
	switch (ret) {
		case 0: {
			playerSwitchSubtitle();
			break;
		}
		case 1: {
			sub_visibility = (sub_visibility == 1) ? 0 : 1;
			break;
		}
		case 5: {
			osd_display_option_subtitle = 0;
			osd_options_menu_subtitle_btn->SetState(STATE_SELECTED);
			break;		
		}
	}
	int het = osd_options_subtitle->GetSelectedOption();
	if (osd_pad_left == 1) { 
		switch (het) {
			case 2: { 
				if (!ass_enabled) {				
					sub_pos--;
					if (sub_pos<0) { sub_pos = 100; }
				}				
				break;
			}
			case 3: {
				sub_delay += 0.1;
				break;
			}
			case 4: {
				if (ass_enabled) {			
					ass_font_scale -= 0.1;
					if (ass_font_scale < 0) { ass_font_scale = 100; }
					ass_force_reload = 1;
				} else {
					text_font_scale_factor -= 0.1;
					if (text_font_scale_factor < 0) { text_font_scale_factor = 100; }
					force_load_font = 1;
				}
				break;		
			}
		}
	osd_pad_left = 0;
	} else if (osd_pad_right == 1) {
		switch (het) {
			case 2: { 
				if (!ass_enabled) {				
				sub_pos++;
				if (sub_pos>100) { sub_pos = 0; }
				}
				break;
			}
			case 3: {
				sub_delay -= 0.1;
				break;
			}
			case 4: {
				if (ass_enabled) {			
					ass_font_scale += 0.1;
					if (ass_font_scale > 100) { ass_font_scale = 0; }
					ass_force_reload = 1;
				} else {
					text_font_scale_factor += 0.1;
					if (text_font_scale_factor > 100) { text_font_scale_factor = 0; }
					force_load_font = 1;
				}
				break;		
			}
		}
	osd_pad_right = 0;	
	}
	if (ret >= 0 || firstRun)  {
		firstRun = false;
		osd_sub_name = playerGetSubtitle();
		osd_subdelay = (sub_delay * 1000);
		sprintf(subtitle_option_list.value[0], osd_sub_name);
		sprintf(subtitle_option_list.value[1], "%s", sub_visibility == 1 ? "Enabled" : "Disabled");
		if (!ass_enabled) { sprintf(subtitle_option_list.value[2], "%d", sub_pos); } else { sprintf(subtitle_option_list.value[2], "%s", "Disabled"); }
		sprintf(subtitle_option_list.value[3], "%.0f ms", osd_subdelay);
		sprintf(subtitle_option_list.value[4], "%.2f", ass_enabled == 1 ? ass_font_scale : text_font_scale_factor);
		osd_options_subtitle->TriggerUpdate();
	}
   } else {
	osd_options_headline->SetText(NULL); 
	osd_options_subtitle_window->SetVisible(false);
	osd_options_subtitle_window->SetFocus(0);
	osd_options_subtitle->SetFocus(0);
	osd_options_window->SetFocus(1);
	}
   }
}
static void osdAudioOptions() {
bool firstRun = true;
char *osd_mute = "";
char *osd_balance = "";
char *osd_volume = "";
float osd_audiodelay;
   if ((osd_display_option_subtitle == 0) && (osd_display_option_video == 0)) {
if (osd_display_option_audio) {
	osd_options_window->SetFocus(0);
	osd_options_headline->SetText("Audio Options");
	osd_options_audio_window->SetVisible(true);
	osd_options_audio_window->SetFocus(1);
	osd_options_audio->SetFocus(1);
	Menu_Frame();
	UpdatePads();	
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
	int ret = osd_options_audio->GetClickedOption();
	switch (ret) {
		case 0: {
			playerSwitchAudio();
			break;
		}
		case 3: {
			playerSwitchMute();
			break;	
		}
		case 5: {
			osd_display_option_audio = 0;
			osd_options_menu_audio_channel_btn->SetState(STATE_SELECTED);
			break;		
		}
	}
	int het = osd_options_audio->GetSelectedOption();
	if (osd_pad_left == 1) { 
		switch (het) {
			case 1: { 
				playerSwitchVolume(0);
				break;
			}
			case 2: {
				playerSwitchBalance(1);
				break;
			}
			case 4: {
				audio_delay += 0.1; 
				break;		
			}
		}
	osd_pad_left = 0;
	} else if (osd_pad_right == 1) {
		switch (het) {
			case 1: { 
				playerSwitchVolume(1);
				break;
			}
			case 2: {
				playerSwitchBalance(0);		
				break;
			}
			case 4: {
				audio_delay -= 0.1; 
				break;		
			}
		}
	osd_pad_right = 0;	
	}
	if (ret >= 0 || firstRun)  {
		firstRun = false;
		osd_volume = playerGetVolume();
		osd_balance = playerGetBalance();
		osd_mute = playerGetMute();
		osd_audiodelay = (audio_delay * -1000);
		sprintf(audio_option_list.value[1], osd_volume);
		sprintf(audio_option_list.value[2], osd_balance);
		sprintf(audio_option_list.value[3], osd_mute);
		sprintf(audio_option_list.value[4], "%.0f ms", osd_audiodelay);
		osd_options_audio->TriggerUpdate();
	}
   } else {
	osd_options_headline->SetText(NULL); 
	osd_options_audio_window->SetVisible(false);
	osd_options_audio_window->SetFocus(0);
	osd_options_audio->SetFocus(0);
	osd_options_window->SetFocus(1);
	}
  }
}
static void osdVideoOptions() {
bool firstRun = true;
char* osd_framedrop = "";
   if ((osd_display_option_subtitle == 0) && (osd_display_option_audio == 0)) {
if (osd_display_option_video) {
	osd_options_window->SetFocus(0);
	osd_options_headline->SetText("Video Options");
	osd_options_video_window->SetVisible(true);
	osd_options_video_window->SetFocus(1);
	osd_options_video->SetFocus(1);
	Menu_Frame();
	UpdatePads();	
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
	int ret = osd_options_video->GetClickedOption();
	switch (ret) {
		case 0: {
			playerSwitchFullscreen();
			break;
		}
		case 1: {
			frame_dropping++;
			if (frame_dropping > 2) { frame_dropping = 0; }
			break;
		}
		case 2: {
			vo_vsync = !vo_vsync;
			break;
		}
		case 3: {
			osd_display_option_video = 0;
			osd_options_menu_pan_btn->SetState(STATE_SELECTED);
			break;		
		}
	}
	if (ret >= 0 || firstRun)  {
		firstRun = false;
		if (frame_dropping == 2) { osd_framedrop = "Hard"; } else if (frame_dropping == 1) { osd_framedrop = "Enabled"; } else { osd_framedrop = "Disabled"; }
		sprintf(video_option_list.value[0], "%s", vo_fs == 1 ? "Enabled" : "Disabled");
		sprintf(video_option_list.value[1], osd_framedrop);
		sprintf(video_option_list.value[2], "%s", vo_vsync == 1 ? "Enabled" : "Disabled");
		osd_options_video->TriggerUpdate();
	}
   } else {
	osd_options_headline->SetText(NULL); 
	osd_options_video_window->SetVisible(false);
	osd_options_video_window->SetFocus(0);
	osd_options_video->SetFocus(0);
	osd_options_window->SetFocus(1);
	}
  }
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

		mainWindow->Remove(video_osd_infobar);

		mainWindow->Remove(osd_options_window);
		mainWindow->Remove(osd_options_subtitle_window);
		mainWindow->Remove(osd_options_audio_window);
		mainWindow->Remove(osd_options_video_window);
		mainWindow->Remove(osd_options_headline);
		// reapply bg
		mainWindow->Append(bgImg);
	}
	osd_display_info = 0;
	osd_show = 0;
	osd_display_option_subtitle = 0;
	osd_display_option_audio = 0;
	osd_display_option_video = 0;
	last_level = -1;
}
static void format_time(char * dest, double time) {
	div_t hrmin, minsec;
	minsec = div(time, 60);
	hrmin = div(minsec.quot, 60);

	sprintf(dest, "%d:%02d:%02d", hrmin.quot, hrmin.rem, minsec.rem);
}
extern "C" void mplayer_osd_draw(int level) {
	//Y-osd button used because libmenu is off
	GuiTrigger osdMenu;
	osdMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_Y);
	GuiButton osdBtn(20, 20);
	osdBtn.SetTrigger(&osdMenu);
	osdBtn.SetSelectable(false);
	mainWindow->Append(&osdBtn);
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
			osd_display_option_subtitle = 0;
			osd_display_option_audio = 0;
			osd_display_option_video = 0;
		}
	} else {
		osd_display_info = 0;
	}
	osdSubtitlesOptions();
	osdAudioOptions();
	osdVideoOptions();
	Menu_Frame();
	last_level = level;	
	mainWindow->Draw();
	if (level == 3) { //this fixes non intended key-presses when osd is not 3 (like when seekbar is present)
	UpdatePads();	
		for (int i = 0; i < 4; i++) {
			mainWindow->Update(&userInput[i]);
		}
		if ((playerGetPause() == 1) && (osdBtn.GetState() == STATE_CLICKED)) {
			osd_level = 1;
		}
	}
	mainWindow->Remove(&osdBtn);
}

static void Browser(const char * title, const char * root) {
	// apply correct icon
	switch (current_menu) {
		case BROWSE_AUDIO:
			browser_folder_icon = browser_music_folder_icon;
			break;
		case BROWSE_VIDEO:
			browser_folder_icon = browser_video_folder_icon;
			extValid = extIsValidVideoExt;
			break;
		case BROWSE_PICTURE:
			browser_folder_icon = browser_photo_folder_icon;
			break;
		default:
			extValid = extAlwaysValid;
			break;
	}
	ResetBrowser();
	if ((strlen(exited_dir_array[current_menu]) != 0) && (exited_root == root)) {
browser_select:	BrowseDevice(exited_dir_array[current_menu], root);
		gui_browser->ResetState();	
		//filelist is only from 0 to 9 (pagesize is set to 10), so if pageindex is 1, item 1 is now 0
		  if (exited_item[current_menu] >= gui_browser->GetPageSize()) {
			browser.pageIndex = (exited_item[current_menu] + 1 - gui_browser->GetPageSize()); 
			gui_browser->fileList[9]->SetState(STATE_SELECTED); 
		} else {
		gui_browser->fileList[exited_item[current_menu]]->SetState(STATE_SELECTED);		
		}
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
	last_menu = current_menu;
	int last_sel_item = -1;

	char tmp[256];

	while (current_menu == last_menu) {
		if (last_sel_item != browser.selIndex) {
browser_counter:	sprintf(tmp, "%d/%d", browser.selIndex + 1, browser.numEntries);
			browser_pagecounter->SetText(tmp);
		}
	
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
						goto browser_counter;
					} else {
						break;
					}
				} else {
					sprintf(mplayer_filename, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);
					sprintf(exited_dir, "%s/", browser.dir); 
					sprintf(seek_filename, "%smplayer/cache/elapsed/%s%s", device_list[0], browserList[browser.selIndex].filename, ".txt");					
					CleanupPath(mplayer_filename);								
					CleanupPath(exited_dir); 
					strncpy(exited_dir_array[current_menu], exited_dir, 2048);
					exited_root = root;
					ShutoffRumble();
					gui_browser->ResetState();		
					if (file_type(mplayer_filename) == BROWSER_TYPE_ELF) {
						current_menu = MENU_ELF;
					} else {
						if ((file_exists(seek_filename)) && (playerSeekPrompt(seek_filename) == -1)) { 
							goto browser_select;
						} else {
						    if ((file_exists(seek_filename)) && (playerSeekChoice == 1)) {
							char* seek_time = playerSeekOpen(seek_filename);
							asprintf(&playerSeekTime, "seek %s 2", seek_time);
							} else {
							playerSeekTime = "seek 0 2";
							}
						remove(seek_filename);
						current_menu = MENU_MPLAYER;	
						}
						
					}
				}
			}
		}
		//Sort button selection
		if (browser_sortBtn.GetState() == STATE_CLICKED) {
			XMPlayerCfg.sort_order++;
			if (XMPlayerCfg.sort_order > 3) {
				XMPlayerCfg.sort_order = 0;
			} 
			sprintf(exited_dir, "%s/", browser.dir); 
			CleanupPath(exited_dir);
			strncpy(exited_dir_array[current_menu], exited_dir, 2048);
			goto browser_select;
		}
		if (bBtn.GetState() == STATE_CLICKED) {
		      if (strcmp(browserList[0].filename, "..") == 0) {
			bBtn.ResetState();
			browser.selIndex = 0;
			BrowserChangeFolder();
			gui_browser->ResetState();
			gui_browser->fileList[0]->SetState(STATE_SELECTED);
			gui_browser->TriggerUpdate();
			goto browser_counter;
		      } else {
			goto browser_exit;
		      }		
		}
		if (backBtn.GetState() == STATE_CLICKED) {
browser_exit:	sprintf(exited_dir, "%s/", browser.dir); 
		CleanupPath(exited_dir);
		strncpy(exited_dir_array[current_menu], exited_dir, 2048);	
		exited_root = root;
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
	mainWindow->Remove(&bBtn);
	mainWindow->Remove(&backBtn);
	mainWindow->Remove(&browser_sortBtn);	
	mainWindow->Remove(browser_sortText);
	mainWindow->Remove(browser_sort_up);
	mainWindow->Remove(browser_sort_down);	
}

static void HomePage() {
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
					xenon_smc_power_shutdown();
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

//SETTINGS MENU
static int XMPSettings() {

	int menu = SETTINGS;
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Exit Action");
	sprintf(options.name[i++], "Language");
	options.length = i;

	for (i = 0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Settings - Menu", 26, 0xfffa9600);
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50, 50);

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
			//"-menu",
			"-ass",
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
