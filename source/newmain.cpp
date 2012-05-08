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
#include <usb/usbmain.h>
#include <diskio/ata.h>
#include <xenon_soc/xenon_power.h>
#include <sys/iosupport.h>

#include <time/time.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_list.h"

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

#include "../build/folder_music_icon_png.h"
#include "../build/folder_photo_icon_png.h"
#include "../build/browser_video_icon_f_png.h"

#include "filebrowser.h"

#include "mplayer_func.h"
#include "folder_video_icon_png.h"


#define GuiCreateButton(name,text,_png,_png_over) \
	GuiImageData image_data_ ## name(_png); \
	GuiImage image_##name(&image_data_##name); \
	GuiImageData image_data_ ## name ## _over(_png_over); \
	GuiImage image_##name##_over(&image_data_##name##_over); \
	GuiButton button_##name(image_data_##name.GetWidth(), image_data_##name.GetHeight()); \
	button_##name.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE); \
	GuiText txt_##name(text, 26, white); \
	txt_##name.SetWrap(true, image_data_##name.GetWidth() - 30); \
	button_##name.SetLabel(&txt_##name); \
	button_##name.SetImage(&image_##name); \
	button_##name.SetImageOver(&image_##name##_over); \
	button_##name.SetEffectGrow(); 


char * root_dev = NULL;

static int device_list_size = 0;
static char device_list[STD_MAX][10];

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

const XeColor white = XeColor{255, 255, 255, 255};

static GuiImage * bgImg = NULL;
static GuiWindow * mainWindow = NULL;
static GuiTrigger * trigA;

static XenosSurface * logo = NULL;

static GuiImage * video_osd_progress_bar_front = NULL;
static GuiImage * video_osd_progress_bar_back = NULL;
static GuiImage * video_osd_bg = NULL;

static GuiImageData * browser_photo_icon = NULL;
static GuiImageData * browser_video_icon = NULL;
static GuiImageData * browser_music_icon = NULL;

static GuiImageData * browser_photo_folder_icon = NULL;
static GuiImageData * browser_video_folder_icon = NULL;
static GuiImageData * browser_music_folder_icon = NULL;

static GuiImageData * browser_selector = NULL;

// no image data, pointer to image
static GuiImageData * browser_folder_icon = NULL;
static GuiImageData * browser_file_icon = NULL;


static GuiText * video_osd_info_filename = NULL;
static GuiText * video_osd_info_cur_time = NULL;
static GuiText * video_osd_info_duration = NULL;

static GuiWindow * video_osd_infobar = NULL;
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

static GuiImage * decoration_state = NULL;
static GuiImage * decoration_keyicon = NULL;
static GuiImage * decoration_keyicon_ex = NULL;
static GuiImage * decoration_wrongkeyicon = NULL;

static GuiList * home_list_v = NULL;
static GuiList * home_list_h = NULL;
static GuiImage * home_list_h_selector = NULL;
static GuiText * home_curitem = NULL;
static char * home_curitem_text = NULL;

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

/** to do **/
static void loadRessources() {
	// HOME PAGE


	//	<image image="image/home_left.png" x="0" y="0" w="522" h="720" bg="1"/>
	//	<image image="image/home_main_function_frame_bg.png" x="0" y="341" w="1280" h="148" bg="1"/>

	home_left = new GuiImage(new GuiImageData(home_left_png));
	home_main_function_frame_bg = new GuiImage(new GuiImageData(home_main_function_frame_bg_png));

	home_main_function_frame_bg->SetPosition(0, 341);
		
	home_list_v= new GuiList(200, 440);
	home_list_v->SetPosition(305, 140);
	home_list_v->SetCount(4);
	home_list_v->SetCenter(3);
	
	home_list_h = new GuiList(465, 110, 'H');
	home_list_h->SetPosition(560, 360);
	home_list_h->SetCount(3);
	home_list_h->SetCenter(2);
	
	
	home_list_h_selector = new GuiImage(new GuiImageData(home_horizontal_select_bar_png));
	home_list_h_selector->SetPosition(715, 350);

	// <text text="@@curitem" x="550" y="610" w="650" h="40" align="right" fontsize="32" textcolor="0xffffff"/>
	home_curitem = new GuiText("@@curitem",32,white);
	home_curitem->SetPosition(550,610);	
	home_curitem->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	
	for(int i = 0;i<device_list_size;i++){
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

	// Browser
	browser_photo_icon = new GuiImageData(browser_photo_icon_f_png);
	browser_video_icon = new GuiImageData(browser_video_icon_f_png);
	browser_music_icon = new GuiImageData(browser_music_icon_f_png);

	//	browser_photo_folder_icon = new GuiImageData(folder_photo_icon_png);
	//	browser_video_folder_icon = new GuiImageData(folder_video_icon_png);
	//	browser_music_folder_icon = new GuiImageData(folder_music_icon_png);

	browser_photo_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_video_folder_icon = new GuiImageData(browser_folder_icon_f_png);
	browser_music_folder_icon = new GuiImageData(browser_folder_icon_f_png);

	browser_selector = new GuiImageData(browser_list_btn_png);

	// no image data, pointer to image
	browser_folder_icon = browser_video_folder_icon;
	browser_file_icon = browser_video_icon;

	// OSD
	video_osd_progress_bar_front = new GuiImage(new GuiImageData(video_player_time_played_line_n_01_png));
	video_osd_progress_bar_back = new GuiImage(new GuiImageData(video_control_time_played_line_bg_png));
	video_osd_bg = new GuiImage(new GuiImageData(video_control_frame_bg_png));

	//<text text="@@info_filename" x="264" y="621" w="644" h="24" fontsize="22" textcolor="0xffffff" speed="1" delay="2" align="left" auto_translate="1"/>
	//<text text="@@info_cur_time" x="943" y="625" w="80" h="20" fontsize="18" textcolor="0xffffff" align="left"/>
	//<text text="@@info_duration" x="1023" y="625" w="95" h="20" fontsize="18" textcolor="0x0096fa" align="left"/>

	//<image x="202" y="613" w="40" h="40" image="@@speed_state"/>
	//<image image="@@play_state" x="202" y="613" w="40" h="40"/>

	XeColor blue;
	blue.a = 0xff;
	blue.r = 0x00;
	blue.g = 0x96;
	blue.b = 0xfa;

	XeColor white;
	white.lcol = 0xFFFFFFFF;

	video_osd_info_filename = new GuiText("info_filename", 22, white);
	video_osd_info_cur_time = new GuiText("info_cur_time", 18, white);
	video_osd_info_duration = new GuiText("info_duration", 18, blue);


	/** video infobar **/
	video_osd_infobar_text_filename = new GuiText("info_bar_filename", 18, white);
	video_osd_infobar_info_filename = new GuiText("video_osd_infobar_info_filename", 18, white);
	video_osd_infobar_text_trackinfo = new GuiText("info_bar_trackinfo", 18, white);
	video_osd_infobar_info_trackinfo = new GuiText("video_osd_infobar_info_trackinfo", 18, white);
	video_osd_infobar_text_resolution = new GuiText("info_bar_resolution", 18, white);
	video_osd_infobar_info_resolution = new GuiText("video_osd_infobar_info_resolution", 18, white);
	video_osd_infobar_text_bitrate = new GuiText("info_bar_bitrate", 18, white);
	video_osd_infobar_info_bitrate = new GuiText("video_osd_infobar_info_bitrate", 18, white);

	video_osd_infobar = new GuiWindow(1280, 720);
	video_osd_infobar->SetPosition(0, 0);
	video_osd_infobar->Append(video_osd_infobar_text_filename);
	video_osd_infobar->Append(video_osd_infobar_info_filename);
	video_osd_infobar->Append(video_osd_infobar_text_trackinfo);
	video_osd_infobar->Append(video_osd_infobar_info_trackinfo);
	video_osd_infobar->Append(video_osd_infobar_text_filename);
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


}

static void common_setup() {
	trigA = new GuiTrigger();

	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiImageData * background = new GuiImageData(welcome_background_bg_png);
	mainWindow = new GuiWindow(screenwidth, screenheight);
	bgImg = new GuiImage(background);
	mainWindow->Append(bgImg);

	loadRessources();
}

extern "C" double playerGetElapsed();
extern "C" double playerGetDuration();
extern "C" const char * playerGetFilename();
extern "C" int playerGetStatus();

static int osd_duration_bar_width;
static int osd_show = 0;
static char osd_duration[10];
static char osd_cur_time[10];
static int got_metadata = 0;

extern "C" void mplayer_osd_open() {
	got_metadata = 0;

	if (osd_show == 0) {
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
		//	
		video_osd_info_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_info_cur_time->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_info_duration->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

		int infobar_x, infobar_y;
		infobar_x = 140;
		infobar_y = 360;
		video_osd_infobar_text_filename->SetPosition(infobar_x, infobar_y);
		video_osd_infobar_info_filename->SetPosition(infobar_x + 210, infobar_y);

		video_osd_infobar_text_trackinfo->SetPosition(infobar_x, infobar_y + 20);
		video_osd_infobar_info_trackinfo->SetPosition(infobar_x + 210, infobar_y + 20);

		video_osd_infobar_text_resolution->SetPosition(infobar_x, infobar_y + 40);
		video_osd_infobar_info_resolution->SetPosition(infobar_x + 210, infobar_y + 40);

		video_osd_infobar_text_bitrate->SetPosition(infobar_x, infobar_y + 60);
		video_osd_infobar_info_bitrate->SetPosition(infobar_x + 210, infobar_y + 60);

		video_osd_infobar_text_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_infobar_info_filename->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

		video_osd_infobar_text_trackinfo->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_infobar_info_trackinfo->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

		video_osd_infobar_text_resolution->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_infobar_info_resolution->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

		video_osd_infobar_text_bitrate->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		video_osd_infobar_info_bitrate->SetAlignment(ALIGN_LEFT, ALIGN_TOP);

		/** video infobar **/
		video_osd_infobar_info_filename->SetText(playetGetMetaData(META_NAME));
		video_osd_infobar_info_trackinfo->SetText(playetGetMetaData(META_VIDEO_CODEC));
		video_osd_infobar_info_resolution->SetText(playetGetMetaData(META_VIDEO_RESOLUTION));
		video_osd_infobar_info_bitrate->SetText(playetGetMetaData(META_VIDEO_BITRATE));


		//<image x="202" y="613" w="40" h="40" image="@@speed_state"/>
		//<image image="@@play_state" x="202" y="613" w="40" h="40"/>

		video_osd_play->SetPosition(202, 613);
		video_osd_pause->SetPosition(202, 613);
		video_osd_stop->SetPosition(202, 613);
		video_osd_next->SetPosition(202, 613);
		video_osd_prev->SetPosition(202, 613);
		video_osd_rewind->SetPosition(202, 613);
		video_osd_forward->SetPosition(202, 613);

		video_osd_play->SetVisible(false);
		video_osd_pause->SetVisible(false);
		video_osd_stop->SetVisible(false);
		video_osd_next->SetVisible(false);
		video_osd_prev->SetVisible(false);
		video_osd_rewind->SetVisible(false);
		video_osd_forward->SetVisible(false);

		video_osd_infobar->SetVisible(false);

		// order
		mainWindow->Append(video_osd_bg);
		mainWindow->Append(video_osd_progress_bar_back);
		mainWindow->Append(video_osd_progress_bar_front);

		mainWindow->Append(video_osd_info_filename);
		mainWindow->Append(video_osd_info_cur_time);
		mainWindow->Append(video_osd_info_duration);

		//
		mainWindow->Append(video_osd_infobar_text_filename);
		mainWindow->Append(video_osd_infobar_info_filename);
		mainWindow->Append(video_osd_infobar_text_trackinfo);
		mainWindow->Append(video_osd_infobar_info_trackinfo);
		mainWindow->Append(video_osd_infobar_text_resolution);
		mainWindow->Append(video_osd_infobar_info_resolution);
		mainWindow->Append(video_osd_infobar_text_bitrate);
		mainWindow->Append(video_osd_infobar_info_bitrate);

		// 		
		mainWindow->Append(video_osd_play);
		mainWindow->Append(video_osd_pause);
		mainWindow->Append(video_osd_stop);
		mainWindow->Append(video_osd_next);
		mainWindow->Append(video_osd_prev);
		mainWindow->Append(video_osd_rewind);
		mainWindow->Append(video_osd_forward);

		// remove bg
		mainWindow->Remove(bgImg);

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		osd_duration_bar_width = img->width;
	}
	osd_show = 1;
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
		mainWindow->Remove(video_osd_infobar_text_filename);
		mainWindow->Remove(video_osd_infobar_info_filename);
		mainWindow->Remove(video_osd_infobar_text_trackinfo);
		mainWindow->Remove(video_osd_infobar_info_trackinfo);
		mainWindow->Remove(video_osd_infobar_text_resolution);
		mainWindow->Remove(video_osd_infobar_info_resolution);
		mainWindow->Remove(video_osd_infobar_text_bitrate);
		mainWindow->Remove(video_osd_infobar_info_bitrate);

		// reapply bg
		mainWindow->Append(bgImg);
	}
	osd_show = 0;
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

		video_osd_info_filename->SetText(playerGetFilename());

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

		if (level == 3) {
			// show file info
			video_osd_infobar->SetVisible(true);

		} else {
			video_osd_infobar->SetVisible(false);
		}		
	}
	
	UpdatePads();
	Menu_Frame();
	mainWindow->Draw();
	//Menu_Render();
	for (int i = 0; i < 4; i++) {
		mainWindow->Update(&userInput[i]);
	}
}

static void Browser(const char * title, const char * root) {
	BrowseDevice("", root);

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

	GuiFileBrowser gui_browser(980, 500, browser_selector, browser_folder_icon, browser_file_icon);



	gui_browser.SetPosition(150, 180);
	gui_browser.SetFontSize(20);
	gui_browser.SetSelectedFontSize(26);
	gui_browser.SetPageSize(10);

	//mainWindow->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
	mainWindow->Append(&gui_browser);

	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_B);

	GuiText menuBtnTxt("B", 18, white);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();

	mainWindow->Append(&menuBtn);


	GuiText browserTxt(title, 64, white);
	browserTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	browserTxt.SetPosition(64, 45);
	browserTxt.SetEffectGrow();

	mainWindow->Append(&browserTxt);

	last_menu = current_menu;

	while (current_menu == last_menu) {

		// update file browser based on arrow xenon_buttons
		// set MENU_EXIT if A xenon_button pressed on a file
		for (int i = 0; i < gui_browser.getPageSize(); i++) {
			if (gui_browser.fileList[i]->GetState() == STATE_CLICKED) {
				gui_browser.fileList[i]->ResetState();
				// check corresponding browser entry
				if (browserList[browser.selIndex].isdir) {
					if (BrowserChangeFolder()) {
						gui_browser.ResetState();
						gui_browser.fileList[0]->SetState(STATE_SELECTED);
						gui_browser.TriggerUpdate();
					} else {
						break;
					}
				} else {
					current_menu = MENU_MPLAYER;
					//mplayer_main(NULL,NULL);

					sprintf(mplayer_filename, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);

					CleanupPath(mplayer_filename);

					ShutoffRumble();
					gui_browser.ResetState();
				}
			}
		}

		if (menuBtn.GetState() == STATE_CLICKED) {
			current_menu = MENU_BACK;
		}

		update();
	}

	mainWindow->Remove(&browserTxt);
	mainWindow->Remove(&gui_browser);
	mainWindow->Remove(&menuBtn);
}

static void HomePage() {
	mainWindow->Append(home_left);
	mainWindow->Append(home_main_function_frame_bg);

	// <wgt_mlist name="VERT_MLIST" image="" x="305" y="140" w="200" h="440" fontsize="26" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="vert" itemcount="4" extra_index="3" extra_len="0"/>

	home_list_v->SetPosition(305, 140);
	home_list_v->SetCount(4);
	home_list_v->SetCenter(3);

	home_list_v->Append(home_video_btn);
	home_list_v->Append(home_music_btn);
	home_list_v->Append(home_photo_btn);
	home_list_v->Append(home_setting_btn);

	mainWindow->Append(home_list_v);

	// <wgt_mlist name="HORI_MLIST" x="560" y="360" w="465" h="110" fontsize="20" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="hori" itemcount="3"/>

	home_list_h->SetPosition(560, 360);
	home_list_h->SetCount(3);
	home_list_h->SetCenter(2);
	
	for(int i = 0;i<device_list_size;i++){		
		home_list_h->Append(home_device_btn[i]);
	}
	

	home_list_h->SetFocus(1);

	home_list_h->SetSelector(home_list_h_selector);

	mainWindow->Append(home_list_h);
	
	// <text text="@@curitem" x="550" y="610" w="650" h="40" align="right" fontsize="32" textcolor="0xffffff"/>
	home_curitem->SetPosition(550,610);
	
	//mainWindow->Append(home_curitem);


	GuiTrigger trigMenu;
	trigMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_A);

	GuiText menuBtnTxt("A", 18, white);
	GuiButton menuBtn(20, 20);
	menuBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	menuBtn.SetPosition(50, -35);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetTrigger(&trigMenu);
	menuBtn.SetEffectGrow();

	mainWindow->Append(&menuBtn);

	while (current_menu == HOME_PAGE) {
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
	
	for(int i = 0;i<device_list_size;i++){		
		home_list_h->Remove(home_device_btn[i]);
	}
	
	home_list_v->Remove(home_video_btn);
	home_list_v->Remove(home_music_btn);
	home_list_v->Remove(home_photo_btn);
	home_list_v->Remove(home_setting_btn);
	
	//mainWindow->Remove(home_curitem);
}

static void do_mplayer(char * filename) {
	static int mplayer_need_init = 1;
	if (mplayer_need_init) {
		char * argv[] = {
			"mplayer.xenon",
			//"-really-quiet",
			//"-demux mkv",
			"-menu",
			//"-menu-startup",
			//"-lavdopts","skiploopfilter=all:threads=2",
			"-lavdopts", "skiploopfilter=all:threads=5",
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

static void findDevices()
{	
	for(int i = 3;i<STD_MAX;i++){
		if(devoptab_list[i]->structSize){
			//strcpy(device_list[device_list_size],devoptab_list[i]->name);			
			sprintf(device_list[device_list_size],"%s:/",devoptab_list[i]->name);			
			printf("findDevices : %s\r\n",device_list[device_list_size]);			
			device_list_size++;
		}
	}
	
	root_dev = device_list[0];
}

extern "C" void init_xtaf(); 

int main(int argc, char** argv) {
	XeColor white;
	white.lcol = 0xFFFFFFFF;

	xenon_make_it_faster(XENON_SPEED_FULL);

	init_mplayer();

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
	fatInitDefault();
	// xtaf
	init_xtaf();
	
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
		TR;
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
