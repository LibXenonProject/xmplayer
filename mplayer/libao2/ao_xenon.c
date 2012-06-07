/*
 * null audio output driver
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <malloc.h>
#include <string.h>
#include "config.h"
#include "libaf/af_format.h"
#include "audio_out.h"
#include "audio_out_internal.h"

#include <xenon_sound/sound.h>
#include <pci/io.h>
#include <ppc/cache.h>

static const ao_info_t info = {
        "Xenon audio output",
        "xenon",
        "Ced2911",
        "#libxenon"
};

#define XENON_BUFFER_SIZE 64*1024

LIBAO_EXTERN(xenon)

/*
// to set/get/query special features/parameters
 */
static int control(int cmd, void *arg) {
        return -1;
}

static void sound_reset(void) {
        xenon_sound_init();
}

/*
// open & setup audio device
// return: 1=success 0=fail
 */
static int init(int rate, int channels, int format, int flags) {
        xenon_sound_init();

        ao_data.buffersize = XENON_BUFFER_SIZE;
        ao_data.outburst = 2048;
        ao_data.channels = 2;
        ao_data.samplerate = 48000;
        ao_data.format = AF_FORMAT_S16_LE;
        ao_data.bps = ao_data.channels * ao_data.samplerate * sizeof (signed short);

        return 1;
}

/*
// close audio device
 */
static void uninit(int immed) {

}

/*
// stop playing and empty buffers (for seeking/pause)
 */
static void reset(void) {
        sound_reset();
}

/*
// stop playing, keep buffers (for pause)
 */
static void audio_pause(void) {
        reset();
}

/*
// resume playing, after audio_pause()
 */
static void audio_resume(void) {
        sound_reset();
}

/*
// return: how many bytes can be played without blocking
 */
static int get_space(void) {

        return xenon_sound_get_free();
}

/*
// plays 'len' bytes of 'data'
// it should round it down to outburst*n
// return: number of bytes played
 */
static int play(void* data, int len, int flags) {
        if (!(flags & AOPLAY_FINAL_CHUNK))
                len -= len % ao_data.outburst;

        xenon_sound_submit(data, len);

        return len;
}

/* 
 * return: delay in seconds between first and last sample in buffer
 */
static float get_delay(void) {
        //return ((float) (ao_data.buffersize - xenon_sound_get_unplayed())) / ((float) ao_data.bps);
        return 0.f;
}

