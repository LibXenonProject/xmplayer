#ifndef XENON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define NAME_MAX 45
#include <sys/iosupport.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <diskio/disc_io.h>

static int defaultDevice = -1;

#define MAXPATHLEN 2048
//---------------------------------------------------------------------------------
const devoptab_t dotab_stdnull = {
	//---------------------------------------------------------------------------------
	"stdnull", // device name
	0, // size of file structure
	NULL, // device open
	NULL, // device close
	NULL, // device write
	NULL, // device read
	NULL, // device seek
	NULL, // device fstat
	NULL, // device stat
	NULL, // device link
	NULL, // device unlink
	NULL, // device chdir
	NULL, // device rename
	NULL, // device mkdir
	0, // dirStateSize
	NULL, // device diropen_r
	NULL, // device dirreset_r
	NULL, // device dirnext_r
	NULL, // device dirclose_r
	NULL, // device statvfs_r
	NULL, // device ftruncate_r
	NULL, // device fsync_r
	NULL // deviceData
};

//---------------------------------------------------------------------------------
const devoptab_t *devoptab_list[STD_MAX] = {
	//---------------------------------------------------------------------------------
	//&dotab_stdnull, &dotab_stdnull, &dotab_stdnull, &dotab_stdnull,
	&dotab_stdnull, &dotab_stdnull, &dotab_stdnull, &dotab_stdnull,
	&dotab_stdnull, &dotab_stdnull, &dotab_stdnull, &dotab_stdnull,
	&dotab_stdnull, &dotab_stdnull, &dotab_stdnull, &dotab_stdnull,
	&dotab_stdnull, &dotab_stdnull, &dotab_stdnull, &dotab_stdnull
};

//---------------------------------------------------------------------------------

int FindDevice(const char* name) {
	//---------------------------------------------------------------------------------
	int i = 0, namelen, dev = -1;

	if (strchr(name, ':') == NULL) return defaultDevice;

	while (i < STD_MAX) {
		if (devoptab_list[i]) {
			namelen = strlen(devoptab_list[i]->name);
			if (strncmp(devoptab_list[i]->name, name, namelen) == 0) {
				if (name[namelen] == ':' || (isdigit(name[namelen]) && name[namelen + 1] == ':')) {
					dev = i;
					break;
				}
			}
		}
		i++;
	}

	return dev;
}

//---------------------------------------------------------------------------------

int RemoveDevice(const char* name) {
	//---------------------------------------------------------------------------------
	int dev = FindDevice(name);

	if (-1 != dev) {
		devoptab_list[dev] = &dotab_stdnull;
		return 0;
	}

	return -1;

}

//---------------------------------------------------------------------------------

int AddDevice(const devoptab_t* device) {
	//---------------------------------------------------------------------------------
	printf("AddDevice : %s\n", device->name);
	int devnum;

	for (devnum = 3; devnum < STD_MAX; devnum++) {

		if ((!strcmp(devoptab_list[devnum]->name, device->name) &&
				strlen(devoptab_list[devnum]->name) == strlen(device->name)) ||
				!strcmp(devoptab_list[devnum]->name, "stdnull")
				)
			break;
	}

	if (devnum == STD_MAX) {
		devnum = -1;
	} else {
		devoptab_list[devnum] = device;
	}
	return devnum;
}

//---------------------------------------------------------------------------------

const devoptab_t* GetDeviceOpTab(const char *name) {
	//---------------------------------------------------------------------------------
	int dev = FindDevice(name);
	if (dev >= 0 && dev < STD_MAX) {
		return devoptab_list[dev];
	} else {
		return NULL;
	}
}

#include <stdio.h>
#include <fcntl.h>

#define XENON_DISK_SECTOR_SIZE 0x200

static FILE * fd = NULL;

bool FILE_FN_MEDIUM_STARTUP(void) {
	fd = fopen("/home/cc/xenon/xbox_hd_dump.img", "rb");
	return true;
};

bool FILE_FN_MEDIUM_ISINSERTED(void) {
	return true;
};

bool FILE_FN_MEDIUM_READSECTORS(sec_t sector, sec_t numSectors, void* buffer) {
	uint64_t sec64 = (uint64_t) sector;
	uint64_t offset = (uint64_t) sec64*XENON_DISK_SECTOR_SIZE;

	fseek(fd, offset, SEEK_SET);
	fread(buffer, 1, numSectors*XENON_DISK_SECTOR_SIZE, fd);
	printf("reading %d sectors at %08x\r\n", numSectors, sector);
	printf("offset : %16lx\r\n", offset);
	printf("disk size : %4x\r\n", XENON_DISK_SECTOR_SIZE);
	return true;
};

bool FILE_FN_MEDIUM_WRITESECTORS(sec_t sector, sec_t numSectors, const void* buffer) {
	return false;
};

bool FILE_FN_MEDIUM_CLEARSTATUS(void) {
	return true;
};

bool FILE_FN_MEDIUM_SHUTDOWN(void) {
	fclose(fd);
	return true;
};

const struct DISC_INTERFACE_STRUCT file_ops = {
	FEATURE_XENON_ATAPI, FEATURE_MEDIUM_CANREAD | FEATURE_XENON_ATAPI,
	FILE_FN_MEDIUM_STARTUP, FILE_FN_MEDIUM_ISINSERTED, FILE_FN_MEDIUM_READSECTORS,
	FILE_FN_MEDIUM_WRITESECTORS, FILE_FN_MEDIUM_CLEARSTATUS, FILE_FN_MEDIUM_SHUTDOWN
};
;

const DISC_INTERFACE* get_io_ata(void) {
	return &file_ops;
}

/*
const INTERFACE_ID _FAT_disc_interfaces[] = {
	{"linux", &get_io_ata},
	{NULL, NULL}
};
*/

#endif