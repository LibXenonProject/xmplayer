#include <stdio.h>
#include <xetypes.h>
#include <limits.h>
#include <math.h>
#include <time/time.h>

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

int access(const char *__path, int __amode) {
	return -1;
}

void waitpid() {

}

int usleep(useconds_t __useconds) {
	udelay(__useconds);
	return 0;
}

