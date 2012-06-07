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

	enum metadata_s {
		/* common info */
		META_NAME = 0,

		/* video stream properties */
		META_VIDEO_CODEC,
		META_VIDEO_BITRATE,
		META_VIDEO_RESOLUTION,

		/* audio stream properties */
		META_AUDIO_CODEC,
		META_AUDIO_BITRATE,
		META_AUDIO_SAMPLES,

		/* ID3 tags and other stream infos */
		META_INFO_TITLE,
		META_INFO_ARTIST,
		META_INFO_ALBUM,
		META_INFO_YEAR,
		META_INFO_COMMENT,
		META_INFO_TRACK,
		META_INFO_GENRE
	};

	typedef enum metadata_s metadata_t;

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
	
	void playerSwitchAudio();
	void playerSwitchSubtitle();
	void playerSwitchFullscreen();
	void playerSwitchVsync();
	
	double playerGetElapsed();
	double playerGetDuration();
	const char * playerGetFilename();
	int playerGetStatus();
	void playerGuiAsked();
	
	const char * playetGetMetaData(metadata_t type);


	void mplayer_osd_close();
	void mplayer_osd_draw(int level);
	void mplayer_osd_open();

#ifdef	__cplusplus
}
#endif

#endif	/* MPLAYER_H */

