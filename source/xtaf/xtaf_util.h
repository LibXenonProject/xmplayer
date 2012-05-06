/* 
 * File:   xtaf_util.h
 * Author: cc
 *
 * Created on 4 mai 2012, 09:50
 */

#ifndef XTAF_UTIL_H
#define	XTAF_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

void XTAFError(char * format, ...);

void* _XTAF_mem_allocate (size_t size);
void* _XTAF_mem_align (size_t size);
void _XTAF_mem_free (void* mem);

#ifdef	__cplusplus
}
#endif

#endif	/* XTAF_UTIL_H */

