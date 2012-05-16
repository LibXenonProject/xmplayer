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

#define FILE_MAX_SIZE 4294967296
#define CLUSTER_END 0xFFFFFFF8

static inline uint32_t XTAF_clusterToSector(xtaf_partition_private* partition, uint32_t cluster) {
	return ((cluster - 1) * partition->bytesPerCluster) + partition->root_offset + partition->partition_start_offset;
}

static inline sec_t _clusterToSector(xtaf_partition_private* partition, uint32_t cluster) {
	return ((cluster - 1) * (sec_t)partition->sectorsPerCluster) + partition->dataStart;
}

static xtaf_partition_private* getPartitionFromPath(const char* path) {
	const devoptab_t *devops;

	devops = GetDeviceOpTab(path);

	if (!devops) {
		return NULL;
	}

	return (xtaf_partition_private*) devops->deviceData;
}

/** build a date time **/
static inline time_t xtaf_build_time(uint16_t date, uint32_t time) {
	return 0;
}

off_t xtaf_seek_r (struct _reent *r, int fd, off_t pos, int dir) {
	xtaf_file_private* file = (xtaf_file_private*)  fd;
	xtaf_partition_private* partition;
	uint32_t cluster, nextCluster;
	int clusCount;
	off_t newPosition;
	uint32_t position;

	if (file == NULL)	 {
		// invalid file
		r->_errno = EBADF;
		return -1;
	}

	partition = file->partition;

	switch (dir) {
		case SEEK_SET:
			newPosition = pos;
			break;
		case SEEK_CUR:
			newPosition = (off_t)file->currentPosition + pos;
			break;
		case SEEK_END:
			newPosition = (off_t)file->filesize + pos;
			break;
		default:
			r->_errno = EINVAL;
			return -1;
	}

	if ((pos > 0) && (newPosition < 0)) {
		r->_errno = EOVERFLOW;
		return -1;
	}

	// newPosition can only be larger than the FILE_MAX_SIZE on platforms where
	// off_t is larger than 32 bits.
	if (newPosition < 0 || ((sizeof(newPosition) > 4) && newPosition > (off_t)FILE_MAX_SIZE)) {
		r->_errno = EINVAL;
		return -1;
	}

	position = (uint32_t)newPosition;

	// Only change the read/write position if it is within the bounds of the current filesize,
	// or at the very edge of the file
	if (position <= file->filesize) {
		// Calculate where the correct cluster is
		// how many clusters from start of file
		clusCount = position / partition->bytesPerCluster;
		cluster = file->startCluster;
		if (position >= file->currentPosition) {
			// start from current cluster
			int currentCount = file->currentPosition / partition->bytesPerCluster;
			if (file->rwPosition.sector == partition->sectorsPerCluster) {
				currentCount--;
			}
			clusCount -= currentCount;
			cluster = file->rwPosition.cluster;
		}
		// Calculate the sector and byte of the current position,
		// and store them
		file->rwPosition.sector = (position % partition->bytesPerCluster) / partition->bytesPerSector;
		file->rwPosition.byte = position % partition->bytesPerSector;

		nextCluster = xfat_get_next_cluster (partition, cluster);
		while ((clusCount > 0) && (nextCluster != CLUSTER_END)) {
			clusCount--;
			cluster = nextCluster;
			nextCluster = xfat_get_next_cluster (partition, cluster);
		}

		// Check if ran out of clusters and it needs to allocate a new one
		if (clusCount > 0) {
			if ((clusCount == 1) && (file->filesize == position) && (file->rwPosition.sector == 0)) {
				// Set flag to allocate a new cluster
				file->rwPosition.sector = partition->sectorsPerCluster;
				file->rwPosition.byte = 0;
			} else {
				r->_errno = EINVAL;
				return -1;
			}
		}

		file->rwPosition.cluster = cluster;
	}

	// Save position
	file->currentPosition = position;

	return position;
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

	st->st_ino = (ino_t) (file->startCluster); // The file serial number is the start cluster
	st->st_size = file->filesize;
	st->st_blksize = partition->bytesPerCluster; // Prefered file I/O block size
	st->st_blocks = (st->st_size + partition->bytesPerCluster - 1) / partition->bytesPerCluster; // File size in blocks

	st->st_mode = file->entryInfo.flags;
	st->st_atime = xtaf_build_time(file->entryInfo.access_date, file->entryInfo.access_time);
	st->st_ctime = xtaf_build_time(file->entryInfo.creation_date, file->entryInfo.creation_time);
	st->st_mtime = xtaf_build_time(file->entryInfo.update_date, file->entryInfo.update_time);

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
	memset(&file_private->entryInfo, 0, sizeof (struct _xtaf_directory_s));

	file_private->partition = partition;

	// set this to 0
	partition->current_sector = 0;
	partition->extent_offset = 0;
	partition->extent_next_cluster = 0;
	partition->extent_offset = 0;

	strcpy(tmp_name, name);

		//err = xtaf_parse(file_private, tmp_name);
	
	err = xtaf_directory_entryFromPath(partition,&file_private->entryInfo,name,NULL);
	if(err==1){
		if(file_private->entryInfo.flags & XTAF_DIR_FLAGS){
			r->_errno = EISDIR;
			return -1;
		}
		
		file_private->filesize = file_private->entryInfo.file_size;
		file_private->startCluster = file_private->entryInfo.starting_cluster;
		
		file_private->currentPosition = 0;
		file_private->rwPosition.cluster = file_private->startCluster;
		
		return (int)file_private;
	}
		
	r->_errno = ENOENT;
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

ssize_t xtaf_read_r (struct _reent *r, int fd, char *ptr, size_t len) {
	xtaf_file_private* file = (xtaf_file_private*)  fd;
	xtaf_partition_private* partition;
	CACHE* cache;
	FILE_POSITION position;
	uint32_t tempNextCluster;
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

	// Don't try to read if the read pointer is past the end of file
	if (file->currentPosition >= file->filesize) {
		r->_errno = EOVERFLOW;
		return 0;
	}

	// Don't read past end of file
	if (len + file->currentPosition > file->filesize) {
		r->_errno = EOVERFLOW;
		len = file->filesize - file->currentPosition;
	}

	remain = len;
	position = file->rwPosition;
	cache = file->partition->cache;

	// Align to sector
	tempVar = partition->bytesPerSector - position.byte;
	if (tempVar > remain) {
		tempVar = remain;
	}

	if ((tempVar < partition->bytesPerSector) && flagNoError)
	{
		_XTAF_cache_readPartialSector ( cache, ptr, _clusterToSector (partition, position.cluster) + position.sector,
			position.byte, tempVar);

		remain -= tempVar;
		ptr += tempVar;

		position.byte += tempVar;
		if (position.byte >= partition->bytesPerSector) {
			position.byte = 0;
			position.sector++;
		}
	}

	// align to cluster
	// tempVar is number of sectors to read
	if (remain > (partition->sectorsPerCluster - position.sector) * partition->bytesPerSector) {
		tempVar = partition->sectorsPerCluster - position.sector;
	} else {
		tempVar = remain / partition->bytesPerSector;
	}

	if ((tempVar > 0) && flagNoError) {
		if (! _XTAF_cache_readSectors (cache, _clusterToSector (partition, position.cluster) + position.sector,
			tempVar, ptr))
		{
			flagNoError = false;
			r->_errno = EIO;
		} else {
			ptr += tempVar * partition->bytesPerSector;
			remain -= tempVar * partition->bytesPerSector;
			position.sector += tempVar;
		}
	}

	// Move onto next cluster
	// It should get to here without reading anything if a cluster is due to be allocated
	if ((position.sector >= partition->sectorsPerCluster) && flagNoError) {
		tempNextCluster = xfat_get_next_cluster(partition, position.cluster);

		if ((remain == 0) && (tempNextCluster == CLUSTER_END)) {
			position.sector = partition->sectorsPerCluster;
		} 
/*
		else if (!_FAT_fat_isValidCluster(partition, tempNextCluster)) {
			r->_errno = EIO;
			flagNoError = false;
		} 
*/
		else
		{
			position.sector = 0;
			position.cluster = tempNextCluster;
		}
	}

	// Read in whole clusters, contiguous blocks at a time
	while ((remain >= partition->bytesPerCluster) && flagNoError) {
		uint32_t chunkEnd;
		uint32_t nextChunkStart = position.cluster;
		size_t chunkSize = 0;

		do {
			chunkEnd = nextChunkStart;
			nextChunkStart = xfat_get_next_cluster (partition, chunkEnd);
			chunkSize += partition->bytesPerCluster;
		} while ((nextChunkStart == chunkEnd + 1) &&
#ifdef LIMIT_SECTORS
		 	(chunkSize + partition->bytesPerCluster <= LIMIT_SECTORS * partition->bytesPerSector) &&
#endif
			(chunkSize + partition->bytesPerCluster <= remain));

		if (!_XTAF_cache_readSectors (cache, _clusterToSector (partition, position.cluster),
				chunkSize / partition->bytesPerSector, ptr))
		{
			flagNoError = false;
			r->_errno = EIO;
			break;
		}
		ptr += chunkSize;
		remain -= chunkSize;

		// Advance to next cluster
		if ((remain == 0) && (nextChunkStart == CLUSTER_END)) {
			position.sector = partition->sectorsPerCluster;
			position.cluster = chunkEnd;
		} 
/*
		else if (!_FAT_fat_isValidCluster(partition, nextChunkStart)) {
			r->_errno = EIO;
			flagNoError = false;
		}
*/
		else 
		{
			position.sector = 0;
			position.cluster = nextChunkStart;
		}
	}

	// Read remaining sectors
	tempVar = remain / partition->bytesPerSector; // Number of sectors left
	if ((tempVar > 0) && flagNoError) {
		if (!_XTAF_cache_readSectors (cache, _clusterToSector (partition, position.cluster),
			tempVar, ptr))
		{
			flagNoError = false;
			r->_errno = EIO;
		} else {
			ptr += tempVar * partition->bytesPerSector;
			remain -= tempVar * partition->bytesPerSector;
			position.sector += tempVar;
		}
	}

	// Last remaining sector
	// Check if anything is left
	if ((remain > 0) && flagNoError) {
		_XTAF_cache_readPartialSector ( cache, ptr,
			_clusterToSector (partition, position.cluster) + position.sector, 0, remain);
		position.byte += remain;
		remain = 0;
	}

	// Length read is the wanted length minus the stuff not read
	len = len - remain;

	// Update file information
	file->rwPosition = position;
	file->currentPosition += len;

	return len;
}

