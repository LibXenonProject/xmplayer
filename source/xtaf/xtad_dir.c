#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/reent.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <debug.h>

#include "xtaf.h"

static xtaf_partition_private* getPartitionFromPath(const char* path) {
	const devoptab_t *devops;

	devops = GetDeviceOpTab(path);

	if (!devops) {
		return NULL;
	}

	return (xtaf_partition_private*) devops->deviceData;
}

DIR_ITER* xtaf_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path) {
	xtaf_dir_entry * state = dirState->dirStruct;
	xtaf_partition_private* partition = getPartitionFromPath(path);
	char tmp_name[260];
	int err = 0;

	if (partition == NULL) {
		r->_errno = ENODEV;
		return NULL;
	}

	// Move the path pointer to the start of the actual path
	if (strchr(path, ':') != NULL) {
		path = strchr(path, ':') + 1;
	}
	if (strchr(path, ':') != NULL) {
		r->_errno = EINVAL;
		return NULL;
	}

	memset(&state->finfo, 0, sizeof (struct _xtaf_directory_s));
	state->partition = partition;

	// set this to 0
	partition->current_sector = 0;
	partition->extent_offset = 0;
	partition->extent_next_cluster = 0;
	partition->extent_offset = 0;

	strcpy(tmp_name, path);

	while (1) {
		err = xtaf_get_dir(state, tmp_name);
		if (err >= 0) {
			switch (err) {
				case 2:
				{
					// folder found
					if ((tmp_name[0] == 0) || (tmp_name[0] == '/' && tmp_name[1] == 0)) {
						return dirState;
					} else {
						continue;
					}
				}
				case 1:
				{
					// file found
					//r->_errno = ENOTDIR;
					//return NULL;
					continue;
				}
				case 0:{
					// root fs
					return dirState;
				}
			}
		} else {
			break;
		}
	}

	r->_errno = ENOENT;
	return NULL;

}

/** build a date time **/
static inline time_t xtaf_build_time(uint16_t date, uint32_t time) {
	return 0;
}

int xtaf_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st) {
	xtaf_dir_entry * state = dirState->dirStruct;
	xtaf_partition_private * partition = state->partition;
	int err = 0;

	if (dirState == NULL) {
		r->_errno = EBADF;
		return -1;
	}

	err = xtaf_parse_entry(partition, &state->finfo);

	if (err == 0) {
		//st->st_ino = (ino_t) (state->finfo>first_cluster); // The file serial number is the start cluster
		st->st_size = state->finfo.file_size;
		st->st_blksize = partition->clusters_size; // Prefered file I/O block size
		st->st_blocks = (st->st_size + partition->clusters_size - 1) / partition->clusters_size; // File size in blocks

		st->st_mode = state->finfo.flags&0x10?S_IFDIR:S_IFREG;		
		
		//printf("mode %02x\n", st->st_mode);
		
		st->st_atime = xtaf_build_time(state->finfo.access_date, state->finfo.access_time);
		st->st_ctime = xtaf_build_time(state->finfo.creation_date, state->finfo.creation_time);
		st->st_mtime = xtaf_build_time(state->finfo.update_date, state->finfo.update_time);

		strncpy(filename, state->finfo.filename, 0x2A);

		return 0;
	}

	r->_errno = ENOENT;
	return -1;
}

int xtaf_dirclose_r(struct _reent *r, DIR_ITER *dirState) {
	return 0;
}