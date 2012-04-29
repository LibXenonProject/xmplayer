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
#include "menu.h"
#include "video.h"
#include "input.h"

#define MAX_INPUTS 4

static WPADData wpad_xenon[MAX_INPUTS];

static struct controller_data_s ctrl[MAX_INPUTS];
static struct controller_data_s old_ctrl[MAX_INPUTS];

#define	STICK_DEAD_ZONE (13107)
#define HANDLE_STICK_DEAD_ZONE(x) ((((x)>-STICK_DEAD_ZONE) && (x)<STICK_DEAD_ZONE)?0:(x-x/abs(x)*STICK_DEAD_ZONE))

void XenonInputInit() {
	usb_init();
	
    memset(ctrl, 0, MAX_INPUTS * sizeof (struct controller_data_s));
    memset(old_ctrl, 0, MAX_INPUTS * sizeof (struct controller_data_s));

    memset(wpad_xenon, 0, MAX_INPUTS * sizeof (WPADData));
}

#define PUSHED(x) ((ctrl[ictrl].x)&&(old_ctrl[ictrl].x==0))
#define RELEASED(x) ((ctrl[ictrl].x==0)&&(old_ctrl[ictrl].x==1))
#define HELD(x) ((ctrl[ictrl].x==1))

uint16_t XenonButtonsDown(int ictrl) {
    uint16_t btn = 0;

    if (PUSHED(a)) {
        btn |= PAD_BUTTON_A;
    }

    if (PUSHED(b)) {
        btn |= PAD_BUTTON_B;
    }

    if (PUSHED(x)) {
        btn |= PAD_BUTTON_X;
    }

    if (PUSHED(y)) {
        btn |= PAD_BUTTON_Y;
    }

    if (PUSHED(up)) {
        btn |= PAD_BUTTON_UP;
    }

    if (PUSHED(down)) {
        btn |= PAD_BUTTON_DOWN;
    }

    if (PUSHED(left)) {
        btn |= PAD_BUTTON_LEFT;
    }

    if (PUSHED(right)) {
        btn |= PAD_BUTTON_RIGHT;
    }

    if (PUSHED(start)) {
        btn |= PAD_BUTTON_START;
    }

    if (PUSHED(select)) {
        btn |= PAD_BUTTON_BACK;
    }

    if (PUSHED(logo)) {
        btn |= PAD_BUTTON_LOGO;
//        enableCapture();
    }

    if (PUSHED(rb)) {
        btn |= PAD_BUTTON_RB;
    }

    if (PUSHED(lb)) {
        btn |= PAD_BUTTON_LB;
    }

    if (PUSHED(s1_z)) {
        btn |= PAD_BUTTON_RSTICK;
    }

    if (PUSHED(s2_z)) {
        btn |= PAD_BUTTON_LSTICK;
    }
    return btn;
}

uint16_t XenonButtonsUp(int ictrl) {
    uint16_t btn = 0;

    if (RELEASED(a)) {
        btn |= PAD_BUTTON_A;
    }

    if (RELEASED(b)) {
        btn |= PAD_BUTTON_B;
    }

    if (RELEASED(x)) {
        btn |= PAD_BUTTON_X;
    }

    if (RELEASED(y)) {
        btn |= PAD_BUTTON_Y;
    }

    if (RELEASED(up)) {
        btn |= PAD_BUTTON_UP;
    }

    if (RELEASED(down)) {
        btn |= PAD_BUTTON_DOWN;
    }

    if (RELEASED(left)) {
        btn |= PAD_BUTTON_LEFT;
    }

    if (RELEASED(right)) {
        btn |= PAD_BUTTON_RIGHT;
    }

    if (RELEASED(start)) {
        btn |= PAD_BUTTON_START;
    }

    if (RELEASED(select)) {
        btn |= PAD_BUTTON_BACK;
    }

    if (RELEASED(logo)) {
        btn |= PAD_BUTTON_LOGO;
    }

    if (RELEASED(rb)) {
        btn |= PAD_BUTTON_RB;
    }

    if (RELEASED(lb)) {
        btn |= PAD_BUTTON_LB;
    }

    if (RELEASED(s1_z)) {
        btn |= PAD_BUTTON_RSTICK;
    }

    if (RELEASED(s2_z)) {
        btn |= PAD_BUTTON_LSTICK;
    }

    return btn;
}

uint16_t XenonButtonsHeld(int ictrl) {
    uint16_t btn = 0;

    if (HELD(a)) {
        btn |= PAD_BUTTON_A;
    }

    if (HELD(b)) {
        btn |= PAD_BUTTON_B;
    }

    if (HELD(x)) {
        btn |= PAD_BUTTON_X;
    }

    if (HELD(y)) {
        btn |= PAD_BUTTON_Y;
    }

    if (HELD(up)) {
        btn |= PAD_BUTTON_UP;
    }

    if (HELD(down)) {
        btn |= PAD_BUTTON_DOWN;
    }

    if (HELD(left)) {
        btn |= PAD_BUTTON_LEFT;
    }

    if (HELD(right)) {
        btn |= PAD_BUTTON_RIGHT;
    }

    if (HELD(start)) {
        btn |= PAD_BUTTON_START;
    }

    if (HELD(select)) {
        btn |= PAD_BUTTON_BACK;
    }

    if (HELD(logo)) {
        btn |= PAD_BUTTON_LOGO;
    }

    if (HELD(rb)) {
        btn |= PAD_BUTTON_RB;
    }

    if (HELD(lb)) {
        btn |= PAD_BUTTON_LB;
    }

    if (HELD(s1_z)) {
        btn |= PAD_BUTTON_RSTICK;
    }

    if (HELD(s2_z)) {
        btn |= PAD_BUTTON_LSTICK;
    }

    return btn;
}

s8 XenonStickX(int i) {
    return HANDLE_STICK_DEAD_ZONE(ctrl[i].s1_x) >> 8;
}

s8 XenonStickY(int i) {
    return HANDLE_STICK_DEAD_ZONE(ctrl[i].s1_y) >> 8;
}

s8 XenonSubStickX(int i) {
    return HANDLE_STICK_DEAD_ZONE(ctrl[i].s2_y) >> 8;
}

s8 XenonSubStickY(int i) {
    return HANDLE_STICK_DEAD_ZONE(ctrl[i].s2_y) >> 8;
}

u8 XenonTriggerL(int i) {
    return ctrl[i].lt;
}

u8 XenonTriggerR(int i) {
    return ctrl[i].rt;
}

void XenonInputUpdate() {
	int i = 0;
    for (i = 0; i < MAX_INPUTS; i++) {
        old_ctrl[i] = ctrl[i];
        usb_do_poll();
        get_controller_data(&ctrl[i], i);
    }
    // update wpad
    for (i = 0; i < MAX_INPUTS; i++) {
        wpad_xenon[i].btns_d = XenonButtonsDown(i);
        wpad_xenon[i].btns_u = XenonButtonsUp(i);
        wpad_xenon[i].btns_h = XenonButtonsHeld(i);

        //float irx = (float)((float)PAD_StickX(i)/128.f);
        //float iry = (float)(-(float)PAD_StickY(i)/128.f)-0.5f;
        //        float iry = 0.5f-((float)PAD_StickY(i)/128.f);
        float iry = 0.5f + ((float) -XenonStickY(i) / 128.f);
        float irx = 0.5f + ((float) XenonStickX(i) / 128.f);

        irx *= screenwidth;
        iry *= screenheight;

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
    XenonInputUpdate();
	int i = 0;
    for (i = 3; i >= 0; i--) {
        userInput[i].pad.btns_d = XenonButtonsDown(i);
        userInput[i].pad.btns_u = XenonButtonsUp(i);
        userInput[i].pad.btns_h = XenonButtonsHeld(i);
//        //        userInput[i].pad.stickX = PAD_StickX(i);
//        //        userInput[i].pad.stickY = PAD_StickY(i);
        userInput[i].pad.substickX = XenonSubStickX(i);
        userInput[i].pad.substickY = XenonSubStickY(i);
        userInput[i].pad.triggerL = XenonTriggerL(i);
        userInput[i].pad.triggerR = XenonTriggerR(i);
    }
}

/****************************************************************************
 * SetupPads
 *
 * Sets up userInput triggers for use
 ***************************************************************************/
void SetupPads() {
    XenonInputInit();

	int i = 0;

    for (i = 0; i < 4; i++) {
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
