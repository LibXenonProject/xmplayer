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

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>

#include "libavutil/intreadwrite.h"
#include "unrar.h"
#include "mp_msg.h"
//char* rar_arch_filename;
/***************************************************************************/

#define RAR_HEADER_SIZE 128

#if !defined(O_BINARY)
    #define O_BINARY 0
#endif

int rar_init_part(struct rar_archive *rar)
{
    uint8_t buf[RAR_HEADER_SIZE];
    int i, len;
    uint16_t size;
    uint32_t spp;
    unsigned int header_size;
    
    mp_msg(MSGT_OPEN, MSGL_DBG2, "[rar] rar_init_part\n");

    // get current rar filesize
    rar->cur_file_size = lseek(rar->fd, 0L, SEEK_END);
    if (rar->cur_file_size < 0) {
        mp_msg(MSGT_STREAM, MSGL_ERR, "[rar] lseek failed: %s\n", 
            strerror(errno));
        return -1;
    }
    // seek back to start
    lseek(rar->fd, 0L, SEEK_SET);

    // read & parse header
    len = read(rar->fd, buf, RAR_HEADER_SIZE);
    if (len < 0) {
        mp_msg(MSGT_STREAM, MSGL_ERR, "[rar] read failed: %s\n", 
            strerror(errno));
        return -1;
    } else if (len < 60) {
        mp_msg(MSGT_STREAM, MSGL_ERR, 
            "[rar] failed to read rar header (too few bytes read): %d\n", 
            len);
        return -1;
    }

    i = 0x14;

    size = AV_RL16(buf + i + 0x05); // size of file_header block
    header_size = i + size; // size of total header
    spp = AV_RL32(buf + i + 0x07); // pack size
    rar->trailing_bytes = rar->cur_file_size - header_size - spp;
    
    // seek past header
    if (lseek(rar->fd, header_size, SEEK_SET) < 0) {
        mp_msg(MSGT_STREAM, MSGL_ERR, "[rar] lseek failed: %s\n", 
            strerror(errno));
        return -1;
    }

    return 0;
}

// open new file
int rar_openfile(struct rar_archive *rar, char *filename) 
{
    mp_msg(MSGT_OPEN, MSGL_DBG2, "[rar] rar_openfile: %s\n", filename);

    if (rar->fd != -1)
        close(rar->fd);

    rar->fd = open(filename, O_RDONLY|O_BINARY);
 
    if (rar->fd == -1) {
        mp_msg(MSGT_STREAM, MSGL_V, "[rar] failed to open part %s: %s\n", 
            filename, strerror(errno));
        return -1;
    }

    rar_init_part(rar);

    return rar->fd;
}

// open part number _num_
int rar_openpart(struct rar_archive *rar, int num) 
{
    char filename[1024];
    
    mp_msg(MSGT_OPEN, MSGL_DBG2, "[rar] rar_openpart %d\n", num);
    
    if (num == rar->cur_file) {
        rar_init_part(rar);
        return rar->fd;
    }
    
    // generate filename for the given volume number
    if (rar->new_naming_scheme) {
        snprintf(filename, 1024, "%s.part%0*d.rar",
            rar->filename,
            rar->num_length,
            num + 1);
    } else {
        if (num == 0) {
            snprintf(filename, 1024, "%s.rar",
                rar->filename);
        } else if (num - 1 > 99) {
            snprintf(filename, 1024, "%s.s%02d",
                rar->filename,
                num - 101);
        } else {
            snprintf(filename, 1024, "%s.r%02d",
                rar->filename,
                num - 1);
        }
    }

    if (rar_openfile(rar, filename) < 0) {
        return -1;
    }

    mp_msg(MSGT_STREAM, MSGL_V, "[rar] opened part %d: %s\n", num+1, filename);

    rar->cur_file = num;
    
    return rar->fd;
}

// open next part
int rar_nextpart(struct rar_archive *rar) 
{
    return rar_openpart(rar, rar->cur_file + 1);
}

int rar_has_rar_filename(char *f) 
{
    int i;
    i = strlen(f);

    if (i < 4) return 0;

    // file ends with .r\d\d
    if ((f[i-3] == 'r' || f[i-3] == 'R') &&
        f[i-2] >= '0' && f[i-2] <= '9' && f[i-1] >= '0' && f[i-1] <= '9')
        return 2;

    // file ends with .rar
    if ((f[i-3] == 'r' || f[i-3] == 'R') && 
        (f[i-2] == 'a' || f[i-2] == 'A') && (f[i-1] == 'r' || f[i-1] == 'R'))
        return 1;

    return 0;
}

// initialize:
//    - basename for all rar files
//    - figure out old or new naming scheme
//    - get header size to skip on each file
//    - get file- and packsize of the file inside the rars
int rar_init(struct rar_archive *rar) 
{
    mp_msg(MSGT_OPEN, MSGL_DBG2, "[rar] rar_init start\n");

    if (rar_openfile(rar, rar->filename) < 0)
        return -1;

    // get filesize
    rar->filesize = lseek(rar->fd, 0, SEEK_END);
    lseek(rar->fd, 0, SEEK_SET);

    // find naming scheme, header size and packsize
    {
        uint8_t buf[RAR_HEADER_SIZE];
        int i, len;
        int size;
        uint64_t s;
     
        // read rar headers
        len = read(rar->fd, buf, RAR_HEADER_SIZE);

        // test if it is marked as a rar archive
        if (len < 61 || memcmp(buf, "Rar!", 4) ||
                buf[0x09] != 0x73 || buf[0x16] != 0x74) {
            mp_msg(MSGT_OPEN, MSGL_DBG2, "[rar] no rar header detected in file\n");
            return -1;
        }

        i = 0x14; // start of file header

        // password protected archives aren't supported
        if (buf[i + 0x03] & 0x04) {
            mp_msg(MSGT_OPEN, MSGL_ERR,
                "[rar] Password protected rar volumes are not supported\n");
            return -1;
        }

        // compressed archives aren't supported
        if (buf[i + 0x19] != 0x30) {
            mp_msg(MSGT_OPEN, MSGL_ERR,
                "[rar] Compressed rar volumes are not supported\n");
            return -1;
        }

        // get naming scheme from archive header
        if (buf[0x0A] & 0x10) {
            rar->new_naming_scheme = 1;
        } else {
            rar->new_naming_scheme = 0;
        }
        
        // read file header

        if (buf[i+0x04] & 0x1) { // large file
            s = AV_RL32(buf + i +0x24); s = s << 32; // unpsize HIGH
            s += AV_RL32(buf + i + 0x0B) & 0xffffffff; // unpsize
            rar->size = s;
        } else
            rar->size = AV_RL32(buf+i+0x0B); // unpsize

        size = AV_RL16(buf + i + 0x05); // size of file_header block
        rar->header_size = i + size; // size of total header (from start to end of this header)
        rar->size_per_part = AV_RL32(buf + i + 0x07); // pack size
        rar->trailing_bytes = rar->filesize - rar->header_size - rar->size_per_part;
	char* file;
	  if (strlen(buf + i + 0x20) > 0) {	
		file = (buf + i + 0x20);
		rar->extension = strrchr(file, '.'); 
	} else if (strlen(buf + i + 0x28) > 0) {
		file = (buf + i + 0x28);
		rar->extension = strrchr(file, '.'); 
	}     
    }

    // set file basename
    {
        char *p;
        int i;
        // remove extension .rar or .r\d\d
        p = strrchr(rar->filename, '.');
        *p = '\0';

        // if new naming scheme, also remove .part\d+ from basename and find
        // the number of digits used in the filename
        if (rar->new_naming_scheme) {
            // locate .part\d+
            p = strrchr(rar->filename, '.');
            if (p)
                p = strstr(p, ".part");
            if (!p || p[5] < '0' || p[5] > '9') {
                // error parsing filename
                mp_msg(MSGT_OPEN, MSGL_ERR, "[rar] error parsing rar filename\n");
                return -1;
            }

            rar->num_length = 0;
            i = 5;
            while (p[i] >= '0' && p[i] <= '9') {
                i++;
                rar->num_length++;
            }
            *p = '\0';
        }
    }

    mp_msg(MSGT_OPEN, MSGL_INFO, "[rar] volume size: %u\n", rar->filesize);
    mp_msg(MSGT_OPEN, MSGL_INFO, "[rar] video filesize: %llu\n", rar->size);

    return 0;
}

/*************************************************************************/

struct rar_archive *rar_open(char *filename) 
{
    struct rar_archive *rar = malloc(sizeof(struct rar_archive));

    rar->filename = strdup(filename);
    rar->new_naming_scheme = 0;
    rar->filesize = 0;
    rar->cur_file = -1;
    rar->fd = -1;

    if (rar_init(rar) < 0 || rar_openpart(rar, 0) < 0) {
        rar_free(rar);
        return NULL;
    }

    return rar;
}

void rar_free(struct rar_archive *rar) 
{
    mp_msg(MSGT_OPEN, MSGL_DBG2, "[rar] rar_free\n");

    free(rar->filename);
    if (rar->fd != -1) {
        close(rar->fd);
        rar->fd = -1;
    }
    free(rar);
}

int rar_read(struct rar_archive *rar, void *buf, int count) 
{
    int res = 0, n;
    off_t pos;

    while (count > 0) {
        pos = lseek(rar->fd, 0L, SEEK_CUR);
        if (pos < 0) {
            mp_msg(MSGT_STREAM, MSGL_ERR, "[rar] lseek failed: %s\n", strerror(errno));
            return -1;
        }

        if (rar->cur_file_size - rar->trailing_bytes - pos < count) {
            // read last part of this file
            n = read(rar->fd, (uint8_t*)buf + res, rar->cur_file_size - rar->trailing_bytes - pos);
            if (n < 0) {
                mp_msg(MSGT_STREAM, MSGL_ERR, "[rar] read failed: %s\n", strerror(errno));
                return n;
            }
            if (rar_nextpart(rar) < 0) { // there is no next part
                return n;
            }
            res += n;
            count -= n;
        } else {
            // read count bytes from file
            n = read(rar->fd, (uint8_t*)buf + res, count);
            if (n < 0) {
                mp_msg(MSGT_STREAM, MSGL_ERR, "[rar] read failed: %s\n", strerror(errno));
                return n;
            }
            res += n;
            count -= n;
            break;
        }
    }

    return res;
}

off_t rar_seek(struct rar_archive *rar, off_t offset) 
{
    int p;
    off_t o, res = -1;

    p = offset / rar->size_per_part;

    if (rar_openpart(rar, p) < 0) {
        return -1;
    }

    o = offset % rar->size_per_part + rar->header_size;
    if ((res = lseek(rar->fd, o, SEEK_SET)) < 0)
        return res;

    return offset;
}

off_t rar_size(struct rar_archive *rar) 
{
    return rar->size;
}

char* playerGetRarExt (char * filename) {
    struct rar_archive *rar = rar_open(filename);
	if (strlen(rar->extension) > 0) {
		return rar->extension;
		rar_free(rar);
	}
}
