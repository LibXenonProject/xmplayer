/*  
 * reads video file inside uncompressed, multi-volume rar archives
 * 
 * Copyright (C) 2009  Emiel Neggers <emiel@neggers.net>
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

/*************************************************************************/
/* RAR ARCHIVE HEADERS:

 MARK_HEAD (7 bytes)
 0x00-0x06    Fixed byte sequence: 0x52 0x61 0x72 0x21 0x1a 0x07 0x00
 
 MAIN_HEAD (13 bytes)
 0x07-0x08    Head crc
 0x09         Header type: 0x73
 0x0A-0x0B    Bit flags:
                0x0001  - Volume attribute (archive volume)
                0x0002  - Archive comment present
                          RAR 3.x uses the separate comment block
                          and does not set this flag.

                0x0004  - Archive lock attribute
                0x0008  - Solid attribute (solid archive)
                0x0010  - New volume naming scheme (\'volname.partN.rar\')
                0x0020  - Authenticity information present
                          RAR 3.x does not set this flag.

                0x0040  - Recovery record present
                0x0080  - Block headers are encrypted
                0x0100  - First volume (set only by RAR 3.0 and later)

                other bits in HEAD_FLAGS are reserved for
                internal use
 0x0C-0x0D      Archive header total size including archive comments
 0x0E-0x13      reserved

 FILE_HEAD (40 bytes + variable length) (starting position 0x14)
 0x00-0x01    Head crc
 0x02         Header type: 0x74
 0x03-0x04    Bit flags:
                0x01 - file continued from previous volume
                0x02 - file continued in next volume
                0x04 - file encrypted with password

                0x08 - file comment present
                       RAR 3.x uses the separate comment block
                       and does not set this flag.

                0x10 - information from previous files is used (solid flag)
                       (for RAR 2.0 and later)

                bits 7 6 5 (for RAR 2.0 and later)

                     0 0 0    - dictionary size   64 KB
                     0 0 1    - dictionary size  128 KB
                     0 1 0    - dictionary size  256 KB
                     0 1 1    - dictionary size  512 KB
                     1 0 0    - dictionary size 1024 KB
                     1 0 1    - dictionary size 2048 KB
                     1 1 0    - dictionary size 4096 KB
                     1 1 1    - file is directory

               0x100 - HIGH_PACK_SIZE and HIGH_UNP_SIZE fields
                       are present. These fields are used to archive
                       only very large files (larger than 2Gb),
                       for smaller files these fields are absent.

               0x200 - FILE_NAME contains both usual and encoded
                       Unicode name separated by zero. In this case
                       NAME_SIZE field is equal to the length
                       of usual name plus encoded Unicode name plus 1.

               0x400 - the header contains additional 8 bytes
                       after the file name, which are required to
                       increase encryption security (so called 'salt').

               0x800 - Version flag. It is an old file version,
                       a version number is appended to file name as ';n'.

              0x1000 - Extended time field present.

              0x8000 - this bit always is set, so the complete
                       block size is HEAD_SIZE + PACK_SIZE
                       (and plus HIGH_PACK_SIZE, if bit 0x100 is set)

 0x05-0x06    File header full size including file name and comments
 0x07-0x0A    Compressed file size
 0x0B-0x0E    Uncompressed file size
 0x0F         Operating system used for archiving
 0x10-0x13    File CRC
 0x14-0x17    Date and time in standard MS DOS format
 0x18         RAR version needed to extract file
              Version number is encoded as 10 * Major version + minor version
 0x19         Packing method (0x30 - storing)
 0x1A-0x1B    File name size NAME_SIZE
 0x1C-0x1F    File attributes
 -- optional (if HIGH_PACK_SIZE and HIGH_UNP_SIZE are set) --
 0x20-0x23    High 4 bytes of 64 bit value of compressed file size
 0x24-0x27    High 4 bytes of 64 bit value of uncompressed file size
 --
 0x20 or 0x28   File name - string of NAME_SIZE bytes size
*/

#ifndef MPLAYER_UNRAR_H
#define MPLAYER_UNRAR_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

struct rar_archive {
    char *filename;
    char *extension;
    unsigned int new_naming_scheme; // 1 = yes, 0 = no

    off_t size; // size of the first file in the rar
    unsigned int header_size; // size of the header junk to skip in each file
    unsigned int size_per_part;
    unsigned int filesize;
    unsigned int trailing_bytes;
  
    int fd; // current file descriptor
    int num_length; // part.001.rar = 3, part.01.rar = 2, etc
    int cur_file; // currently opened file number (0 .. num_files - 1)
    unsigned int cur_file_size;
};

int rar_has_rar_filename(char *f);
struct rar_archive *rar_open(char *filename);
void rar_free(struct rar_archive *rar);
int rar_read(struct rar_archive *rar, void *buf, int count);
off_t rar_seek(struct rar_archive *rar, off_t offset);
off_t rar_size(struct rar_archive *rar);
char* playerGetRarExt (char* filename);
#endif /* MPLAYER_UNRAR_H */
