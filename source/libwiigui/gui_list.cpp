/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_filebrowser.cpp
 *
 * GUI class definitions
 */

#include "gui.h"
#include "gui_list.h"
#include "../w_input.h"
#include "../gui_debug.h"

/**
 * Constructor for the GuiFileBrowser class.
 */
GuiList::GuiList(int w, int h, int orientation) {
	width = w;
	height = h;
	numEntries = 0;
	selectedItem = 0;
	selectable = true;
	listChanged = true; // trigger an initial list update
	focus = 0; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, 0, PAD_BUTTON_A);
	trig2 = new GuiTrigger;
	trig2->SetSimpleTrigger(-1, 0, 0);

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, 0, PAD_BUTTON_A);

	this->orientation = orientation;

	listCenter = 1;

	this->selector = NULL;
}

/**
 * Destructor for the GuiFileBrowser class.
 */
GuiList::~GuiList() {
	delete trigHeldA;
	delete trigA;
	delete trig2;
	_elements.clear();
}

void GuiList::Append(GuiButton* e) {
	if (e == NULL)
		return;

	Remove(e);
	_elements.push_back(e);
	e->SetParent(this);
//	if (orientation == 'V')
//		e->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
//	else
//		e->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	e->SetAnimation(true);
	e->SetAnimationDuration(1);
}

void GuiList::Insert(GuiButton* e, u32 index) {
	if (e == NULL || index > (_elements.size() - 1))
		return;

	Remove(e);
	_elements.insert(_elements.begin() + index, e);
	e->SetParent(this);
}

void GuiList::Remove(GuiButton* e) {
	if (e == NULL)
		return;

	u32 elemSize = _elements.size();
	for (u32 i = 0; i < elemSize; ++i) {
		if (e == _elements.at(i)) {
			_elements.erase(_elements.begin() + i);
			break;
		}
	}
}

bool GuiList::IsInside(GuiElement *parent,GuiElement *child) {

	if (orientation == 'V') {
		if (child->GetTop() < parent->GetTop()) {
			return false;
		}
		if (child->GetTop() + child->GetHeight() > parent->GetTop() + parent->GetHeight()) {
			return false;
		}
	} else {
		if (child->GetLeft() < parent->GetLeft()) {
//			printf("%d < %d\n",child->GetLeft() , parent->GetLeft());
			return false;
		}
		if (child->GetLeft() + child->GetWidth() > parent->GetLeft() + parent->GetWidth()) {
			return false;
		}
	}

	return true;
}

void GuiList::RemoveAll() {
	_elements.clear();
}

bool GuiList::Find(GuiButton* e) {
	if (e == NULL)
		return false;

	u32 elemSize = _elements.size();
	for (u32 i = 0; i < elemSize; ++i)
		if (e == _elements.at(i))
			return true;
	return false;
}

GuiButton* GuiList::GetGuiElementAt(u32 index) const {
	if (index >= _elements.size())
		return NULL;
	return _elements.at(index);
}

u32 GuiList::GetSize() {
	return _elements.size();
}

void GuiList::SetFocus(int f) {
	focus = f;

	u32 elemSize = _elements.size();
	for (u32 i = 0; i < elemSize; ++i) {
		_elements.at(i)->ResetState();
	}

	if (f == 1 && selectedItem >= 0)
		_elements.at(selectedItem)->SetState(STATE_SELECTED);
}

void GuiList::ResetState() {
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	u32 elemSize = _elements.size();
	for (u32 i = 0; i < elemSize; ++i) {
		_elements.at(i)->ResetState();
	}
}

void GuiList::TriggerUpdate() {
	listChanged = true;
}

/**
 * Draw the button on screen
 */
void GuiList::Draw() {
	if (!this->IsVisible()) {
		return;
	}

	if (this->selector) {
		this->selector->Draw();
	}

	u32 elemSize = _elements.size();
	for (u32 i = 0; i < elemSize; ++i) {
		_elements.at(i)->Draw();
	}


	this->UpdateEffects();
}

void GuiList::DrawTooltip() {
}

void GuiList::SetCount(int c) {
	this->itemCount = c;
}

void GuiList::SetCenter(int c) {
	this->listCenter = c;
}

int GuiList::GetCenter() {
	return this->listCenter;
}

void GuiList::SetSelector(GuiImage * img) {
	this->selector = img;
}

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

void GuiList::Update(GuiTrigger * t) {
	int xstep, ystep;

	int start, count;

	if (state == STATE_DISABLED || !t)
		return;

	int c = this->listCenter;

	u32 elemSize = _elements.size();

	if (selectedItem > c)
		start = (c - 1) - selectedItem;
	else
		start = 0;
	count = MIN(itemCount, (elemSize) - start);

	u32 pos0 = (c - 1) - MIN(selectedItem, c - 1);
	start = MIN(MAX(0, selectedItem - (c - 1)), (elemSize + 1) - itemCount);
	count = MIN(itemCount - pos0, elemSize);
	if (start + count > elemSize)
		count--;

	for (u32 i = 0; i < elemSize; ++i) {
		_elements.at(i)->UpdateEffects();
		_elements.at(i)->SetState(STATE_DISABLED);
	}

	ystep = this->height / itemCount;
	xstep = this->width / itemCount;

	for (u32 i = 0; i < elemSize; ++i) {
		if (_elements.at(i)->GetState() == STATE_DISABLED)
			_elements.at(i)->SetState(STATE_DEFAULT);

		if (orientation == 'V') {
			u32 pos = (c + (i - start)) - MIN(selectedItem, c - 1);
			_elements.at(i)->SetAnimationPosition(0, (pos - 1) * ystep);
			//			_elements.at(i)->SetPosition(0, (pos - 1) * ystep);
		} else {
			u32 pos = (c + (i - start)) - MIN(selectedItem, c - 1);
			_elements.at(i)->SetAnimationPosition((pos - 1) * xstep, 0);
			//			_elements.at(i)->SetPosition((pos - 1) * xstep, 0);
		}

		if (IsInside(this,_elements.at(i))) {
			_elements.at(i)->SetEffect(EFFECT_FADE_OUT, 1, 0);
		} else {
			_elements.at(i)->SetEffect(EFFECT_FADE_OUT, -1, 0);
		}
	}

	if (orientation == 'V') {
		if (t->Up()) {
			selectedItem--;
		} else if (t->Down()) {
			selectedItem++;
		}
	} else {
		if (t->Right()) {
			selectedItem++;
		} else if (t->Left()) {
			selectedItem--;
		}
	}

	// clamp selected item value
	if (selectedItem < 0)
		selectedItem = 0;
	if (selectedItem >= elemSize - 1)
		selectedItem = elemSize - 1;

	if (updateCB)
		updateCB(this);
}

int GuiList::GetValue() {
	return selectedItem;
}

void GuiList::SetSelected(int c) {
	selectedItem = c;
}