/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * Modified by Ced2911, 2011
 *
 * input.h
 * Wii/GameCube controller management
 ***************************************************************************/

#ifndef _INPUT_H_
#define _INPUT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "w_input.h"
#include <input/input.h>

#define PI 				3.14159265f
#define PADCAL			50

void SetupPads();
void UpdatePads();
void ShutoffRumble();
void DoRumble(int i);

void XenonInputInit();
uint16_t XenonButtonsDown(int ictrl);
uint16_t XenonButtonsUp(int ictrl);
uint16_t XenonButtonsHeld(int ictrl);
void XenonInputUpdate();

#ifdef __cplusplus
}
#endif
#endif
