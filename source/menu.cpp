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
#include <sys/iosupport.h>

#include <time/time.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_list.h"
#include "libwiigui/gui_tab.h"

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
#include "../build/logo_png.h"
#include "../build/browser_list_btn_png.h"
#include "../build/browser_folder_icon_f_png.h"

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

#include "../build/options_menu_bg_x5_png.h"
#include "../build/options_menu_bg_x6_png.h"
#include "../build/options_menu_bg_x7_png.h"

#include "../build/video_info_bg_png.h"

#include "filebrowser.h"

#include "mplayer_func.h"
#include "folder_video_icon_png.h"

char * root_dev = NULL;

static int device_list_size = 0;
static char device_list[STD_MAX][10];

// used for osd (from getch2-xenon.c)

enum {
	MENU_BACK = -1,
	HOME_PAGE = 1,
	MENU_MPLAYER,
	BROWSE = 0x10,
	BROWSE_VIDEO,
	BROWSE_AUDIO,
	BROWSE_PICTURE,
	OSD = 0x20,
};

/**
 * used at loading
 **/
static XenosSurface * logo = NULL;

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

static GuiTab * video_osd_infobar = NULL;
static GuiImage * video_osd_infobar_bg = NULL;
static GuiText * video_osd_infobar_text_filename = NULL;
static GuiText * video_osd_infobar_info_filename = NULL;
static GuiText * video_osd_infobar_text_trackinfo = NULL;
static GuiText * video_osd_infobar_info_trackinfo = NULL;
static GuiText * video_osd_infobar_text_resolution = NULL;
static GuiText * video_osd_infobar_info_resolution = NULL;
static GuiText * video_osd_infobar_text_bitrate = NULL;
static GuiText * video_osd_infobar_info_bitrate = NULL;

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
static GuiImageData * browser_file_icon = NULL;

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
static GuiButton * home_music_btn = NULL;
static GuiButton * home_photo_btn = NULL;
static GuiButton * home_setting_btn = NULL;

static GuiImage * home_video_img = NULL;
static GuiImage * home_music_img = NULL;
static GuiImage * home_photo_img = NULL;
static GuiImage * home_setting_img = NULL;

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

static GuiButton * osd_options_menu_audio_channel_btn = NULL;
static GuiButton * osd_options_menu_info_btn = NULL;
static GuiButton * osd_options_menu_pan_btn = NULL;
static GuiButton * osd_options_menu_repeat_btn = NULL;
static GuiButton * osd_options_menu_subtitle_btn = NULL;
static GuiButton * osd_options_menu_zoomin_btn = NULL;
static GuiButton * osd_options_menu_zoomout_btn = NULL;

static char mplayer_filename[2048];

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

static void osd_option_default_callback(void * data){
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
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_sub_callback(void * data) {
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		playerSwitchSubtitle();
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

	home_list_v = new GuiList(200, 440);
	home_list_h = new GuiList(465, 110, 'H');

	// <wgt_mlist name="VERT_MLIST" image="" x="305" y="140" w="200" h="440" fontsize="26" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="vert" itemcount="4" extra_index="3" extra_len="0"/>
	home_list_v->SetPosition(305, 140);
	home_list_v->SetCount(4);
	home_list_v->SetCenter(3);

	// <wgt_mlist name="HORI_MLIST" x="560" y="360" w="465" h="110" fontsize="20" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="hori" itemcount="3"/>
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

	home_video_btn = new GuiButton(home_video_img->GetWidth(), home_video_img->GetHeight());
	home_music_btn = new GuiButton(home_music_img->GetWidth(), home_music_img->GetHeight());
	home_photo_btn = new GuiButton(home_photo_img->GetWidth(), home_photo_img->GetHeight());
	home_setting_btn = new GuiButton(home_setting_img->GetWidth(), home_setting_img->GetHeight());

	home_video_btn->SetIcon(home_video_img);
	home_music_btn->SetIcon(home_music_img);
	home_photo_btn->SetIcon(home_photo_img);
	home_setting_btn->SetIcon(home_setting_img);
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
	
	browser_pagecounter = new GuiText("@@pagecounter",24,0xFFFFFFFF);		
	browser_pagecounter->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_pagecounter->SetPosition(980, 54);
	browser_pagecounter->SetMaxWidth(200);
	browser_pagecounter->SetStyle(FTGX_JUSTIFY_RIGHT);
	
	browser_subheadline = new GuiText("@@subheadline",24,0xFFFFFFFF);
	browser_subheadline->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_subheadline->SetPosition(100, 70);

	browser_headline = new GuiText("@@headline", 32, 0xfffa9600);
	browser_headline->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_headline->SetPosition(100, 40);
	browser_headline->SetEffectGrow();
	
	browser_up_icon = new GuiImage(new GuiImageData(browser_list_arrow_up_png));
	browser_down_icon =  new GuiImage(new GuiImageData(browser_list_arrow_down_png));;
	
	// <image image="@@moreprev" x="150" y="650" w="19" h="13"/>
	// <image image="@@morenext" x="1111" y="650" w="19" h="13"/>
	browser_up_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_down_icon->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_up_icon->SetPosition(150, 650);
	browser_down_icon->SetPosition(1111, 650);
	
	browser_top_bg = new GuiImage(new GuiImageData(browser_top_png));
	browser_top_bg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browser_top_bg->SetPosition(0, 0);
	
	// no image data, pointer to image
	browser_folder_icon = browser_video_folder_icon;
	browser_file_icon = browser_video_icon;

	gui_browser = new GuiFileBrowser(980, 500, browser_selector, browser_folder_icon, browser_file_icon);
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
	video_osd_infobar_text_filename = new GuiText("@@info_bar_filename", 18, 0xffffffff);
	video_osd_infobar_info_filename = new GuiText("@@video_osd_infobar_info_filename", 18, 0xffffffff);
	video_osd_infobar_text_trackinfo = new GuiText("@@info_bar_trackinfo", 18, 0xffffffff);
	video_osd_infobar_info_trackinfo = new GuiText("@@video_osd_infobar_info_trackinfo", 18, 0xffffffff);
	video_osd_infobar_text_resolution = new GuiText("@@info_bar_resolution", 18, 0xffffffff);
	video_osd_infobar_info_resolution = new GuiText("video_osd_infobar_info_resolution", 18, 0xffffffff);
	video_osd_infobar_text_bitrate = new GuiText("@@info_bar_bitrate", 18, 0xffffffff);
	video_osd_infobar_info_bitrate = new GuiText("@@video_osd_infobar_info_bitrate", 18, 0xffffffff);

	video_osd_infobar_bg = new GuiImage(new GuiImageData(video_info_bg_png));
	
	video_osd_infobar_bg->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	video_osd_infobar_bg->SetPosition(68,240);
	
	
	video_osd_infobar = new GuiTab(500, 80);
	video_osd_infobar->SetPosition(110, 350);
	
	video_osd_infobar->setCol(2);
	video_osd_infobar->setRow(4);
	
	video_osd_infobar->SetBackground(video_osd_infobar_bg);
		
	video_osd_infobar->Append(video_osd_infobar_text_filename);
	video_osd_infobar->Append(video_osd_infobar_info_filename);
	video_osd_infobar->Append(video_osd_infobar_text_trackinfo);
	video_osd_infobar->Append(video_osd_infobar_info_trackinfo);
	video_osd_infobar->Append(video_osd_infobar_text_resolution);
	video_osd_infobar->Append(video_osd_infobar_info_resolution);
	video_osd_infobar->Append(video_osd_infobar_text_bitrate);
	video_osd_infobar->Append(video_osd_infobar_info_bitrate);

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
	osd_options_window = new GuiTab(410, 50);
	osd_options_window->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	osd_options_window->SetPosition(435, 30);

	osd_options_window->setRow(1);
	osd_options_window->setCol(7);


	// <image image="image/slideshow_options_menu_bg_x7.png" x="410" y="0" w="460" h="91" bg="1" disable="@@disable-options_bg"/>
	osd_options_bg_x5 = new GuiImage(new GuiImageData(options_menu_bg_x5_png));
	osd_options_bg_x6 = new GuiImage(new GuiImageData(options_menu_bg_x6_png));
	osd_options_bg_x7 = new GuiImage(new GuiImageData(options_menu_bg_x7_png));

	osd_options_bg_x7->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	osd_options_bg_x7->SetPosition(410, 0);

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

	osd_options_menu_audio_channel_btn = new GuiButton(osd_options_menu_audio_channel_icon_f->GetWidth(), osd_options_menu_audio_channel_icon_f->GetHeight());
	osd_options_menu_info_btn = new GuiButton(options_menu_info_icon_f->GetWidth(), options_menu_info_icon_f->GetHeight());
	osd_options_menu_pan_btn = new GuiButton(options_menu_pan_icon_f->GetWidth(), options_menu_pan_icon_f->GetHeight());
	osd_options_menu_repeat_btn = new GuiButton(options_menu_repeat_icon_d->GetWidth(), options_menu_repeat_icon_d->GetHeight());
	osd_options_menu_subtitle_btn = new GuiButton(options_menu_subtitle_icon_f->GetWidth(), options_menu_subtitle_icon_f->GetHeight());
	osd_options_menu_zoomin_btn = new GuiButton(options_menu_zoomin_icon_f->GetWidth(), options_menu_zoomin_icon_f->GetHeight());
	osd_options_menu_zoomout_btn = new GuiButton(options_menu_zoomout_icon_f->GetWidth(), options_menu_zoomout_icon_f->GetHeight());

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

	// callback*
	
	osd_options_menu_audio_channel_btn->SetUpdateCallback(osd_options_audio_callback);
	osd_options_menu_info_btn->SetUpdateCallback(osd_options_info_callback);
	osd_options_menu_pan_btn->SetUpdateCallback(osd_options_pan_callback);
	osd_options_menu_repeat_btn->SetUpdateCallback(osd_options_loop_callback);
	osd_options_menu_subtitle_btn->SetUpdateCallback(osd_options_sub_callback);
	osd_options_menu_zoomin_btn->SetUpdateCallback(osd_option_default_callback);
	osd_options_menu_zoomout_btn->SetUpdateCallback(osd_option_default_callback);
	
	osd_options_window->SetBackground(osd_options_bg_x7);
	osd_options_window->Append(osd_options_menu_audio_channel_btn);
	osd_options_window->Append(osd_options_menu_info_btn);
	osd_options_window->Append(osd_options_menu_pan_btn);
	osd_options_window->Append(osd_options_menu_repeat_btn);
	osd_options_window->Append(osd_options_menu_subtitle_btn);
	osd_options_window->Append(osd_options_menu_zoomin_btn);
	osd_options_window->Append(osd_options_menu_zoomout_btn);

	osd_options_menu_audio_channel_btn->SetFocus(1);

	// position
	for (u32 i = 1; i < osd_options_window->GetSize(); i++) {
		osd_options_window->GetGuiElementAt(i)->SetTrigger(trigA);
	}
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

extern "C" void mplayer_osd_open() {
	got_metadata = 0;

	if (osd_show == 0) {
		/** video infobar **/
		video_osd_infobar_info_filename->SetText(playetGetMetaData(META_NAME));
		video_osd_infobar_info_trackinfo->SetText(playetGetMetaData(META_VIDEO_CODEC));
		video_osd_infobar_info_resolution->SetText(playetGetMetaData(META_VIDEO_RESOLUTION));
		video_osd_infobar_info_bitrate->SetText(playetGetMetaData(META_VIDEO_BITRATE));

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

		// remove bg
		mainWindow->Remove(bgImg);

		osd_options_window->SetFocus(1);

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		osd_duration_bar_width = img->width;
	}
	osd_show = 1;
	last_level=-1;
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
	last_level=-1;
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

		if(last_level!=level){
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
		}
	} else {
		osd_display_info = 0;
	}
	
	last_level=level;

	UpdatePads();
	Menu_Frame();
	mainWindow->Draw();
	//Menu_Render();
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
}

static void Browser(const char * title, const char * root) {
	gui_browser->ResetState();
	ResetBrowser();
	BrowseDevice("/", root);

	// apply correct icon
	switch (current_menu) {
		case BROWSE_AUDIO:
			browser_folder_icon = browser_music_folder_icon;
			browser_file_icon = browser_music_icon;
			break;

		case BROWSE_VIDEO:
			browser_folder_icon = browser_video_folder_icon;
			browser_file_icon = browser_video_icon;
			break;
		case BROWSE_PICTURE:
			browser_folder_icon = browser_photo_folder_icon;
			browser_file_icon = browser_photo_icon;
			break;

		default:
			break;
	}

	
	//mainWindow->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
	mainWindow->Append(gui_browser);

	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_B);

	GuiText menuBtnTxt("B", 18, 0xffffffff);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	menuBtn.SetLabel(&menuBtnTxt);
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
		if(last_sel_item!=browser.selIndex){
			sprintf(tmp,"%d/%d",browser.selIndex+1,browser.numEntries);
			browser_pagecounter->SetText(tmp);
		}
		
		last_sel_item = browser.selIndex;
		
		if(browser.pageIndex){
			// draw prev
			browser_up_icon->SetVisible(true);
		}
		else{
			browser_up_icon->SetVisible(false);
		}
		//if (browser.selIndex<browser.numEntries * ) {
		if (browser.pageIndex + browser.selIndex +3 < browser.numEntries){
			browser_down_icon->SetVisible(true);
		}
		else{			
			browser_down_icon->SetVisible(false);
		}

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
					} else {
						break;
					}
				} else {
					current_menu = MENU_MPLAYER;
					//mplayer_main(NULL,NULL);

					sprintf(mplayer_filename, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);

					CleanupPath(mplayer_filename);

					ShutoffRumble();
					gui_browser->ResetState();
				}
			}
		}

		if (menuBtn.GetState() == STATE_CLICKED) {
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
	mainWindow->Append(home_left);
	mainWindow->Append(home_main_function_frame_bg);


	home_list_v->Append(home_video_btn);
	home_list_v->Append(home_music_btn);
	home_list_v->Append(home_photo_btn);
	home_list_v->Append(home_setting_btn);

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

	GuiText menuBtnTxt("A", 18, 0xffffffff);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();

	mainWindow->Append(&menuBtn);

	static int last_device = -1;
	
	while (current_menu == HOME_PAGE) {
		
		int h_val = home_list_h->GetValue();
		
		if(last_device!=h_val){
			home_curitem->SetText(device_list[h_val]);
		}
		
		last_device = h_val;
		
		if (menuBtn.GetState() == STATE_CLICKED) {
			switch (home_list_v->GetValue()) {
				case 0:
					current_menu = BROWSE_VIDEO;
					break;
				case 1:
					current_menu = BROWSE_AUDIO;
					break;
				case 2:
					current_menu = BROWSE_PICTURE;
					break;
					//				case 3:
					//					current_menu = BROWSE_VIDEO;
					//					break;
			}
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
	mainWindow->Remove(home_curitem);
}

static void do_mplayer(char * filename) {
	static int mplayer_need_init = 1;
	if (mplayer_need_init) {
		char * argv[] = {
			"mplayer.xenon",
			"-really-quiet",
			//"-demuxer","mkv",
			"-menu",
			//"-menu-startup",
			"-lavdopts","skiploopfilter=all:threads=5",
		//	"-lavdopts", "skiploopfilter=all:threads=2",
			"-vsync",
			//"uda:/mplayer/loop.mov","-loop","0",
			//"-lavdopts","skiploopfilter=all",
			//"-novideo",
			//"-vo","null",
			//"-vc","ffmpeg4",
			//"-v",
			//"-nosound",
			//"-vfm","xvid",
			//"-ao","null",
			//"-dvd-device","uda:/dvd/THE_SMURFS/","dvd://1",
			//			"uda:/video.avi",
			//			"uda:/dbz.avi",
			//"uda:/video.m2ts",
			//"uda:/video2.mp4",
			//"dvd://1"
			//"uda:/trailer.mkv",
			//"uda:/lockout-tlr1_h1080p.mov"
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

static int need_gui = 1;

static void gui_loop() {
	while (need_gui) {
		if (current_menu == HOME_PAGE) {
			HomePage();
		} else if (current_menu == BROWSE_VIDEO) {
			Browser("Video", root_dev);
		} else if (current_menu == BROWSE_AUDIO) {
			Browser("Audio", root_dev);
		} else if (current_menu == BROWSE_PICTURE) {
			Browser("Photo", root_dev);
		} else if (current_menu == MENU_MPLAYER) {
			MenuMplayer();
		} else if (current_menu == MENU_BACK) {
			current_menu = HOME_PAGE;
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

int main(int argc, char** argv) {
	xenon_make_it_faster(XENON_SPEED_FULL);
	
	// Init Video
	InitVideo();

	/** loool **/
	logo = loadPNGFromMemory((unsigned char*) logo_png);
	
	Xe_SetClearColor(g_pVideoDevice, 0xFFFFFFFF);
	
	Menu_DrawImg(0, 0, 1280, 720, logo, 0, 1, 1, 0xff);

	Menu_Render();

	// Init Freetype
	InitFreeType((u8*) font_ttf, font_ttf_size);

	// usb
	usb_init();
	xenon_ata_init();
	usb_do_poll();
	
	mount_all_devices();
	
	init_mplayer();

	findDevices();

	// pads
	SetupPads();

	ChangeFontSize(26);

	common_setup();

	current_menu = HOME_PAGE;

	//current_menu = OSD;
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

	// make sur to leave the gui
	mplayer_osd_close();

	current_menu = last_menu;

	gui_loop();
}

/**
 * return to mplayer
 */
extern "C" void mplayer_return_to_player() {
	need_gui = 0;
}
