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
#include <time/time.h>
#include <ppc/timebase.h>
//#include "menu.h"
#include "video.h"
#include "input.h"

void doScreenCapture();

#define MAX_INPUTS 4

static struct controller_data_s ctrl[MAX_INPUTS];
static struct controller_data_s old_ctrl[MAX_INPUTS];

#define	STICK_DEAD_ZONE (13107)
#define HANDLE_STICK_DEAD_ZONE(x) ((((x)>-STICK_DEAD_ZONE) && (x)<STICK_DEAD_ZONE)?0:(x-x/abs(x)*STICK_DEAD_ZONE))

#define PUSHED(x) ((ctrl[ictrl].x)&&(old_ctrl[ictrl].x==0))
#define RELEASED(x) ((ctrl[ictrl].x==0)&&(old_ctrl[ictrl].x==1))
#define HELD(x) ((ctrl[ictrl].x==1))

uint16_t WPAD_ButtonsDown(int ictrl) {
	uint16_t btn = 0;

	if (PUSHED(a)) {
		btn |= WPAD_CLASSIC_BUTTON_A;
	}

	if (PUSHED(b)) {
		btn |= WPAD_CLASSIC_BUTTON_B;
	}

	if (PUSHED(x)) {
		btn |= WPAD_CLASSIC_BUTTON_X;
	}

	if (PUSHED(y)) {
		btn |= WPAD_CLASSIC_BUTTON_Y;
	}

	if (PUSHED(up)) {
		btn |= WPAD_CLASSIC_BUTTON_UP;
	}

	if (PUSHED(down)) {
		btn |= WPAD_CLASSIC_BUTTON_DOWN;
	}

	if (PUSHED(left)) {
		btn |= WPAD_CLASSIC_BUTTON_LEFT;
	}

	if (PUSHED(right)) {
		btn |= WPAD_CLASSIC_BUTTON_RIGHT;
	}

	//    if (PUSHED(start)) {
	//        btn |= WPAD_CLASSIC_BUTTON_START;
	//    }
	//
	//    if (PUSHED(select)) {
	//        btn |= WPAD_CLASSIC_BUTTON_BACK;
	//    }
	//
	//    if (PUSHED(logo)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LOGO;
	//        enableCapture();
	//    }
	//
	//    if (PUSHED(rb)) {
	//        btn |= WPAD_CLASSIC_BUTTON_RB;
	//    }
	//
	//    if (PUSHED(lb)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LB;
	//    }
	//
	//    if (PUSHED(s1_z)) {
	//        btn |= WPAD_CLASSIC_BUTTON_RSTICK;
	//    }
	//
	//    if (PUSHED(s2_z)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LSTICK;
	//    }
	return btn;
}

uint16_t WPAD_ButtonsUp(int ictrl) {
	uint16_t btn = 0;

	if (RELEASED(a)) {
		btn |= WPAD_CLASSIC_BUTTON_A;
	}

	if (RELEASED(b)) {
		btn |= WPAD_CLASSIC_BUTTON_B;
	}

	if (RELEASED(x)) {
		btn |= WPAD_CLASSIC_BUTTON_X;
	}

	if (RELEASED(y)) {
		btn |= WPAD_CLASSIC_BUTTON_Y;
	}

	if (RELEASED(up)) {
		btn |= WPAD_CLASSIC_BUTTON_UP;
	}

	if (RELEASED(down)) {
		btn |= WPAD_CLASSIC_BUTTON_DOWN;
	}

	if (RELEASED(left)) {
		btn |= WPAD_CLASSIC_BUTTON_LEFT;
	}

	if (RELEASED(right)) {
		btn |= WPAD_CLASSIC_BUTTON_RIGHT;
	}

	//    if (RELEASED(start)) {
	//        btn |= WPAD_CLASSIC_BUTTON_START;
	//    }
	//
	//    if (RELEASED(select)) {
	//        btn |= WPAD_CLASSIC_BUTTON_BACK;
	//    }
	//
	//    if (RELEASED(logo)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LOGO;
	//    }
	//
	//    if (RELEASED(rb)) {
	//        btn |= WPAD_CLASSIC_BUTTON_RB;
	//    }
	//
	//    if (RELEASED(lb)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LB;
	//    }
	//
	//    if (RELEASED(s1_z)) {
	//        btn |= WPAD_CLASSIC_BUTTON_RSTICK;
	//    }
	//
	//    if (RELEASED(s2_z)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LSTICK;
	//    }

	return btn;
}

uint16_t WPAD_ButtonsHeld(int ictrl) {
	uint16_t btn = 0;

	if (HELD(a)) {
		btn |= WPAD_CLASSIC_BUTTON_A;
	}

	if (HELD(b)) {
		btn |= WPAD_CLASSIC_BUTTON_B;
	}

	if (HELD(x)) {
		btn |= WPAD_CLASSIC_BUTTON_X;
	}

	if (HELD(y)) {
		btn |= WPAD_CLASSIC_BUTTON_Y;
	}

	if (HELD(up)) {
		btn |= WPAD_CLASSIC_BUTTON_UP;
	}

	if (HELD(down)) {
		btn |= WPAD_CLASSIC_BUTTON_DOWN;
	}

	if (HELD(left)) {
		btn |= WPAD_CLASSIC_BUTTON_LEFT;
	}

	if (HELD(right)) {
		btn |= WPAD_CLASSIC_BUTTON_RIGHT;
	}
	//
	//    if (HELD(start)) {
	//        btn |= WPAD_CLASSIC_BUTTON_START;
	//    }
	//
	//    if (HELD(select)) {
	//        btn |= WPAD_CLASSIC_BUTTON_BACK;
	//    }
	//
	//    if (HELD(logo)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LOGO;
	//    }
	//
	//    if (HELD(rb)) {
	//        btn |= WPAD_CLASSIC_BUTTON_RB;
	//    }
	//
	//    if (HELD(lb)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LB;
	//    }
	//
	//    if (HELD(s1_z)) {
	//        btn |= WPAD_CLASSIC_BUTTON_RSTICK;
	//    }
	//
	//    if (HELD(s2_z)) {
	//        btn |= WPAD_CLASSIC_BUTTON_LSTICK;
	//    }

	return btn;
}

uint16_t PAD_ButtonsDown(int ictrl) {
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

uint16_t PAD_ButtonsUp(int ictrl) {
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

uint16_t PAD_ButtonsHeld(int ictrl) {
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

	if (HELD(logo) && HELD(select) && HELD(start))
		exit(0);

	return btn;
}

s8 PAD_StickX(int i) {
	return HANDLE_STICK_DEAD_ZONE(ctrl[i].s1_x) >> 8;
}

s8 PAD_StickY(int i) {
	return HANDLE_STICK_DEAD_ZONE(ctrl[i].s1_y) >> 8;
}

s8 PAD_SubStickX(int i) {
	return HANDLE_STICK_DEAD_ZONE(ctrl[i].s2_y) >> 8;
}

s8 PAD_SubStickY(int i) {
	return HANDLE_STICK_DEAD_ZONE(ctrl[i].s2_y) >> 8;
}

u8 PAD_TriggerL(int i) {
	return ctrl[i].lt;
}

u8 PAD_TriggerR(int i) {
	return ctrl[i].rt;
}

void PAD_Init() {
	memset(ctrl, 0, MAX_INPUTS * sizeof (struct controller_data_s));
	memset(old_ctrl, 0, MAX_INPUTS * sizeof (struct controller_data_s));
}



void PAD_Update() {
	usb_do_poll();
	
	static s64 lasttime = 0;
	s64 curtime = mftb();

	int i = 0;
	for (i = 0; i < MAX_INPUTS; i++) {
		old_ctrl[i] = ctrl[i];
		get_controller_data(&ctrl[i], i);
		
		if(tb_diff_msec(curtime, lasttime) > 60){
			if(ctrl[i].select && ctrl[i].logo){
				doScreenCapture();
				lasttime = curtime;
			}
		}
	}
}
