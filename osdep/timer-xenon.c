/*
 * precise timer routines for Linux
 * copyright (C) LGB & A'rpi/ASTRAL
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "config.h"
#include "timer.h"

#include <ppc/timebase.h>
#include <time/time.h>

const char timer_name[] = "Xenon native";

int usec_sleep(int usec_delay)
{
	udelay(usec_delay);
	return 0;
}

static inline uint64_t xenon_time(){
	return mftb()/PPC_TIMEBASE_FREQ;
}

// Returns current time in microseconds
unsigned int GetTimer(void)
{
	return mftb()/(PPC_TIMEBASE_FREQ/1000000);
}

// Returns current time in milliseconds
unsigned int GetTimerMS(void)
{
	return mftb()/(PPC_TIMEBASE_FREQ/1000);
}

static unsigned int RelativeTime = 0;

// Returns time spent between now and last call in seconds
float GetRelativeTime(void)
{
    unsigned int t,r;
    t = GetTimer();
    //t *= 16; printf("time = %ud\n", t);
    r = t - RelativeTime;
    RelativeTime = t;
    return (float) r * 0.000001F;
}

// Initialize timer, must be called at least once at start
void InitTimer(void)
{
    GetRelativeTime();
}
