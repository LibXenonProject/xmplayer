/* 
 * File:   Page.cpp
 * Author: cc
 * 
 * Created on 27 avril 2012, 21:05
 */

#include "Page.h"
#include "input.h"

Page::Page() {
}

Page::~Page() {
}

void Page::addCallback(page_callback * elt) {
	callback_list.push_back(elt);
}

void Page::addImage(menu_image * elt) {
	image_list.push_back(elt);
}

void Page::addText(menu_text * elt) {
//	text_list.push_back(elt);
//	// create the font if needed
//	getFontBySize(elt->font_size);
	FreeTypeGX * font = getFontBySize(text_list[elt->font_size]->font_size);
	menu_image * element = new menu_image();
	element->img = font->createText(elt->text,elt->color);
	element->x =elt->x;
	element->y = elt->y;
	element->w =elt->w;
	element->h =elt->h;
	image_list.push_back(element);
};

FreeTypeGX * Page::getFontBySize(int size) {
	FreeTypeGX * font = NULL;

	// create the needed font ?
	for (int i = 0; i < font_list.size(); i++) {
		if (font_list[i]->getFontSize() == size) {
			font = font_list[i];
		}
	}

	if (font == NULL) {
		font = new FreeTypeGX(size);
		font_list.push_back(font);
	}

	return font;
}

void Page::draw() {
	for (int i = 0; i < image_list.size(); i++) {
		DrawMenuImage(image_list[i]);
	}
	for (int i = 0; i < text_list.size(); i++) {
		if(text_list[i]->visible){
			FreeTypeGX * font = getFontBySize(text_list[i]->font_size);
			font->drawText(text_list[i]->x, text_list[i]->y, text_list[i]->text, text_list[i]->color, text_list[i]->style);
		}
		text_list[i]->visible = 0;
	}
	for (int i = 0; i < callback_list.size(); i++) {
		callback_list[i]->callback->call();
	}
	Menu_Render();
}

List::List(char p) {
	orientation = p;
	position = 0;
}

void List::setToPage(Page * page) {
	page_callback * pcallback = new page_callback();
	pcallback->callback = this;
	page->addCallback(pcallback);

	this->page = page;
}

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

void List::call() {
	// called when updating
	int xstep, ystep;
	
	int start, count;
	
//	start=MAX(0,getPosition()-getItemCount()/2);
//	count=MIN(getItemCount(),(getSize())-start);
	
	start=MAX(0,getPosition()-getItemCount()/2);
	count=MIN(getItemCount(),(getSize())-start);

	if (orientation == 'V') {
		xstep = 0;
		ystep = h / getItemCount();

		for (int i = start; i < start+count; i++) {
			text_list[i]->setDimension(text_list[i]->x, (y + h) - (ystep * (i-start)), text_list[i]->w, text_list[i]->h);
			
			if(i==position)
				text_list[i]->color.r = 0x00;
			else
				text_list[i]->color.r = 0xff;			
			
			text_list[i]->visible = 1;
		}
	} else {
		xstep = w / getItemCount();
		ystep = 0;

		for (int i = start; i < start+count; i++) {
			
			if(i==position)
				text_list[i]->color.r = 0x00;
			else
				text_list[i]->color.r = 0xff;
			
			text_list[i]->setDimension((x + w) - (xstep * (i-start)), text_list[i]->y, text_list[i]->w, text_list[i]->h);
			
			text_list[i]->visible = 1;
		}
	}
}

void List::addText(wchar_t * str) {
	menu_text * text = new menu_text();
	text->text = str;
	text->color = color;
	text->style = style;
	text->font_size = font_size;

	text->setDimension(x, y, w, h);

	text_list.push_back(text);
	page->addText(text);
}

void List::setOrentation(char p) {
	orientation = p;
}

void List::setColor(XeColor _color) {
	color = _color;
}

void List::setFontSize(int s) {
	font_size = s;
}

void List::setDimension(int x, int y, int w, int h) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}

void List::setStyle(int _style) {
	this->style = _style;
}

void List::setPosition(int pos) {
	this->position = pos;
}

int List::getPosition() {
	return this->position;
}

int List::getSize() {
	return text_list.size();
}

void List::setItemCount(int v) {
	item_count = v;
}

int List::getItemCount() {
	return item_count;
}