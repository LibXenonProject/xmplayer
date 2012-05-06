/*
 bit_ops.h
 Functions for dealing with conversion of data between types

 Copyright (c) 2006 Michael "Chishm" Chisholm
	
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _BIT_OPS_H
#define _BIT_OPS_H

#include <stdint.h>

#if 0 // BIG ENDIAN (XBOX 360)

/*-----------------------------------------------------------------
Functions to deal with big endian values stored in uint8_t arrays
-----------------------------------------------------------------*/
static inline uint16_t u8array_to_u16 (const uint8_t* item, int offset) {
	return ( item[offset] | (item[offset + 1] << 8));
}

static inline uint32_t u8array_to_u32 (const uint8_t* item, int offset) {
	return ( item[offset] | (item[offset + 1] << 8) | (item[offset + 2] << 16) | (item[offset + 3] << 24));
}

static inline void u16_to_u8array (uint8_t* item, int offset, uint16_t value) {
	item[offset]     = (uint8_t) value;
	item[offset + 1] = (uint8_t)(value >> 8);
}

static inline void u32_to_u8array (uint8_t* item, int offset, uint32_t value) {
	item[offset]     = (uint8_t) value;
	item[offset + 1] = (uint8_t)(value >> 8);
	item[offset + 2] = (uint8_t)(value >> 16);
	item[offset + 3] = (uint8_t)(value >> 24);
}
#else // LIL ENDIAN (PC)
/*-----------------------------------------------------------------
Functions to deal with big endian values stored in uint8_t arrays
-----------------------------------------------------------------*/
static inline uint16_t u8array_to_u16 (const uint8_t* item, int offset) {
	return ( item[offset+1] | (item[offset] << 8));
}

static inline uint32_t u8array_to_u32 (const uint8_t* item, int offset) {
	return ( item[offset+3] | (item[offset + 2] << 8) | (item[offset + 1] << 16) | (item[offset] << 24));
}

static inline void u16_to_u8array (uint8_t* item, int offset, uint16_t value) {
	item[offset + 1]     = (uint8_t) value;
	item[offset + 0] = (uint8_t)(value >> 8);
}

static inline void u32_to_u8array (uint8_t* item, int offset, uint32_t value) {
	item[offset + 3]     = (uint8_t) value;
	item[offset + 2] = (uint8_t)(value >> 8);
	item[offset + 1] = (uint8_t)(value >> 16);
	item[offset + 0] = (uint8_t)(value >> 24);
}
#endif

#endif // _BIT_OPS_H
