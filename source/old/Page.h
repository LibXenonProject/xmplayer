/* 
 * File:   Page.h
 * Author: cc
 *
 * Created on 27 avril 2012, 21:05
 */

#ifndef PAGE_H
#define	PAGE_H

#include "video.h"
#include "FreeTypeGX.h"
#include <vector>
#include <wchar.h>

typedef struct {
	struct XenosSurface * img;
	int w, h, x, y;
} menu_image;

class menu_text
{
public:
	wchar_t * text;
	int w, h, x, y;
	int font_size;
	XeColor color;
	int style;
	int visible;
	void setDimension(int x,int y,int w,int h){
		this->x=x;
		this->y=y;
		this->w=w;
		this->h=h;
	}
};

class Callback
{
public:
    virtual void call() = 0;
};

typedef struct {
	Callback * callback;
	int when;
} page_callback;

class Page {
public:
	Page();
	virtual ~Page();
	void addImage(menu_image *);
	void addText(menu_text *);
	void addCallback(page_callback *);
	void draw();
private:
	std::vector <page_callback*> callback_list;
	std::vector <menu_image*> image_list;
	std::vector <menu_text*> text_list;
	std::vector <FreeTypeGX*> font_list;
	
	FreeTypeGX * getFontBySize(int size);
};


class List:public Callback{
private:
	char orientation;
	Page * page;
	int position;
	int w, h, x, y;	
	int font_size;
	int style;
	XeColor color;
	int item_count;
public:
	List(char p='V');
	void setToPage(Page * page);
	void setOrentation(char p);
	void setDimension(int x,int y,int w, int h);
	void setStyle(int _style);
	void setFontSize(int s);
	void setColor(XeColor _color);
	int getSize();
	// visible item
	void setItemCount(int v);
	int getItemCount();
	
	void setPosition(int pos);
	int getPosition();
	void call();
	void addText(wchar_t * text);
	std::vector <menu_text*> text_list;
};

static inline void DrawMenuImage(menu_image * element) {
	Menu_DrawImg(element->x, element->y, element->w, element->h, element->img, 0, 1, 1, 255);
}


static inline void load_element(menu_image * element, const void * png, int x, int y, int w, int h) {
	element->img = loadPNGFromMemory((unsigned char*)png);
	element->x = x;
	element->y = y;
	element->w = w;
	element->h = h;
}

#endif	/* PAGE_H */

