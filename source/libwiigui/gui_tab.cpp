#include "gui.h"
#include "gui_tab.h"
#include "../w_input.h"
#include "../gui_debug.h"

GuiTab::GuiTab(int w, int h) {
	width = w;
	height = h;
	focus = 0; // allow focus
	rowCount = 1;
	colCount = 1;
	e_bg = NULL;
}

void GuiTab::Append(GuiElement* e) {
	int posX,posY,colW,colH;
	
	if (e == NULL)
		return;

	Remove(e);
	_elements.push_back(e);
	e->SetParent(this);
	
	int elementSize = GetSize();
	if(e_bg)
		elementSize--;
	elementSize--;

	colW = width / colCount;
	colH = height / rowCount;
	
	posX = (elementSize % colCount) * colW;
	posY =(elementSize / colCount)  * colH;
	
	// position
	e->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	e->SetPosition(posX, posY);
};

void GuiTab::setRow(int r) {
	rowCount = r;
};

void GuiTab::setCol(int c) {
	colCount = c;
};

void GuiTab::SetBackground(GuiElement* e) {
	e_bg = e;
	
	if (e == NULL)
		return;
	
	// good position
	e->SetPosition(e->GetLeft() -GetLeft() ,e->GetTop()-GetTop());

	e->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	
	Remove(e);
	_elements.push_back(e);
	e->SetParent(this);
};