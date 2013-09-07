/* 
 * File:   mplayer.h
 * Author: cc
 *
 * Created on 29 avril 2012, 03:57
 */

#ifndef MPLAYER_FUNC_H
#define	MPLAYER_FUNC_H

#ifdef	__cplusplus
extern "C" {
#endif
	// force to relink if mplayer or ffmpeg changed
	#include "../mplayer/version.h"
	#include "mplayer_seek.h"
	
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
	char* playerGetRarExt (char * filename);
	// use that to load a new file
	// mplayer.cIf the interface file and object file get out of sync for any reason, t
	void mplayer_load(char * filename);

	// return to gui - doesn't exit mplayer process
	// newmain.cpp
	void mplayer_return_to_gui(void);

	// return to mplayer
	// newmain.cpp
	void mplayer_return_to_player(void);
	
	void mplayer_switch_audio(void);
	void mplayer_switch_mute(void);
	void mplayer_switch_subtitle(void);
	void mplayer_switch_fullscreen(void); 
	void mplayer_switch_vsync(void);
	void mplayer_switch_balance(int left); //1 towards left, 0 towards right (broken)
	void mplayer_switch_volume(int up); //1 turns up, 0 turns down
	void mplayer_turn_off_subtitle(void);

	char* mplayer_get_subtitle(void);
	char* mplayer_get_mute(void);	
	char* mplayer_get_balance(void);
	float mplayer_get_volume(void);
      //  char* playerGetAudioStreams();
	double mplayer_get_elapsed(void);
	double mplayer_get_duration(void);
	const char * mplayer_get_filename(void);
	int mplayer_get_status(void);
	int mplayer_get_pause(void);
	void mplayer_gui_asked(void);

	const char * mplayet_get_meta_data(metadata_t type);

	void mplayer_osd_close(void);
	void mplayer_osd_draw(int level);
	void mplayer_osd_open(void);
	
	// load save return 0 if fail, 1 on success
	int save_file(const char * filename, void * in, int size);
	int load_file(const char * filename, void ** buf, int * size);
	int file_exists(const char * filename);

#ifdef	__cplusplus
}
#endif

#endif	/* MPLAYER_FUNC_H */

