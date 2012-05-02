#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
// libxenon miss
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <debug.h>
#include <stdio.h>
#include <usb/usbmain.h>
#include <ppc/timebase.h>
#include <threads/threads.h>
#include <xetypes.h>
#include <xenon_smc/xenon_smc.h>
#include <console/console.h>
#include <sys/time.h>
#include <time/time.h>

#include <byteswap.h>

#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>

#include <limits.h>
// libxenon miss
#include <sys/time.h>
#include <time/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <time/time.h>

#include <math.h>

#include <debug.h>

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
