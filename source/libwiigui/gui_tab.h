/* 
 * File:   gui_tab.h
 * Author: cc
 *
 * Created on 11 mai 2012, 14:32
 */

#ifndef GUI_TAB_H
#define	GUI_TAB_H

#include "gui.h"

class GuiTab : public GuiWindow {
public:
	GuiTab(int w,int h);
	void Append(GuiElement* e);
	
	void setRow(int r);
	void setCol(int c);
	
	void SetBackground(GuiElement* e);
private:
	int colCount;
	int rowCount;
	
	int lastX;
	int lastY;
	
	GuiElement* e_bg;
};


#endif	/* GUI_TAB_H */

