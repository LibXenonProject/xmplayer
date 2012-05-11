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
	if (e == NULL)
		return;

	Remove(e);
	_elements.push_back(e);
	e->SetParent(this);
	
	int elementSize = GetSize();
	if(e_bg)
		elementSize--;

	int colW = width / colCount;
	int colH = height / rowCount;
	
	int posX = (elementSize % colCount) * colW;
	int posY = (elementSize / rowCount) * colH;
	
	// position
	SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	SetPosition(posX, posY);
};

void GuiTab::setRow(int r) {
	rowCount = r;
};

void GuiTab::setCol(int c) {
	colCount = c;
};

void GuiTab::SetBackground(GuiElement* e) {
	e_bg = e;
	Append(e);
	// good position

};