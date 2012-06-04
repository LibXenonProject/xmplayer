#pragma once

#include <diskio/disc_io.h>
#include "cache.h"

#ifdef XTAF_DEBUG
#define xprintf(...) printf(__VA_ARGS__)
#else
#define	xprintf(...)
#endif

#define XTAF_DIR_FLAGS 0x10

//#define xprintf(...) printf(__VA_ARGS__)

#define DEFAULT_CACHE_PAGES 8
#define DEFAULT_SECTORS_PAGE 128

#define MAX_PARTITION_PER_HDD 4

//#define CLUSTER_EOF 0xFFF

/**
 * Partition header
 **/
struct _xtaf_partition_hdr_s {
	unsigned char magic[4]; //32
	uint32_t id; //64
	uint32_t sector_per_cluster; //96
	uint32_t root_cluster; //128
};

struct _x_hdd_security_sector_s {
	unsigned char serial_number[0x14];
	unsigned char firmware_revision[0x8];
	unsigned char model_number[0x28];
	unsigned char ms_logo_hash[0x14];
	uint32_t number_of_sector;
	unsigned char signature[0x100];
	uint32_t ms_logo_size;
	void * ms_logo;
};

/**
 * The file flags and the date and time fields are in the same format as the one used in the FAT file system.
 * For time, bits 15-11 represent the hour, 10-5 the minutes, 4-0 the seconds.
 * For date, bits 15-9 are the year, 8-5 are the month, 4-0 are the day.
 **/
struct _xtaf_directory_s {
	unsigned char filename_size;
	unsigned char flags;
	unsigned char filename[0x2A]; //padded with either 0x00 or 0xFF bytes
	uint32_t starting_cluster; //0 for empty files
	uint32_t file_size; //0 for directories
	uint16_t creation_date;
	uint16_t creation_time;
	uint16_t access_date;
	uint16_t access_time;
	uint16_t update_date;
	uint16_t update_time;
};

typedef struct {
	uint32_t cluster;
	sec_t    sector;
	int32_t  offset;
} DIR_ENTRY_POSITION;

typedef struct {
	uint32_t   cluster;
	sec_t sector;
	int32_t   byte;
} FILE_POSITION;

typedef struct xtaf_entry{
	DIR_ENTRY_POSITION dataStart;
	// @todo remove it...
	struct _xtaf_directory_s _xtaf_dir;
} xtaf_entry;

/**
 * stat
 **/
typedef struct xtaf_stat {
	uint32_t mode; /* Protection */
	uint32_t size; /* Taille totale en octets */
	uint32_t atime; /* Heure dernier acces */
	uint32_t mtime; /* Heure derniere modification */
	uint32_t ctime; /* Heure dernier changement etat */
} xtaf_stat;

/**
 * per partition information
 */
typedef struct xtaf_partition_private {
	/* which partition */
	uint8_t partitionNumber;

	uint64_t partition_start_offset;
	sec_t dataStart; // ??
	sec_t rootDirStart;// same ??
	
	sec_t fatStart;
	
	uint32_t root_entries;
	uint32_t clusters;

	uint64_t fat_offset;
	uint64_t root_offset;
	uint64_t data_offset;
	uint32_t fat_file_size;

	uint32_t extent_offset;
	uint32_t extent_len;
	uint32_t extent_next_cluster;

	uint32_t numberOfSectors;
	uint32_t sectorsPerCluster;
	uint32_t bytesPerCluster;
	uint32_t bytesPerSector;
	uint32_t rootDirCluster;
	
	uint32_t cwdCluster;
	
	
	
	unsigned char magic[4]; //32
	
	uint32_t partitionId;
	
	uint32_t current_sector;

	char * partitionName;
	
	const DISC_INTERFACE* disc;
	CACHE*                cache;

	/**  **/
	uint8_t found;
	
} xtaf_partition_private;


typedef struct{
	struct _xtaf_directory_s entryInfo;
	
	/* first cluster */
	uint32_t startCluster;
	uint32_t filesize;
	
	/* which partition */
	xtaf_partition_private * partition;
} xtaf_dir_entry;

/**
 * per file information
 */
typedef struct xtaf_file_private {
	/* which partition */
	xtaf_partition_private * partition;

	/* first cluster */
	uint32_t startCluster;
	uint32_t filesize;

	/* position in stream */
	uint64_t currentPosition;

	/* information from hdd */
	FILE_POSITION        rwPosition;
	FILE_POSITION        appendPosition;
	
	struct _xtaf_directory_s entryInfo;

	/* busy flags*/
	uint8_t busy;
} xtaf_file_private;

/**
 * static in xtaf.c
 */
typedef struct xtaf_context {
	/** xenon ata things **/
	void*dev;

	/** used by xtaf_open **/
	uint8_t fat_name[0x2A + 1];

	/** xx partition  **/
	xtaf_partition_private * priv[MAX_PARTITION_PER_HDD];
	struct _xtaf_directory_s finfo;

} xtaf_context;

typedef struct xtaf_partition_table {
	char name[8];
	uint64_t offset;
	uint64_t length;
} xtaf_partition_table;

int xtaf_check_filename(const char *s1, const char *s2, size_t n);

extern xtaf_partition_table partition_table[3];

#define RAW_BUF 0x200
#define MAX_SECTS 8

/** xtaf_init init sata, and look for xtaf partition **/
int xtaf_init(struct xtaf_context *ctx, DISC_INTERFACE *disc);
/** free memory **/
int xtaf_shutdown(struct xtaf_context *ctx);

int hdd_is_xtaf(void *dev);

/***/
int xtaf_parse(xtaf_file_private * file_private, char *name);
uint32_t xfat_get_next_cluster(xtaf_partition_private *priv, uint32_t cluster_id);



off_t xtaf_seek_r(struct _reent *r, int fd, off_t pos, int dir) ;
int xtaf_fstat_r(struct _reent *r, int fd, struct stat *st);
int xtaf_open_r(struct _reent *r, void * files, const char *name, int flags, int mode) ;
ssize_t xtaf_read_r (struct _reent *r, int fd, char *data, size_t len);
ssize_t _xtaf_read_r(struct _reent *r, int fd, unsigned char *data, size_t len);
int xtaf_close_r (struct _reent *r, int fd);
int xtaf_stat_r(struct _reent *r, const char *path, struct stat *st);

int xtaf_directory_entryFromPath(xtaf_partition_private* partition, struct _xtaf_directory_s * entry,const char* path, const char* pathEnd) ;

DIR_ITER* xtaf_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path);
int xtaf_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st) ;
int xtaf_dirclose_r(struct _reent *r, DIR_ITER *dirState);
int xtaf_parse_entry(xtaf_partition_private * priv, struct _xtaf_directory_s * data);


int XTATMount();