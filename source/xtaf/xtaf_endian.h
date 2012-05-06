/* 
 * File:   xtaf_endian.h
 * Author: cc
 *
 * Created on 3 mai 2012, 18:14
 */

#ifndef XTAF_ENDIAN_H
#define	XTAF_ENDIAN_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef XENON
	#undef LITTLE_ENDIAN
#else
	#define LITTLE_ENDIAN
#endif
	
	
static inline uint16_t le16(const uint8_t *p) {
    return p[1] | (p[0] << 8);
}

static inline uint32_t le32(const uint8_t *p) {
    return p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24);
}

static inline uint16_t be16(const uint8_t *p) {
    return p[0] | (p[1] << 8);
}

static inline uint32_t be32(const uint8_t *p) {
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

static inline uint32_t bswap32(uint32_t t) {
    return ((t & 0xFF) << 24) | ((t & 0xFF00) << 8) | ((t & 0xFF0000) >> 8) | ((t & 0xFF000000) >> 24);
}

static inline uint16_t bswap16(uint16_t x) {
    return ((x << 8) & 0xff00) | ((x >> 8) & 0x00ff);
}

static inline uint32_t host2be32(uint32_t val) {
#ifdef LITTLE_ENDIAN
    return bswap32(val);
#else
    return val;
#endif
}

static inline uint16_t host2be16(uint16_t val) {
#ifdef LITTLE_ENDIAN
    return bswap32(val);
#else
    return val;
#endif
}

static inline uint32_t read32be(uint32_t val) {
#ifdef LITTLE_ENDIAN
    return bswap32(val);
#else
    return val;
#endif
}

static inline uint16_t read16be(uint16_t val) {
#ifdef LITTLE_ENDIAN
    return bswap16(val);
#else
    return val;
#endif
}

#ifdef	__cplusplus
}
#endif

#endif	/* XTAF_ENDIAN_H */

