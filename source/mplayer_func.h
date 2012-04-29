/* 
 * File:   mplayer.h
 * Author: cc
 *
 * Created on 29 avril 2012, 03:57
 */

#ifndef MPLAYER_H
#define	MPLAYER_H

#ifdef	__cplusplus
extern "C" {
#endif
	// force to relink if mplayer or ffmpeg changed
	#include "../mplayer/version.h"

	// mplayer_func.c
	void init_mplayer(void);

	// mplayer.c - must be done only 1 time !!!
	int mplayer_main(int argc, char *argv[]);

	// use that to load a new file
	// mplayer.c
	void mplayer_load(char * filename);
	
	// return to gui - doesn't exit mplayer process
	// newmain.cpp
	void mplayer_return_to_gui();
	
	// return to mplayer
	// newmain.cpp
	void mplayer_return_to_player();
	
	
	 void mplayer_osd_close();
	 void mplayer_osd_draw();
	 void mplayer_osd_open();

#ifdef	__cplusplus
}
#endif

#endif	/* MPLAYER_H */

