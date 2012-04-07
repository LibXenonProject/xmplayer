
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
#include <xetypes.h>
#include <xenon_soc/xenon_power.h>
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
#include <xenon_smc/xenon_smc.h>

#include <diskio/ata.h>

#include <libfat/fat.h>

#include <math.h>

char MPLAYER_DATADIR[100]; 
char MPLAYER_CONFDIR[100]; 
char MPLAYER_LIBDIR[100];

void init_xenon(){
	xenon_make_it_faster(XENON_SPEED_FULL);
	
	xenos_init(VIDEO_MODE_AUTO);
	console_init();
	
	setenv("HOME","usb:",1);
	strcpy(MPLAYER_DATADIR,"usb:/mplayer");
	strcpy(MPLAYER_CONFDIR,"usb:/mplayer");
	strcpy(MPLAYER_LIBDIR,"usb:/mplayer");
	
	usb_init();
	usb_do_poll();
	xenon_ata_init();
	
	fatInitDefault();
}


long long llrint(double x) {
    union {
        double d;
        uint64_t u;
    } u = {x};

    uint64_t absx = u.u & 0x7fffffffffffffffULL;

    // handle x is zero, large, or NaN simply convert to long long and return
    if (absx >= 0x4330000000000000ULL) {
        long long result = (long long) x; //set invalid if necessary

        //Deal with overflow cases
        if (x < (double) LONG_LONG_MIN)
            return LONG_LONG_MIN;

        // Note: float representation of LONG_LONG_MAX likely inexact,
        //		  which is why we do >= here
        if (x >= -((double) LONG_LONG_MIN))
            return LONG_LONG_MAX;

        return result;
    }

    // copysign( 0x1.0p52, x )
    u.u = (u.u & 0x8000000000000000ULL) | 0x4330000000000000ULL;

    //round according to current rounding mode
    x += u.d;
    x -= u.d;

    return (long long) x;
}



int	access(const char *__path, int __amode ){
	return -1;
}

void waitpid(){
	
}

int usleep(useconds_t __useconds){
	udelay(__useconds);
}


