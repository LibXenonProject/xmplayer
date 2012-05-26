#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/iosupport.h>
#include <xenon_soc/xenon_power.h>
#include <debug.h>
// libxenon miss

static char MPLAYER_ENV[100];
char MPLAYER_DATADIR[100]; 
char MPLAYER_CONFDIR[100]; 
char MPLAYER_LIBDIR[100];

void pthread_init();

#define DEFAULT_DEVICE 3

void init_mplayer(){
	pthread_init();	

	// try to use the 1st device
	sprintf(MPLAYER_ENV,"%s:",devoptab_list[DEFAULT_DEVICE]->name);
	sprintf(MPLAYER_DATADIR,"%s:/mplayer",devoptab_list[DEFAULT_DEVICE]->name);
	sprintf(MPLAYER_CONFDIR,"%s:/mplayer",devoptab_list[DEFAULT_DEVICE]->name);
	sprintf(MPLAYER_LIBDIR,"%s:/mplayer",devoptab_list[DEFAULT_DEVICE]->name);
	
	setenv("HOME",MPLAYER_ENV,1);
}

