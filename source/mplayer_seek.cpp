#include <stdio.h>
#include <math.h>
#include "gettext.h"
#include "menu.h"

//**************************************************************************
// Seek utils functions
//**************************************************************************

#define _(x)	gettext(x)

xmplayer_seek_information * playerSeekOpen(char * file)
{
	xmplayer_seek_information * seek = NULL;
	load_file(file, (void**)&seek, NULL);
	return seek;
}

double playerSeekPrompt(char * seekfile)
{
	int min, sec, hr;
	char seekstring[100];
	double seektime = 0;
	xmplayer_seek_information * seek = playerSeekOpen(seekfile);
	if (seek)
		seektime = seek->seek_time;		
		
	hr = (seektime / 3600);
	min = fmod(seektime, 3600) / 60;
	sec = fmod(seektime, 60);
	if (seektime < 3600) {
		sprintf(seekstring, "%s %02d:%02d", _("Resume from"), min, sec);
	} else {
		sprintf(seekstring, "%s %d:%02d:%02d", _("Resume from"), hr, min, sec);
	}
		
	if (SmallWindowPrompt(seekstring, "Start from beginning")) {
		return seektime;
	}
	else {
		return 0;
	}
}

void format_time(char * dest, double time)
{
	int min, sec, hr;
	hr = (time / 3600);
	min = fmod(time, 3600) / 60;
	sec = fmod(time, 60);

	sprintf(dest, "%d:%02d:%02d", hr, min, sec);
}
