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


//**************************************************************************
// D-Pad direction for OSD Settings
//**************************************************************************
extern int osd_level;

//**************************************************************************
// Gui
//**************************************************************************
extern GuiImage * bgImg;
extern GuiWindow * mainWindow;
extern GuiTrigger * trigA;

// video osd
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

// Osd option menu
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

//osd subtitle options
static GuiWindow * osd_options_subtitle_window = NULL;
static GuiOptionBrowserOsd * osd_options_subtitle = NULL;
static OptionList subtitle_option_list;
//osd audio options
static GuiWindow * osd_options_audio_window = NULL;
static GuiOptionBrowserOsd * osd_options_audio = NULL;
static OptionList audio_option_list;
//osd video options
static GuiWindow * osd_options_video_window = NULL;
static GuiOptionBrowserOsd * osd_options_video = NULL;
static OptionList video_option_list;

static GuiText * osd_options_headline = NULL;

//**************************************************************************
//	osd settings
//**************************************************************************
static int osd_display_info = 0;
static int osd_display_option_audio = 0;
static int osd_display_option_video = 0;
static int osd_display_option_subtitle = 0;

static int osd_duration_bar_width;
static int osd_show = 0;
static char osd_duration[10];
static char osd_cur_time[10];
static int got_metadata = 0;

static int last_level = 0;
static char tmpbuff[1024];

static int first_enter;
/**
 * Reset controller
 **/
static void ResetController()
{
	int i = 0;
	for (i = 0; i < 4; i++) {
		struct controller_data_s ctrl_zero = {};
		set_controller_data(i, &ctrl_zero);
	}
}

/**
 * Callback for osd option bar
 **/
static void osd_option_default_callback(void * data)
{
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_pan_callback(void * data)
{
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		osd_display_option_video = !osd_display_option_video;
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_vsync_callback(void * data)
{
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		mplayer_switch_vsync();
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_next_callback(void * data)
{
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		button->ResetState();
		//playerTurnOffSubtitle(); //turns off subs, atm if not, subs will be freezed on a video without them
		mplayer_gui_asked(); //playback resume
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_loop_callback(void * data)
{
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_info_callback(void * data)
{
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		osd_display_info = !osd_display_info;
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_audio_callback(void * data)
{
	GuiButton *button = (GuiButton *) data;
	if (button->GetState() == STATE_CLICKED) {
		osd_display_option_audio = !osd_display_option_audio;
		button->ResetState();
		button->SetState(STATE_SELECTED);
	}
}

static void osd_options_sub_callback(void * data)
{
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
void LoadOsdRessources()
{ // OSD
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
	osd_options_subtitle = new GuiOptionBrowserOsd(400, 275, new GuiImageData(osd_options_browser_btn_png), &subtitle_option_list);
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
	osd_options_audio = new GuiOptionBrowserOsd(400, 275, new GuiImageData(osd_options_browser_btn_png), &audio_option_list);
	osd_options_audio->SetPosition(0, 40);
	osd_options_audio->SetCol2Position(150);
	osd_options_audio_window->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	osd_options_audio_window->Append(new GuiImage(new GuiImageData(osd_options_browser_bg_png)));
	osd_options_audio_window->Append(osd_options_audio);
	
	/**
	 * Osd Video window
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
	osd_options_video = new GuiOptionBrowserOsd(400, 275, new GuiImageData(osd_options_browser_btn_png), &video_option_list);
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

extern "C" void mplayer_osd_open()
{
	got_metadata = 0;

	if (osd_show == 0) {
		/** video infobar **/
		sprintf(tmpbuff, "%s %s", _("Filename:"), mplayet_get_meta_data(META_NAME));
		video_osd_infobar_text_filename->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Resolution:"), mplayet_get_meta_data(META_VIDEO_RESOLUTION));
		video_osd_infobar_text_resolution->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Video Codec:"), mplayet_get_meta_data(META_VIDEO_CODEC));
		video_osd_infobar_text_trackinfo->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Video Bitrate:"), mplayet_get_meta_data(META_VIDEO_BITRATE));
		video_osd_infobar_text_bitrate->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Audio Codec:"), mplayet_get_meta_data(META_AUDIO_CODEC));
		video_osd_infobar_text_a_codec->SetText(tmpbuff);

		sprintf(tmpbuff, "%s %s", _("Audio Bitrate:"), mplayet_get_meta_data(META_AUDIO_BITRATE));
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
	first_enter = 1;
}

static void OsdSubtitlesOptions()
{
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
				case 0:
				{
					mplayer_switch_subtitle();
					break;
				}
				case 1:
				{
					sub_visibility = (sub_visibility == 1) ? 0 : 1;
					break;
				}
				case 5:
				{
					osd_display_option_subtitle = 0;
					osd_options_menu_subtitle_btn->SetState(STATE_SELECTED);
					break;
				}
			}
			int het = osd_options_subtitle->GetSelectedOption();
			if (osd_options_subtitle->GetLeft() == 1) {
				switch (het) {
				case 2:
				{
					if (!ass_enabled) {
						sub_pos--;
						if (sub_pos < 0) {
							sub_pos = 100;
						}
					}
					break;
				}
				case 3:
				{
					sub_delay += 0.1;
					break;
				}
				case 4:
				{
					if (ass_enabled) {
						ass_font_scale -= 0.1;
						if (ass_font_scale < 0) {
							ass_font_scale = 100;
						}
						ass_force_reload = 1;
					} else {
						text_font_scale_factor -= 0.1;
						if (text_font_scale_factor < 0) {
							text_font_scale_factor = 100;
						}
						force_load_font = 1;
					}
					break;
				}
				}
			} else if (osd_options_subtitle->GetRight() == 1) {
				switch (het) {
				case 2:
				{
					if (!ass_enabled) {
						sub_pos++;
						if (sub_pos > 100) {
							sub_pos = 0;
						}
					}
					break;
				}
				case 3:
				{
					sub_delay -= 0.1;
					break;
				}
				case 4:
				{
					if (ass_enabled) {
						ass_font_scale += 0.1;
						if (ass_font_scale > 100) {
							ass_font_scale = 0;
						}
						ass_force_reload = 1;
					} else {
						text_font_scale_factor += 0.1;
						if (text_font_scale_factor > 100) {
							text_font_scale_factor = 0;
						}
						force_load_font = 1;
					}
					break;
				}
				}
			}
			if (ret >= 0 || firstRun) {
				firstRun = false;
				osd_sub_name = mplayer_get_subtitle();
				osd_subdelay = (sub_delay * -1000);
				if ((osd_subdelay < 0.100) && (osd_subdelay > -0.100)) {
					osd_subdelay = 0;
				}
				sprintf(subtitle_option_list.value[0], osd_sub_name);
				sprintf(subtitle_option_list.value[1], "%s", sub_visibility == 1 ? "Enabled" : "Disabled");
				if (!ass_enabled) {
					sprintf(subtitle_option_list.value[2], "%d", sub_pos);
				} else {
					sprintf(subtitle_option_list.value[2], "%s", "Disabled");
				}
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

static void OsdAudioOptions()
{
	bool firstRun = true;
	char *osd_mute = "";
	char *osd_balance = "";
	float osd_audiodelay, osd_volume;
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
				case 0:
				{
					mplayer_switch_audio();
					break;
				}
				case 3:
				{
					mplayer_switch_mute();
					break;
				}
				case 5:
				{
					osd_display_option_audio = 0;
					osd_options_menu_audio_channel_btn->SetState(STATE_SELECTED);
					break;
				}
			}
			int het = osd_options_audio->GetSelectedOption();
			if (osd_options_audio->GetLeft() == 1) {
				switch (het) {
				case 1:
				{
					mplayer_switch_volume(0);
					break;
				}
				/*case 2:
				{
					mplayer_switch_balance(1);
					break;
				} */
				case 4:
				{
					audio_delay += 0.1;
					break;
				}
				}
			} else if (osd_options_audio->GetRight() == 1) {
				switch (het) {
				case 1:
				{
					mplayer_switch_volume(1);
					break;
				}
				/*case 2:
				{
					mplayer_switch_balance(0);
					break;
				}*/
				case 4:
				{
					audio_delay -= 0.1;
					break;
				}
				}
			}
			if (ret >= 0 || firstRun) {
				firstRun = false;
				osd_volume = mplayer_get_volume();
				//osd_balance = mplayer_get_balance();
				osd_mute = mplayer_get_mute();
				osd_audiodelay = (audio_delay * -1000);
				if ((osd_audiodelay < 0.100) && (osd_audiodelay > -0.100)) {
					osd_audiodelay = 0;
				}
				sprintf(audio_option_list.value[1], "%.0f", osd_volume);
				sprintf(audio_option_list.value[2], "Disabled"); //balance is not working
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

static void OsdVideoOptions()
{
	bool firstRun = true;
	char osd_framedrop[100] = {};
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
				case 0:
				{
					mplayer_switch_fullscreen();
					break;
				}
				case 1:
				{
					frame_dropping++;
					if (frame_dropping > 2) {
						frame_dropping = 0;
					}
					break;
				}
				case 2:
				{
					vo_vsync = !vo_vsync;
					break;
				}
				case 3:
				{
					osd_display_option_video = 0;
					osd_options_menu_pan_btn->SetState(STATE_SELECTED);
					break;
				}
			}
			if (ret >= 0 || firstRun) {
				firstRun = false;
				if (frame_dropping == 2) {
					strcpy(osd_framedrop, "Hard");
				} else if (frame_dropping == 1) {
					strcpy(osd_framedrop, "Enabled");
				} else {
					strcpy(osd_framedrop, "Disabled");
				}
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

extern "C" void mplayer_osd_close()
{
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

extern "C" void mplayer_osd_draw(int level)
{
	if ((mplayer_get_pause() == 1) && (first_enter == 1)) {
		ResetController();
		first_enter = 0;
	}	
	//Y-osd button used because libmenu is off
	GuiTrigger osdMenu;
	osdMenu.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_Y);
	GuiButton osdBtn(20, 20);
	osdBtn.SetTrigger(&osdMenu);
	osdBtn.SetSelectable(false);
	mainWindow->Append(&osdBtn);
	if (osd_show) {

		double duration = mplayer_get_duration();
		double elapsed = mplayer_get_elapsed();

		struct XenosSurface * img = video_osd_progress_bar_front->GetImage();
		float pourcents = (float) (elapsed * 100) / (float) duration;
		float width = (float) osd_duration_bar_width * (pourcents / 100.0);
		img->width = width;

		format_time(osd_duration, duration);
		format_time(osd_cur_time, elapsed);

		video_osd_info_cur_time->SetText(osd_cur_time);
		video_osd_info_duration->SetText(osd_duration);

		if (last_level != level) {
			video_osd_info_filename->SetText(mplayer_get_filename());
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

		switch (mplayer_get_status()) {
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
	OsdSubtitlesOptions();
	OsdAudioOptions();
	OsdVideoOptions();
	Menu_Frame();
	last_level = level;
	mainWindow->Draw();
	if (level == 3) { //this fixes non intended key-presses when osd is not 3 (like when seekbar is present)
		UpdatePads();
		for (int i = 0; i < 4; i++) {
			mainWindow->Update(&userInput[i]);
		}
		if ((mplayer_get_pause() == 1) && (osdBtn.GetState() == STATE_CLICKED)) {
			osd_level = 1;
			osd_display_info = 0;
			osd_display_option_subtitle = 0;
			osd_display_option_audio = 0;
			osd_display_option_video = 0;

		}
	}
	mainWindow->Remove(&osdBtn);
}

