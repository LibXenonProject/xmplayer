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

#include <stdint.h>

#include <usb/usbmain.h>
#include <ppc/timebase.h>

#include <xetypes.h>

#include "keycodes.h"

#include "mp_fifo.h"

#include "getch2.h"

#include <debug.h>


//#include <input/input.h>

struct controller_data_s {
	signed short s1_x, s1_y, s2_x, s2_y;
	int s1_z, s2_z, lb, rb, start, select, a, b, x, y, up, down, left, right;
	unsigned char lt, rt;
	int logo;
};

int get_controller_data(struct controller_data_s *d, int port);
void set_controller_data(int port, const struct controller_data_s *d);
void set_controller_rumble(int port, uint8_t l, uint8_t r);


int screen_width = 80;
int screen_height = 24;
char * erase_to_end_of_line = NULL;

static int getch2_status = 0;
int gui_input_use = 0;


void get_screen_size(void) {
	
}

#define MAX_INPUTS 4

static struct controller_data_s ctrl[MAX_INPUTS];
static struct controller_data_s old_ctrl[MAX_INPUTS];

#define PUSHED(x) ((ctrl[ictrl].x)&&(old_ctrl[ictrl].x==0))
#define RELEASED(x) ((ctrl[ictrl].x==0)&&(old_ctrl[ictrl].x==1))
#define HELD(x) ((ctrl[ictrl].x==1))

static void get_controller_down(struct controller_data_s * pCtrl, int ictrl) {
	memset(&ctrl[ictrl], 0, sizeof (struct controller_data_s));
	memset(pCtrl, 0, sizeof (struct controller_data_s));

	get_controller_data(&ctrl[ictrl], ictrl);

	if (PUSHED(down))
		pCtrl->down = 1;
	if (PUSHED(up))
		pCtrl->up = 1;
	if (PUSHED(left))
		pCtrl->left = 1;
	if (PUSHED(right))
		pCtrl->right = 1;
	if (PUSHED(a))
		pCtrl->a = 1;
	if (PUSHED(b))
		pCtrl->b = 1;
	if (PUSHED(x))
		pCtrl->x = 1;
	if (PUSHED(y))
		pCtrl->y = 1;
	if (PUSHED(start))
		pCtrl->x = 1;
	if (PUSHED(select))
		pCtrl->y = 1;
	if (PUSHED(logo))
		pCtrl->y = 1;

	old_ctrl[ictrl] = ctrl[ictrl];
}

static int getch2_internal(void) {
	int i = 0;

	int key = -1;
	static s64 lasttime = 0;
	s64 curtime = mftb();

	usb_do_poll();

	if (getch2_status && tb_diff_msec(curtime, lasttime) > 60) {

		struct controller_data_s d_ctrl;

		get_controller_down(&d_ctrl, i);
		
		if(gui_input_use==0){
			if (d_ctrl.left)
				key = KEY_LEFT;
			if (d_ctrl.right)
				key = KEY_RIGHT;
			if (d_ctrl.up)
				key = KEY_UP;
			if (d_ctrl.down)
				key = KEY_DOWN;

			if (d_ctrl.start)
				key = KEY_ENTER;
			if (d_ctrl.select)
				key = ' ';
			if (d_ctrl.logo)
				key = 'X';

			if (d_ctrl.a)
				key = 'a';
			if (d_ctrl.b)
				key = 'b';
			if (d_ctrl.x)
				key = 'x';

			if (d_ctrl.rb)
				key = 'r';
			if (d_ctrl.lb)
				key = 'l';

/*
			if (ctrl[i].left)
				key = KEY_LEFT;
			if (ctrl[i].right)
				key = KEY_RIGHT;
			if (ctrl[i].up)
				key = KEY_UP;
			if (ctrl[i].down)
				key = KEY_DOWN;

			if (ctrl[i].start)
				key = KEY_ENTER;
			if (ctrl[i].select)
				key = ' ';
			if (ctrl[i].logo)
				key = 'X';

			if (ctrl[i].a)
				key = 'a';
			if (ctrl[i].b)
				key = 'b';
			if (ctrl[i].x)
				key = 'x';

			if (ctrl[i].rb)
				key = 'r';
			if (ctrl[i].lb)
				key = 'l';
 */ 
			
			if (ctrl[i].rt>100)
				key = 'R';
			if (ctrl[i].lt>100)
				key = 'L';

			// kill => next !!!
			if(ctrl[i].rb&ctrl[i].lb){
				//key = 'q';
				//key = 'n';
				exit(0);
			}
		}
		
		// Always
		if (d_ctrl.y)
			key = 'y';

		lasttime = curtime;
	}

	return key;
}

void getch2(void) {
	int r = getch2_internal();
	if (r >= 0)
		mplayer_put_key(r);
}

void getch2_enable(void) {
	TR;
	usb_do_poll();
	// hack
	int i = 0;
	for(i=0;i<4;i++){
		struct controller_data_s ctrl_zero = {};
		set_controller_data(i,&ctrl_zero);
	}
	getch2_status = 1;
}

void getch2_disable(void) {
	TR;
	if (!getch2_status) return; // already disabled / never enabled
	getch2_status = 0;
}


#if defined(HAVE_LANGINFO) && defined(CONFIG_ICONV)
#include <locale.h>
#include <langinfo.h>
#endif

#ifdef CONFIG_ICONV

char* get_term_charset(void) {
#ifdef HAVE_LANGINFO
	static const char *charset_aux = "ASCII";
	char *charset = NULL;
	setlocale(LC_CTYPE, "");
	charset = nl_langinfo(CODESET);
	setlocale(LC_CTYPE, "C");
	if (charset == NULL || charset[0] == '\0')
		charset = charset_aux;
#else
	static const char *charset = "ASCII";
#endif
	return charset;
}
#endif

