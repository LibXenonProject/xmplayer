#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/reent.h>
#include <diskio/disc_io.h>
#include <sys/iosupport.h>

#include <debug.h>
#include "fakeio.h"
#include "xtaf.h"
#include "ata.h"

#include "xtaf_util.h"
#include "xtaf_endian.h"

static const devoptab_t dotab_xtaf = {
	"xtaf",
	sizeof (xtaf_file_private),
	xtaf_open_r,
	xtaf_close_r,
	NULL, // write_r
	xtaf_read_r,
	xtaf_seek_r,
	xtaf_fstat_r,
	xtaf_stat_r,
	NULL, // link_r
	NULL, // unlink_r
	NULL, // chdir_r
	NULL, // rename_r
	NULL, // mkdir_r
	sizeof (xtaf_dir_entry), // dirStateSize
	xtaf_diropen_r, // diropen_r
	NULL, // dirreset_r
	xtaf_dirnext_r, // dirnext_r
	xtaf_dirclose_r, // dirclose_r
	NULL, // statvfs_r
	NULL, // ftruncate_r
	NULL, // fsync_r
	NULL, // deviceData
	NULL, // chmod_r
	NULL  // fchmod_r
};

xtaf_partition_table partition_table[3] = {
	{"hdd0", 0x120eb0000, 0x10000000}, // Xbox 1 Backwards Compatibility
	{"hdd1", 0x130eb0000, 0}, // 360
	{"null", 0, 0}// End
};

//extern xtaf_context *pCtx;

/** return the size of the file alocation table **/
static uint32_t getFatSize(xtaf_partition_private *priv) {
	struct _xtaf_partition_hdr_s * part_hdr = &priv->partition_hdr;

	uint64_t file_system_size = priv->file_system_size;
	uint32_t spc = part_hdr->sector_per_cluster; /* sectors per cluster */

	uint32_t numclusters = (uint32_t) (file_system_size / spc);
	uint8_t fatmult = numclusters >= 0xfff4 ? sizeof (uint32_t) : sizeof (uint16_t);
	uint32_t fatsize = numclusters * fatmult;

	if (fatsize % 4096 != 0)
		fatsize = (((fatsize / 4096) + 1) * 4096); /* round up if not multiple of 4 kB */

	return fatsize / XENON_DISK_SECTOR_SIZE;
}

void print_partition_information(struct xtaf_partition_private *partition) {
	char magic[5];
	memcpy(magic, (const char*) partition->partition_hdr.magic, 4);
	magic[4] = 0;

	printf("\n\nPartition Information :\n");
	printf("fat_file_size       =	%08x\n", partition->fat_file_size * XENON_DISK_SECTOR_SIZE);

	printf("magic               =	%s\n", magic);
	printf("id                  =	%08x\n", partition->partition_hdr.id);
	printf("sector_per_cluster  =	%08x\n", partition->partition_hdr.sector_per_cluster);

	printf("fat_offset          =	%16lx\n", (partition->fat_offset + partition->partition_start_offset) * XENON_DISK_SECTOR_SIZE);
	printf("root_cluster        =	%08x\n", partition->partition_hdr.root_cluster);
	printf("clusters_size       =	%08x\n", partition->clusters_size);
	printf("root_offset         =	%16lx\n", (partition->root_offset + partition->partition_start_offset) * XENON_DISK_SECTOR_SIZE);
	
	
	printf("fat_offset s         =	%8x\n", (partition->fat_offset + partition->partition_start_offset) );
	printf("root_offset s       =%8x\n", (partition->root_offset + partition->partition_start_offset));
}

int xtaf_init_fs(struct xtaf_partition_private *part_info) {
	struct _xtaf_partition_hdr_s * part_hdr = &part_info->partition_hdr;

	part_info->fat_file_size = getFatSize(part_info);
	part_info->clusters_size = part_hdr->sector_per_cluster * XENON_DISK_SECTOR_SIZE;
	part_info->fat_offset = 0x1000 / XENON_DISK_SECTOR_SIZE;
	part_info->root_offset = part_info->fat_offset + part_info->fat_file_size;
	part_info->extent_offset = 0;
	part_info->current_sector = 0;

	print_partition_information(part_info);

	return 0;
}

/** return 0 if hdd is retail, 1 if is hdd is devkit **/
int xtaf_check_hdd_type() {
	// read start of hdd
	/*
		if (raw_read(pCtx, 0)) {
			uint32_t * hdd_hdr = (uint32_t *) raw_buf;
			if (hdd_hdr[0] == 0) {
				return 0;
			}
			return 1;
		}
		//failed
		return -1;
	 */
	return 0;
}

xtaf_partition_private * xtaf_mount(void * disc, uint32_t start_sector, uint32_t num_sectors, uint32_t cacheSize, uint32_t sectorsPerPage, uint8_t *sectorBuffer ) {
	bool err;
	static int partition_nbr = 0;
	devoptab_t* devops;
	char name[]="xf0";	
	name[2] = partition_nbr+'0';

	xtaf_partition_private * partition = (xtaf_partition_private*)_XTAF_mem_allocate(sizeof(xtaf_partition_private));

	partition->disc = disc;

	partition->partition_number = partition_nbr;

	partition->partition_start_offset = start_sector;

	partition->file_system_size = num_sectors;

	// Read the header
	//err = ioread_sector(partition, sectorBuffer, 0, 1);
	
	err = _XTAF_disc_readSectors(disc,start_sector,1,sectorBuffer);

	if (err==false) {
		_XTAF_mem_free(partition);
		return NULL;
	}

	if (memcmp(sectorBuffer, "XTAF", 4) == 0) {
		printf("found a parition at %08x\n",start_sector);
		
		struct _xtaf_partition_hdr_s * p = (struct _xtaf_partition_hdr_s*) sectorBuffer;

		// Bswap partition header (for little endian cpu)
		memcpy(partition->partition_hdr.magic, p->magic, 4);
		partition->partition_hdr.id = host2be32(p->id);
		partition->partition_hdr.root_cluster = host2be32(p->root_cluster);
		partition->partition_hdr.sector_per_cluster = host2be32(p->sector_per_cluster);
		partition->partition_name = partition_table[partition_nbr].name;
		partition->found = 1;
		//return xtaf_init_fs(priv);
		xtaf_init_fs(partition);
	} else {
		_XTAF_mem_free(partition);
		return NULL;
	}

	partition->cache = _XTAF_cache_constructor(cacheSize, sectorsPerPage, partition->disc, start_sector + num_sectors, 0x200);

	partition_nbr++;
	

	devops = _XTAF_mem_allocate (sizeof(devoptab_t) + strlen(name) + 1);
	
	// Use the space allocated at the end of the devoptab struct for storing the name
	char * nameCopy = (char*)(devops+1);
	
	// Add an entry for this device to the devoptab table
	memcpy (devops, &dotab_xtaf, sizeof(dotab_xtaf));
	strcpy (nameCopy, name);
	devops->name = nameCopy;
	devops->deviceData = partition;

	err =  AddDevice (devops);
	
	printf("err = %d\r\n",err);

	return partition;
}

/** xtaf_init init sata, and look for xtaf partition **/
int xtaf_init(struct xtaf_context *ctx, DISC_INTERFACE * disc) {
	/*
		if (pCtx == NULL) {
			pCtx = ctx;
		}
	 */
	ctx->dev = disc;
	if (disc == NULL) {
		xprintf("disc is nulll\r\n\r\n");
	}
	
	// start disc
	disc->startup();

	int err;
	int partition_nbr = 0;

	int is_devkit_hdd = xtaf_check_hdd_type();

	if (is_devkit_hdd == 1) {
		printf("Devkit hdd not supported\r\n");
		return -1;
	}

	int found = 0;

	uint8_t *sectorBuffer = (uint8_t*) _XTAF_mem_allocate(XENON_DISK_SECTOR_SIZE);

	// use only 1 parition for now ...
	while (1) {
		uint32_t start_sector, num_sectors;

		start_sector = partition_table[partition_nbr].offset / XENON_DISK_SECTOR_SIZE;

		if (partition_table[partition_nbr].length == 0) {
#ifdef XENON
			//num_sectors = (uint32_t) dev->size - start_sector;
			//num_sectors = (uint32_t)  (312581808) - start_sector;
			extern struct xenon_ata_device ata;
			num_sectors = (uint32_t)  (ata.size) - start_sector;
#else
			num_sectors = (20003880960 / XENON_DISK_SECTOR_SIZE) - start_sector; // hdd_dump file size
#endif
		} else {
			num_sectors = (partition_table[partition_nbr].length) / XENON_DISK_SECTOR_SIZE;
		}

		xtaf_partition_private * partition =xtaf_mount(disc,start_sector, num_sectors, DEFAULT_CACHE_PAGES, DEFAULT_SECTORS_PAGE,sectorBuffer);

		if(partition==NULL){
			//error
		}
		else{
			// OK
			found++;
		}

		ctx->priv[partition_nbr] = partition;

		partition_nbr++;
		if (partition_table[partition_nbr].offset == 0)
			break;
	}

	_XTAF_mem_free(sectorBuffer);

	return found;
}
