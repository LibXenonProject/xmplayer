#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
// libxenon miss

char MPLAYER_DATADIR[100]; 
char MPLAYER_CONFDIR[100]; 
char MPLAYER_LIBDIR[100];

void pthread_init();

void init_mplayer(){
	pthread_init();	
	setenv("HOME","uda:",1);
	strcpy(MPLAYER_DATADIR,"uda:/mplayer");
	strcpy(MPLAYER_CONFDIR,"uda:/mplayer");
	strcpy(MPLAYER_LIBDIR,"uda:/mplayer");
}
