#ifndef H_ATA
#define H_ATA


#ifdef WIN32
#define __attribute__(x)
#endif

#define XENON_ATA_REG_DATA      0
#define XENON_ATA_REG_ERROR     1
#define XENON_ATA_REG_FEATURES  1
#define XENON_ATA_REG_SECTORS   2
#define XENON_ATA_REG_SECTNUM   3
#define XENON_ATA_REG_CYLLSB    4
#define XENON_ATA_REG_CYLMSB    5
#define XENON_ATA_REG_LBALOW    3
#define XENON_ATA_REG_LBAMID    4
#define XENON_ATA_REG_LBAHIGH   5
#define XENON_ATA_REG_DISK      6
#define XENON_ATA_REG_CMD       7
#define XENON_ATA_REG_STATUS    7

#define XENON_ATA_REG2_CONTROL   0



enum xenon_ata_commands
  {
    XENON_ATA_CMD_READ_SECTORS = 0x20,
    XENON_ATA_CMD_READ_SECTORS_EXT = 0x24,
    XENON_ATA_CMD_WRITE_SECTORS = 0x30,
    XENON_ATA_CMD_WRITE_SECTORS_EXT = 0x34,
    XENON_ATA_CMD_IDENTIFY_DEVICE = 0xEC,
    XENON_ATA_CMD_IDENTIFY_PACKET_DEVICE = 0xA1,
    XENON_ATA_CMD_PACKET = 0xA0
  };

enum
  {
    XENON_ATA_CHS = 0,
    XENON_ATA_LBA,
    XENON_ATA_LBA48
  };

#define XENON_DISK_SECTOR_SIZE 0x200
#define XENON_CDROM_SECTOR_SIZE 2048

//#include <diskio/diskio.h>

struct xenon_ata_device {
    uint32_t ioaddress;
    uint32_t ioaddress2;

    int atapi;

    int addressing_mode;

    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors_per_track;

    uint32_t size;
    struct bdev *bdev;
};

struct xenon_atapi_read
{
  uint8_t code;
  uint8_t reserved1;
  uint32_t lba;
  uint32_t length;
  uint8_t reserved2[2];
} __attribute__((packed));


int xenon_ata_init();
int xenon_atapi_init();

#endif
