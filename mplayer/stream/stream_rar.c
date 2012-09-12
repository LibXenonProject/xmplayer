/*    
 * reads video file inside of uncompressed, multi-volume rar archives
 * 
 * Copyright (C) 2009    Emiel Neggers <emiel@neggers.net>
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

#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "mp_msg.h"
#include "stream.h"
#include "help_mp.h"
#include "m_option.h"
#include "m_struct.h"

#include "unrar.h"

static struct stream_priv_s {
    char *filename;
    char *filename2;
    struct rar_archive *rar;
} stream_priv_dflts = {
    NULL, NULL, NULL
};

#define ST_OFF(f) M_ST_OFF(struct stream_priv_s, f)
/// URL definition
static m_option_t stream_opts_fields[] = {
    {"string", ST_OFF(filename), CONF_TYPE_STRING, 0, 0, 0, NULL},
    {"filename", ST_OFF(filename2), CONF_TYPE_STRING, 0, 0, 0, NULL},
    { NULL, NULL, 0, 0, 0, 0,    NULL }
};
static struct m_struct_st stream_opts = {
    "rar",
    sizeof(struct stream_priv_s),
    &stream_priv_dflts,
    stream_opts_fields
};    

static int fill_buffer(stream_t *s, char* buffer, int max_len)
{
    struct stream_priv_s* p = s->priv;
    int r = rar_read(p->rar, buffer, max_len);
    return (r <= 0) ? -1 : r;
}

static int seek(stream_t *s, off_t newpos)
{
    struct stream_priv_s* p = s->priv;
    s->pos = newpos;
    if (rar_seek(p->rar, s->pos) < 0) {
        s->eof = 1;
        return 0;
    }
    return 1;
}

static int control(stream_t *s, int cmd, void *arg) 
{
    struct stream_priv_s* p = s->priv;
    switch(cmd) {
    case STREAM_CTRL_GET_SIZE:
        *((off_t*)arg) = rar_size(p->rar);
        return 1;
    }
    return STREAM_UNSUPPORTED;
}

static int open_rar(stream_t *stream, int mode, void* opts, int* file_format) 
{
    struct stream_priv_s *p = opts;
    char *filename;
    int res = STREAM_UNSUPPORTED;

    // get filename from option 'url'
    if (p->filename)
        filename = p->filename;
    else if (p->filename2)
        filename = p->filename2;
    else { 
        res = STREAM_UNSUPPORTED;
        goto out;
    }

    // do not handle any non-rar file
    if (!rar_has_rar_filename(filename)) {
        res = STREAM_UNSUPPORTED;
        goto out;
    }

    // no write support
    if (mode != STREAM_READ) {
        mp_msg(MSGT_OPEN, MSGL_ERR, "[rar] Unsupported open mode %d\n", mode);
        res = STREAM_ERROR;
        goto out;
    }

    p->filename = filename;
    p->rar = rar_open(p->filename);

    if (p->rar == NULL) {
        mp_msg(MSGT_OPEN, MSGL_ERR, "[rar] Failed to open rar archive %s\n", p->filename);
        res = STREAM_UNSUPPORTED;
        goto out;
    }

    stream->end_pos = rar_size(p->rar);
    stream->type = STREAMTYPE_FILE;

    stream->fd = -1;
    stream->seek = seek;
    stream->fill_buffer = fill_buffer;
    stream->control = control;
    stream->priv = p;

    res = STREAM_OK;

out:
    if (res != STREAM_OK)
        m_struct_free(&stream_opts, opts);
    return res;
}

stream_info_t stream_info_rar = {
    "RAR",
    "rar",
    "Emiel Neggers",
    "play videos stored in uncompressed rar files",
    open_rar,
    { "rar", "", NULL },
    &stream_opts,
    1 // Urls are an option string
};
