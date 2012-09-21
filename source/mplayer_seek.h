#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

	// total struct size is 100
	typedef struct {
		time_t last_date_played;	// 4 - when we saved the information
		double seek_time;			// 12 - the position in the stream
		unsigned char not_used[88]; // for later use
	} xmplayer_seek_information;

#ifdef	__cplusplus
}
#endif
