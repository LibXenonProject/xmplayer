#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/iosupport.h>
#include <xenon_soc/xenon_power.h>
#include <debug.h>

extern void cErrorPrompt(const char *msg);

static char MPLAYER_ENV[100];
char MPLAYER_DATADIR[100]; 
char MPLAYER_CONFDIR[100]; 
char MPLAYER_LIBDIR[100];

void pthread_init();

void init_mplayer(){
	int i = 0;
	int found = 0;
	struct stat s;
	char tmp_dir[100];
	pthread_init();	

	// check for a device mount with a mplayer dir on it
	for(i = 3;i<STD_MAX;i++) {
		if (devoptab_list[i]->structSize) {
			sprintf(tmp_dir, "%s:/mplayer/mplayer.conf", devoptab_list[i]->name);
			if(stat(tmp_dir,&s) == 0) {
				sprintf(MPLAYER_ENV,"%s:",devoptab_list[i]->name);
				sprintf(MPLAYER_DATADIR,"%s:/mplayer",devoptab_list[i]->name);
				sprintf(MPLAYER_CONFDIR,"%s:/mplayer",devoptab_list[i]->name);
				sprintf(MPLAYER_LIBDIR,"%s:/mplayer",devoptab_list[i]->name);
				found ++;
				break;
			}
		}
	}
	// Display an error message
	if (found == 0) {
		cErrorPrompt("Can't found required files, exit");
	}
	
	setenv("HOME",MPLAYER_ENV,1);
}

