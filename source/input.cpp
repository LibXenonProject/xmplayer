/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * Modified by Ced2911, 2011
 *
 * input.cpp
 * Wii/GameCube controller management
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <input/input.h>
#include <usb/usbmain.h>
#include <xetypes.h>
#include <xenos/xe.h>
//#include "menu.h"
#include "video.h"
#include "input.h"
#include "libwiigui/gui.h"

void enableCapture();

#define MAX_INPUTS 4

int rumbleRequest[4] = {0, 0, 0, 0};
GuiTrigger userInput[4];
static int rumbleCount[4] = {0, 0, 0, 0};

static WPADData wpad_xenon[MAX_INPUTS];

#define	STICK_DEAD_ZONE (13107)
#define HANDLE_STICK_DEAD_ZONE(x) ((((x)>-STICK_DEAD_ZONE) && (x)<STICK_DEAD_ZONE)?0:(x-x/abs(x)*STICK_DEAD_ZONE))

void XenonInputInit() {
    PAD_Init();
    memset(wpad_xenon, 0, MAX_INPUTS * sizeof (WPADData));
}

#define PUSHED(x) ((ctrl[ictrl].x)&&(old_ctrl[ictrl].x==0))
#define RELEASED(x) ((ctrl[ictrl].x==0)&&(old_ctrl[ictrl].x==1))
#define HELD(x) ((ctrl[ictrl].x==1))

void XenonInputUpdate() {
    // update wpad
    for (int i = 0; i < MAX_INPUTS; i++) {
        wpad_xenon[i].btns_d = WPAD_ButtonsDown(i);
        wpad_xenon[i].btns_u = WPAD_ButtonsUp(i);
        wpad_xenon[i].btns_h = WPAD_ButtonsHeld(i);

        //float irx = (float)((float)PAD_StickX(i)/128.f);
        //float iry = (float)(-(float)PAD_StickY(i)/128.f)-0.5f;
        //        float iry = 0.5f-((float)PAD_StickY(i)/128.f);
        float iry = 0.5f + ((float) -PAD_StickY(i) / 128.f);
        float irx = 0.5f + ((float) PAD_StickX(i) / 128.f);

//        irx *= screenwidth;
//        iry *= screenheight;

        wpad_xenon[i].ir.x = irx;
        wpad_xenon[i].ir.y = iry;

        wpad_xenon[i].ir.valid = 0;
    }
}

/****************************************************************************
 * UpdatePads
 *
 * Scans pad and wpad
 ***************************************************************************/
void UpdatePads() {	
    PAD_Update();
	
    XenonInputUpdate();

    for (int i = 3; i >= 0; i--) {
        userInput[i].pad.btns_d = PAD_ButtonsDown(i);
        userInput[i].pad.btns_u = PAD_ButtonsUp(i);
        userInput[i].pad.btns_h = PAD_ButtonsHeld(i);
        //        userInput[i].pad.stickX = PAD_StickX(i);
        //        userInput[i].pad.stickY = PAD_StickY(i);
        userInput[i].pad.substickX = PAD_SubStickX(i);
        userInput[i].pad.substickY = PAD_SubStickY(i);
        userInput[i].pad.triggerL = PAD_TriggerL(i);
        userInput[i].pad.triggerR = PAD_TriggerR(i);
    }
}

/****************************************************************************
 * SetupPads
 *
 * Sets up userInput triggers for use
 ***************************************************************************/
void SetupPads() {
    XenonInputInit();


    for (int i = 0; i < 4; i++) {
        userInput[i].chan = i;
        userInput[i].wpad = &wpad_xenon[i];
        userInput[i].wpad->exp.type = EXP_CLASSIC;
    }
}

/****************************************************************************
 * ShutoffRumble
 ***************************************************************************/

void ShutoffRumble() {

}

/****************************************************************************
 * DoRumble
 ***************************************************************************/

void DoRumble(int i) {

}