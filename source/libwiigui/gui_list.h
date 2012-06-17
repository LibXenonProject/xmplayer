/* 
 * File:   gui_list.h
 * Author: cc
 *
 * Created on 28 avril 2012, 01:24
 */

#ifndef GUI_LIST_H
#define	GUI_LIST_H

#include "gui.h"
#include <vector>

#define MAX_LISTSIZE 10

//!Display a list of files

class GuiList : public GuiElement {
public:
	GuiList(int w, int h, int orientation = 'V');
	~GuiList();
	void ResetState();

	void SetFocus(int f);
	void Draw();
	void DrawTooltip();
	void TriggerUpdate();
	void Update(GuiTrigger * t);

	//!Appends a GuiElement to the GuiWindow
	//!\param e The GuiElement to append. If it is already in the GuiWindow, it is removed first
	void Append(GuiButton* e);
	//!Inserts a GuiElement into the GuiWindow at the specified index
	//!\param e The GuiElement to insert. If it is already in the GuiWindow, it is removed first
	//!\param i Index in which to insert the element
	void Insert(GuiButton* e, u32 i);
	//!Removes the specified GuiElement from the GuiWindow
	//!\param e GuiElement to be removed
	void Remove(GuiButton* e);
	//!Removes all GuiElements
	void RemoveAll();
	//!Looks for the specified GuiElement
	//!\param e The GuiElement to find
	//!\return true if found, false otherwise
	bool Find(GuiButton* e);
	//!Returns the GuiElement at the specified index
	//!\param index The index of the element
	//!\return A pointer to the element at the index, NULL on error (eg: out of bounds)
	GuiButton* GetGuiElementAt(u32 index) const;
	//!Returns the size of the list of elements
	//!\return The size of the current element list
	u32 GetSize();
	// number of visible item
	void SetCount(int c);
	void SetCenter(int c);
	int GetCenter();
	void SetSelected(int c);
	
		//!Checks whether the specified element is within the element's boundaries
	//!\return true if contained within, false otherwise
	bool IsInside(GuiElement *parent,GuiElement *child);

	void SetSelector(GuiImage *);

	int GetValue();
protected:
	GuiImage * selector;

	GuiSound * btnSoundOver;
	GuiSound * btnSoundClick;
	GuiTrigger * trigA;
	GuiTrigger * trig2;
	GuiTrigger * trigHeldA;

	int orientation;
	int selectedItem;
	int numEntries;
	int itemCount;
	int listCenter;
	bool listChanged;
	std::vector<GuiButton*> _elements; //!< Contains all elements within the GuiList
};

#endif	/* GUI_LIST_H */

