/* 
 * File:   gui_debug.h
 * Author: cc
 *
 * Created on 25 décembre 2011, 20:26
 */

#ifndef GUI_DEBUG_H
#define	GUI_DEBUG_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <xenon_uart/xenon_uart.h>

#define BP {printf("[Breakpoint] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);getch();}
#define TR {printf("[Trace] in function %s, line %d, file %s\n",__FUNCTION__,__LINE__,__FILE__);}

void buffer_dump(void * buf, int size);

#ifdef	__cplusplus
}
#endif

#endif	/* GUI_DEBUG_H */

