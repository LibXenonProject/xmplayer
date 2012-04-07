/*
 * GyS-TermIO v2.0 (for GySmail v3)
 * a very small replacement of ncurses library
 *
 * copyright (C) 1999 A'rpi/ESP-team
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#define MAX_KEYS 64
#define BUF_LEN 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include <console/console.h>

#include <stdint.h>


#include <usb/usbmain.h>
#include <ppc/timebase.h>

#include "keycodes.h"

#include "mp_fifo.h"

#include <debug.h>


//#include <input/input.h>

struct controller_data_s
{
	signed short s1_x, s1_y, s2_x, s2_y;
	int s1_z, s2_z, lb, rb, start, select, a, b, x, y, up, down, left, right;
	unsigned char lt, rt;
	int logo;
};

int get_controller_data(struct controller_data_s *d, int port);
void set_controller_data(int port, const struct controller_data_s *d);
void set_controller_rumble(int port, uint8_t l, uint8_t r);


static int getch2_len=0;
static char getch2_buf[BUF_LEN];

int screen_width=80;
int screen_height=24;
char * erase_to_end_of_line = NULL;

typedef struct {
  int len;
  int code;
  char chars[8];
} keycode_st;
static keycode_st getch2_keys[MAX_KEYS];
static int getch2_key_db=0;


static int getch2_status=0;

void get_screen_size(void){
	console_get_dimensions(&screen_width,&screen_height);
}

#define MAX_INPUTS 4

static struct controller_data_s ctrl[MAX_INPUTS];
static struct controller_data_s old_ctrl[MAX_INPUTS];

#define PUSHED(x) ((ctrl[ictrl].x)&&(old_ctrl[ictrl].x==0))
#define RELEASED(x) ((ctrl[ictrl].x==0)&&(old_ctrl[ictrl].x==1))
#define HELD(x) ((ctrl[ictrl].x==1))

void get_controller_down(struct controller_data_s * pCtrl, int ictrl){
	memset(&ctrl[ictrl],0,sizeof(struct controller_data_s));
	memset(pCtrl,0,sizeof(struct controller_data_s));
	
	get_controller_data(&ctrl,ictrl);
	
	if(PUSHED(down))
		pCtrl->down=1;
	if(PUSHED(up))
		pCtrl->up=1;
	if(PUSHED(left))
		pCtrl->left=1;
	if(PUSHED(right))
		pCtrl->right=1;
	if(PUSHED(a))
		pCtrl->a=1;
	if(PUSHED(b))
		pCtrl->b=1;
	if(PUSHED(x))
		pCtrl->x=1;	
	if(PUSHED(y))
		pCtrl->y=1;
	if(PUSHED(start))
		pCtrl->x=1;	
	if(PUSHED(select))
		pCtrl->y=1;
	if(PUSHED(logo))
		pCtrl->y=1;
	
	old_ctrl[ictrl] = ctrl[ictrl];
}

static int getch2_internal(void)
{
	int i = 0;
	int max_ctrl=4;
	
	int key = -1;
	static s64 lasttime = 0;
	s64 curtime = mftb();

	usb_do_poll();
	
	if (getch2_status && tb_diff_msec(curtime,lasttime)>60 )		
	{
		
		struct controller_data_s ctrl;
		
		get_controller_down(&ctrl,i);
		
		if(ctrl.left)
			key = KEY_LEFT;		
		if(ctrl.right)
			key = KEY_RIGHT;	
		if(ctrl.up)
			key = KEY_UP;		
		if(ctrl.down)
			key = KEY_DOWN;
	
		
		if(ctrl.start)
			key = KEY_ENTER;
		if(ctrl.select)
			key = KEY_ESC;	
		if(ctrl.logo)
			key = 'o';
		
		if(ctrl.a)
			key = KEY_PLAYPAUSE;		
		if(ctrl.b)
			key = KEY_STOP;		
		if(ctrl.x)
			key = KEY_PAUSE;
		if(ctrl.y)
			key = KEY_MENU;
		
		if(ctrl.rb)
			key = KEY_NEXT;
		if(ctrl.lb)
			key = KEY_PREV;
	
		lasttime = curtime;
	}
	
	return key;
}

void getch2(void)
{
    int r = getch2_internal();
    if (r >= 0)
		mplayer_put_key(r);
}

void getch2_enable(void){
	TR;
    getch2_status=1;
}

void getch2_disable(void){
	TR;
    if(!getch2_status) return; // already disabled / never enabled
    getch2_status=0;
}
