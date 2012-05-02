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
	//	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigA->SetSimpleTrigger(-1, 0, PAD_BUTTON_A);
	trig2 = new GuiTrigger;
	//	trig2->SetSimpleTrigger(-1, WPAD_BUTTON_2, 0);
	trig2->SetSimpleTrigger(-1, 0, 0);

	trigHeldA = new GuiTrigger;
	//	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trigHeldA->SetHeldTrigger(-1, 0, PAD_BUTTON_A);

	//	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	//	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	this->orientation = orientation;

	listCenter = 1;
	
	this->selector = NULL;
}

/**
 * Destructor for the GuiFileBrowser class.
 */
GuiList::~GuiList() {
	//	delete btnSoundOver;
	//	delete btnSoundClick;
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
	if (orientation == 'V')
		e->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	else
		e->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	
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

	//
//		// debug 
//		XeColor color;
//		color.lcol = 0xFF00007F;
//	
//		int currLeft = this->GetLeft();
//		int thisTop = this->GetTop();
//	
//		int thisHeight = this->GetHeight();
//		int thisWidth = this->GetWidth();
//	
//		Menu_DrawRectangle(currLeft, thisTop, thisWidth, thisHeight, color, 1);

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
	//img->SetParent(this);
	//img->SetAlignment(ALIGN_CENTRE,ALIGN_MIDDLE);
	//img->SetPosition(this->GetLeft(),this->GetTop());
}

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

void GuiList::Update(GuiTrigger * t) {
	int xstep, ystep;

	int start, count;

	if (state == STATE_DISABLED || !t)
		return;

	int c = this->listCenter;
	//int c = 3;

	u32 elemSize = _elements.size();

	//start = MAX(0, selectedItem - itemCount / 2);
	//start = MAX(0, selectedItem - itemCount);
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
		//_elements.at(i)->SetVisible(false);
		_elements.at(i)->SetState(STATE_DISABLED);
	}

	ystep = this->height / itemCount;
	xstep = this->width / itemCount;

	/*
	for (u32 i = start; i < start + count; ++i) {

		if (_elements.at(i)->GetState() == STATE_DISABLED)
			_elements.at(i)->SetState(STATE_DEFAULT);

		_elements.at(i)->SetVisible(true);
		if (orientation == 'V') {
			u32 pos = (c + (i - start)) - MIN(selectedItem, c - 1);
			//_elements.at(i)->SetPosition(0, (pos - 1) * ystep);
			_elements.at(i)->SetAnimationPosition(0, (pos - 1) * ystep);
		} else {
			u32 pos = (c + (i - start)) - MIN(selectedItem, c - 1);
			//_elements.at(i)->SetPosition((pos - 1) * xstep, 0);
			_elements.at(i)->SetAnimationPosition((pos - 1) * xstep, 0);
		}

		//		if (i == selectedItem) {
		//			_elements.at(i)->SetAlpha(0x7f);
		//			_elements.at(i)->SetScale(1.2);
		//		} else {
		//			_elements.at(i)->SetScale(1);
		//			_elements.at(i)->SetAlpha(0xff);
		//		}
	}
	 */ 
	
	
	for (u32 i = 0; i < elemSize; ++i) {
		if (_elements.at(i)->GetState() == STATE_DISABLED)
			_elements.at(i)->SetState(STATE_DEFAULT);

		if (orientation == 'V') {
			u32 pos = (c + (i - start)) - MIN(selectedItem, c - 1);
			//_elements.at(i)->SetPosition(0, (pos - 1) * ystep);
			_elements.at(i)->SetAnimationPosition(0, (pos - 1) * ystep);
		} else {
			u32 pos = (c + (i - start)) - MIN(selectedItem, c - 1);
			//_elements.at(i)->SetPosition((pos - 1) * xstep, 0);
			_elements.at(i)->SetAnimationPosition((pos - 1) * xstep, 0);
		}
		
		//if(i>=start && i < start + count )
		if(_elements.at(i)->IsInside(this))
//		if(
//				this->IsInside(_elements.at(i)->GetMinX(),_elements.at(i)->GetMinY()) && 
//				this->IsInside(_elements.at(i)->GetMaxX(),_elements.at(i)->GetMaxY()) 
//			)
		{
			//_elements.at(i)->SetVisible(true);
			_elements.at(i)->SetEffect(EFFECT_FADE_OUT,1,0);
		}
		else{
			//_elements.at(i)->SetVisible(false);
			_elements.at(i)->SetEffect(EFFECT_FADE_OUT,-1,0);
		}
	}

	if (orientation == 'V') {
		if (t->Up()) {
//			_elements.at(selectedItem)->ResetState();
			selectedItem--;
		} else if (t->Down()) {
//			_elements.at(selectedItem)->ResetState();
			selectedItem++;
		}
	} else {
//		if (t->Down() || t->Up()) {
//			selectedItem = 0;
//		}
		if (t->Right()) {
//			_elements.at(selectedItem)->ResetState();
			selectedItem++;
		} else if (t->Left()) {
//			_elements.at(selectedItem)->ResetState();
			selectedItem--;
		}
	}

	// clamp selected item value
	if (selectedItem < 0)
		selectedItem = 0;
	if (selectedItem >= elemSize - 1)
		selectedItem = elemSize - 1;
	
	//_elements.at(selectedItem)->SetState(STATE_SELECTED,t->chan);
	
//	for (u32 i = 0; i < elemSize; ++i) {
//		_elements.at(i)->Update(t);
//	}

	if (updateCB)
		updateCB(this);
}


int GuiList::GetValue(){
	return selectedItem;
}