/* 
 * File:   newmain.c
 * Author: cc
 *
 * Created on 27 avril 2012, 19:58
 */

#include <stdio.h>
#include <stdlib.h>

#include "Page.h"
#include "input.h"

#include "../build/home_left_png.h"
#include "../build/home_main_function_frame_bg_png.h"
#include "../build/welcome_background_bg_png.h"
#include "../build/font_ttf.h"




int main(int argc, char** argv) {
	XeColor white;
	white.lcol=0xFFFFFFFF;
	
	// Init Video
	InitVideo();
	
	// Init Freetype
	InitFreeType((u8*)font_ttf,font_ttf_size);
	
	// usb
	XenonInputInit();
	
	ChangeFontSize(26);
	
	Page home_page;	
	menu_image bg, home_frame, home_left;
	
//<wgt_mlist name="VERT_MLIST" image="" x="305" y="140" w="200" h="440" fontsize="26" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="vert" itemcount="4" extra_index="3" extra_len="0"/>
//<wgt_mlist name="HORI_MLIST" x="560" y="360" w="465" h="110" fontsize="20" duration="250" motion_type="decrease" textcolor="0xffffff" align="hcenter" direction="hori" itemcount="3"/>
	List home_v, home_h;
	home_v.setToPage(&home_page);
	home_v.setOrentation('V');
	home_v.setColor(white);
	// home_v.setDimension(305,140,200,440);
	home_v.setDimension(305,140,200,550);
	home_v.setFontSize(26);
	home_v.setItemCount(5);
	//home_v.setStyle(FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	home_v.setStyle(FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE);
	home_v.addText(charToWideChar("Video"));
	home_v.addText(charToWideChar("Audio"));
	home_v.addText(charToWideChar("Settings"));
	home_v.addText(charToWideChar("About"));
	home_v.addText(charToWideChar("About1"));
	home_v.addText(charToWideChar("About2"));
	home_v.addText(charToWideChar("About3"));
	home_v.addText(charToWideChar("About4"));
	
	
	home_h.setToPage(&home_page);
	home_h.setOrentation('H');
	home_h.setColor(white);
	home_h.setDimension(560,360,465,110);
	home_h.setFontSize(20);
	home_h.setItemCount(3);
	home_v.setStyle(FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	home_h.addText(charToWideChar("Usb"));
	home_h.addText(charToWideChar("Dvd"));
	home_h.addText(charToWideChar("Network"));
	home_h.addText(charToWideChar("HardDrive"));
	
//	// horizontal
//	menu_text video,audio,settings;
//	
//	// vertical
//	menu_text video_usb;
//	menu_text audio_usb;
//	menu_text setting_usb;
//	menu_text setting_video;
//	menu_text setting_other;

	//<image image="image/home_left.png" x="0" y="0" w="522" h="720" bg="1"/>
	//<image image="image/home_main_function_frame_bg.png" x="0" y="341" w="1280" h="148" bg="1"/>

	// load png
	load_element(
			&bg,
			welcome_background_bg_png,
			0, 0, 1280, 720
	);

	load_element(
			&home_frame,
			home_main_function_frame_bg_png, 0, 341, 1280, 148
			);
	load_element(
			&home_left,
			home_left_png, 0, 0, 522, 720
			);
	
	home_page.addImage(&bg);
	home_page.addImage(&home_frame);
	home_page.addImage(&home_left);

	struct XenosSurface * fb = Xe_GetFramebufferSurface(g_pVideoDevice);

	while (1) {
		XenonInputUpdate();
		
		int btn = XenonButtonsDown(0);
		if(btn&PAD_BUTTON_DOWN){
			int pos =	home_v.getPosition();
			if(pos>0)
				home_v.setPosition(pos-1);
		}
		if(btn&PAD_BUTTON_UP){
			int pos =	home_v.getPosition();
			if(pos<home_v.getSize()-1)
				home_v.setPosition(pos+1);
		}
		if(btn&PAD_BUTTON_RIGHT){
			int pos =	home_h.getPosition();
			if(pos>0)
				home_h.setPosition(pos-1);
		}
		if(btn&PAD_BUTTON_LEFT){
			int pos =	home_h.getPosition();
			if(pos<home_h.getSize()-1)
				home_h.setPosition(pos+1);
		}
		
		home_page.draw();
	}

	return (EXIT_SUCCESS);
}

