#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/reent.h>
#include <sys/iosupport.h>
#include <debug.h>

#include "fakeio.h"
#include "xtaf.h"

#include "ata.h"

typedef struct {
	uint32_t cluster;
	sec_t sector;
	int32_t byte;
} FILE_POSITION;

static inline uint32_t XTAF_clusterToSector(xtaf_partition_private* partition, uint32_t cluster) {
	return ((cluster - 1) * partition->clusters_size) + partition->root_offset + partition->partition_start_offset;
}

static inline sec_t _clusterToSector(xtaf_partition_private* partition, uint32_t cluster) {
	// sector = ((cluster - 1) * cluster_size) + partition->root_offset + partition->partition_start_offset;
	return (cluster >= 1) ?
			((cluster - 1) * (sec_t) partition->clusters_size) + partition->partition_start_offset + partition->root_offset :
			partition->partition_start_offset + partition->root_offset;
}

static xtaf_partition_private* getPartitionFromPath(const char* path) {
	const devoptab_t *devops;

	devops = GetDeviceOpTab(path);

	if (!devops) {
		return NULL;
	}

	return (xtaf_partition_private*) devops->deviceData;
}

/** read a file into a buffer **/
ssize_t ffxtaf_read_r(struct _reent *r, int fd, char *ptr, size_t len) {
	xtaf_file_private* file = (xtaf_file_private*) fd;
	xtaf_partition_private * partition = NULL;
	CACHE* cache;
	FILE_POSITION position;
	uint32_t tempNextCluster;
	uint32_t bytesPerSector = XENON_DISK_SECTOR_SIZE;
	uint32_t bytesPerCluster;
	uint32_t sectorsPerCluster;
	unsigned int tempVar;
	size_t remain;
	bool flagNoError = true;

	// Short circuit cases where len is 0 (or less)
	if (len <= 0) {
		return 0;
	}

	// Make sure we can actually read from the file
	if (file == NULL) {
		r->_errno = EBADF;
		return -1;
	}

	partition = file->partition;
	sectorsPerCluster = partition->clusters_size;
	bytesPerCluster = sectorsPerCluster * bytesPerSector;
	// Don't try to read if the read pointer is past the end of file
	if (file->pos >= file->finfo.file_size) {
		r->_errno = EOVERFLOW;
		return 0;
	}

	// Don't read past end of file
	if (len + file->pos > file->finfo.file_size) {
		r->_errno = EOVERFLOW;
		len = file->finfo.file_size - file->pos;
	}

	remain = len;
	//position.byte = file->pos;
	
	position.sector = file->pos / XENON_DISK_SECTOR_SIZE;
	position.byte  = file->pos % XENON_DISK_SECTOR_SIZE;
	position.cluster = file->finfo.starting_cluster;
	
	cache = file->partition->cache;

	// Align to sector
	tempVar = bytesPerSector - position.byte;
	if (tempVar > remain) {
		tempVar = remain;
	}

	if ((tempVar < bytesPerSector) && flagNoError) {
		_XTAF_cache_readPartialSector(cache, ptr, _clusterToSector(partition, position.cluster) + position.sector,
				position.byte, tempVar);

		remain -= tempVar;
		ptr += tempVar;

		position.byte += tempVar;
		if (position.byte >= bytesPerSector) {
			position.byte = 0;
			position.sector++;
		}
	}

	// align to cluster
	// tempVar is number of sectors to read
	if (remain > (sectorsPerCluster - position.sector) * bytesPerSector) {
		tempVar = sectorsPerCluster - position.sector;
	} else {
		tempVar = remain / bytesPerSector;
	}

	if ((tempVar > 0) && flagNoError) {
		if (!_XTAF_cache_readSectors(cache, _clusterToSector(partition, position.cluster) + position.sector,
				tempVar, ptr)) {
			flagNoError = false;
			r->_errno = EIO;
		} else {
			ptr += tempVar * bytesPerSector;
			remain -= tempVar * bytesPerSector;
			position.sector += tempVar;
		}
	}

	// Move onto next cluster
	// It should get to here without reading anything if a cluster is due to be allocated
	if ((position.sector >= sectorsPerCluster) && flagNoError) {
		tempNextCluster = xfat_get_next_cluster(partition, position.cluster);
		/*
				if ((remain == 0) && (tempNextCluster == CLUSTER_EOF)) {
					position.sector = sectorsPerCluster;
				} else if (!_FAT_fat_isValidCluster(partition, tempNextCluster)) {
					r->_errno = EIO;
					flagNoError = false;
				} else {
		 */
		position.sector = 0;
		position.cluster = tempNextCluster;
		// }
	}

	// Read in whole clusters, contiguous blocks at a time
	while ((remain >= bytesPerCluster) && flagNoError) {
		uint32_t chunkEnd;
		uint32_t nextChunkStart = position.cluster;
		size_t chunkSize = 0;

		do {
			chunkEnd = nextChunkStart;
			nextChunkStart = xfat_get_next_cluster(partition, chunkEnd);
			chunkSize += bytesPerCluster;
		} while ((nextChunkStart == chunkEnd + 1) &&
#ifdef LIMIT_SECTORS
				(chunkSize + partition->bytesPerCluster <= LIMIT_SECTORS * partition->bytesPerSector) &&
#endif
				(chunkSize + bytesPerCluster <= remain));

		if (!_XTAF_cache_readSectors(cache, _clusterToSector(partition, position.cluster),
				chunkSize / bytesPerSector, ptr)) {
			flagNoError = false;
			r->_errno = EIO;
			break;
		}
		ptr += chunkSize;
		remain -= chunkSize;

		// Advance to next cluster
/*
		if ((remain == 0) && (nextChunkStart == CLUSTER_EOF)) {
			position.sector = partition->sectorsPerCluster;
			position.cluster = chunkEnd;
		} else if (!_FAT_fat_isValidCluster(partition, nextChunkStart)) {
			r->_errno = EIO;
			flagNoError = false;
		} else {
*/
			position.sector = 0;
			position.cluster = nextChunkStart;
//		}
	}

	// Read remaining sectors
	tempVar = remain / bytesPerSector; // Number of sectors left
	if ((tempVar > 0) && flagNoError) {
		if (!_XTAF_cache_readSectors(cache, _clusterToSector(partition, position.cluster),
				tempVar, ptr)) {
			flagNoError = false;
			r->_errno = EIO;
		} else {
			ptr += tempVar * bytesPerSector;
			remain -= tempVar * bytesPerSector;
			position.sector += tempVar;
		}
	}

	// Last remaining sector
	// Check if anything is left
	if ((remain > 0) && flagNoError) {
		_XTAF_cache_readPartialSector(cache, ptr,
				_clusterToSector(partition, position.cluster) + position.sector, 0, remain);
		position.byte += remain;
		remain = 0;
	}

	// Length read is the wanted length minus the stuff not read
	len = len - remain;

	// Update file information
	//file->pos = position;
	file->pos += len;
	return len;
}

/** read a file into a buffer **/
ssize_t xtaf_read_r(struct _reent *r, int fd, char *data, size_t len) {
	xtaf_file_private* file = (xtaf_file_private*) fd;
	xtaf_partition_private * partition = NULL;

	uint64_t sector = 0;
	uint64_t copied = 0;
	uint64_t cluster_size = 0;
	uint32_t cluster = 0;
	uint32_t max = 0;
	uint32_t seek_sector = 0;
	uint32_t seek_pos = 0;
	uint32_t count;

	// Short circuit cases where len is 0 (or less)
	if (len <= 0) {
		return 0;
	}
	
	if((len + file->pos )> file->finfo.file_size){
		len = file->finfo.file_size -  file->pos;
	}

	// Make sure we can actually read from the file
	if (file == NULL) {
		r->_errno = EBADF;
		return -1;
	}

	partition = file->partition;

	sector = partition->current_sector + partition->root_offset + partition->partition_start_offset;
	copied = 0;
	cluster_size = partition->clusters_size;
	cluster = file->finfo.starting_cluster;

	max = cluster_size;

	seek_sector = file->pos / XENON_DISK_SECTOR_SIZE;
	seek_pos = file->pos % XENON_DISK_SECTOR_SIZE;


	sector += seek_sector;

	if (data == NULL) {

		return -1;
	}
	if (len == 0) {
		return -1;
	}

	// Parse each dir entry
	while (len) {
		uint32_t n = ((max)<(len) ? (uint32_t) (max) : (len));
		
		xprintf("xtaf_read_file - offset : %lx\r\n", (sector * 0x200) + seek_pos);

		if(n == cluster_size && seek_pos == 0){
			bool read_err;			
			_XTAF_cache_readSectors(partition->cache, sector, partition->partition_hdr.sector_per_cluster,  data + copied );
			copied+=cluster_size;
		}
		else{
			// lazy way
			count = (n / XENON_DISK_SECTOR_SIZE) + 1;
			
			while (count) {
				uint32_t ssize = ((XENON_DISK_SECTOR_SIZE)<(n) ? (uint32_t) (XENON_DISK_SECTOR_SIZE) : (n));
				bool read_err;

				read_err = _XTAF_cache_readPartialSector(partition->cache, data + copied, sector, seek_pos, ssize);
				if (read_err == false) {
					printf("read err\r\n");
				}
				count--;
				sector++;
				copied += ssize;
				/* aligned */
				seek_pos = 0;
			}
		}


		// for next loop
		cluster = xfat_get_next_cluster(partition, cluster);
		sector = ((cluster - 1) * cluster_size) + partition->root_offset + partition->partition_start_offset;
		//sector = ((cluster - 1) * cluster_size) + partition->root_offset +  partition->partition_start_offset;

		len -= n;
		file->pos += n;

		xprintf("xtaf_read_file : n %llx\r\n", n);
	}

	xprintf("xtaf_read_file : red %llx\r\n", copied);

	return copied;
}

/** build a date time **/
static inline time_t xtaf_build_time(uint16_t date, uint32_t time) {
	return 0;
}

off_t xtaf_seek_r(struct _reent *r, int fd, off_t pos, int dir) {
	xtaf_file_private* file = (xtaf_file_private*) fd;
	off_t newPosition;

	if (file == NULL) {
		// invalid file
		r->_errno = EBADF;
		return -1;
	}

	switch (dir) {
		case SEEK_SET:
			newPosition = pos;
			break;
		case SEEK_CUR:
			newPosition = (off_t) file->pos + pos;
			break;
		case SEEK_END:
			newPosition = (off_t) file->pos + pos;
			break;
		default:
			r->_errno = EINVAL;
			return -1;
	}

	if ((pos > 0) && (newPosition < 0)) {
		r->_errno = EOVERFLOW;
		return -1;
	}

	// Save position
	file->pos = newPosition;

	return newPosition;
}

int xtaf_fstat_r(struct _reent *r, int fd, struct stat *st) {
	xtaf_file_private* file = (xtaf_file_private*) fd;
	xtaf_partition_private* partition = NULL;

	if (file == NULL) {
		// invalid file
		r->_errno = EBADF;
		return -1;
	}

	partition = file->partition;

	st->st_ino = (ino_t) (file->first_cluster); // The file serial number is the start cluster
	st->st_size = file->finfo.file_size;
	st->st_blksize = partition->clusters_size; // Prefered file I/O block size
	st->st_blocks = (st->st_size + partition->clusters_size - 1) / partition->clusters_size; // File size in blocks

	st->st_mode = file->finfo.flags;
	st->st_atime = xtaf_build_time(file->finfo.access_date, file->finfo.access_time);
	st->st_ctime = xtaf_build_time(file->finfo.creation_date, file->finfo.creation_time);
	st->st_mtime = xtaf_build_time(file->finfo.update_date, file->finfo.update_time);

	return 0;
}

/** parse a xtaf partition for a specified file **/
int xtaf_open_r(struct _reent *r, void * filestruct, const char *name, int flags, int mode) {
	// to move
	xtaf_file_private * file_private = (xtaf_file_private*) filestruct;
	//xtaf_partition_private* partition = file_private->partition;

	xtaf_partition_private* partition = getPartitionFromPath(name);

	char tmp_name[260];
	int err = -1;

	if (partition == NULL) {
		r->_errno = ENODEV;
		return -1;
	}

	// Move the path pointer to the start of the actual path
	if (strchr(name, ':') != NULL) {
		name = strchr(name, ':') + 1;
	}
	if (strchr(name, ':') != NULL) {
		r->_errno = EINVAL;
		return -1;
	}

	// Open the file for read-only access
	if ((flags & 0x03) == O_RDONLY) {

	} else {
		r->_errno = EACCES;
		return -1;
	}

	/*
		memset(&pCtx->finfo, 0, sizeof (struct _xtaf_directory_s));
		memset(pCtx->fat_name, 0, 0x2A);
	 */
	memset(file_private, 0, sizeof (xtaf_file_private));
	memset(&file_private->finfo, 0, sizeof (struct _xtaf_directory_s));

	file_private->partition = partition;

	// set this to 0
	partition->current_sector = 0;
	partition->extent_offset = 0;
	partition->extent_next_cluster = 0;
	partition->extent_offset = 0;

	strcpy(tmp_name, name);

	while (1) {
		err = xtaf_parse(file_private, tmp_name);
		if (err > 0) {
			switch (err) {
				case 2:
				{
					// folder found
					continue;
				}
				case 1:
				{
					// file found
					file_private->pos = 0;
					return file_private;
				}
			}
		} else {
			break;
		}
	}

	return -1;
};

int xtaf_close_r(struct _reent *r, int fd) {
	xtaf_file_private* file = (xtaf_file_private*) fd;
	int ret = 0;

	if (!file) {
		r->_errno = EBADF;
		return -1;
	}

	return ret;
}

int xtaf_stat_r(struct _reent *r, const char *path, struct stat *st) {
	xtaf_file_private* file = (xtaf_file_private*) malloc(sizeof (xtaf_file_private));

	int fd = xtaf_open_r(r, file, path, O_RDONLY, 0);
	if (fd) {
		xtaf_fstat_r(r, fd, st);

		xtaf_close_r(r, fd);
	}

	return 0;
}

/**
 * No supported yet func
 */
int xtaf_link_r(struct _reent *r, const char *existing, const char *newLink) {
	r->_errno = ENOTSUP;
	return -1;
}