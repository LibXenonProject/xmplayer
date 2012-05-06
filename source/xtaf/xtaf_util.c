#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/types.h>
#include <malloc.h>
#include "xtaf_util.h"

void XTAFError(char * format, ...) {
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf(buffer);
	va_end(args);
	system("PAUSE");
}


void* _XTAF_mem_allocate (size_t size) {
	return malloc (size);
}

void* _XTAF_mem_align (size_t size) {
	return memalign (32, size);
}

void _XTAF_mem_free (void* mem) {
	free (mem);
}