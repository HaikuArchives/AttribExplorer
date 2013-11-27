// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// FileWindow definition.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


#ifndef FILEWINDOW_H
#define FILEWINDOW_H

// Standard C++ Library headers
#include <vector>
using namespace std;

// BeOS headers
#include <Entry.h>
#include <ListView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Node.h>
#include <Rect.h>
#include <String.h>
#include <Window.h>

// Project headers
#include "Attribute.h"

class FileWindow : public BWindow {
	private:
		void				ClearList(void);
		void				BuildList(void);
		void				RemoveNodeAttribs(void);
		void				WriteNodeAttrib(const Attribute *attrib);
		void				CopyNodeAttribs(void);
		void				PasteNodeAttribs(void);
		BNode*				GetNode(void);
		void				ShowErrorMessage(BString msg);
		void				SelectionChanged(void);
		BMenu				*editMenu;
		BMenu				*viewMenu;
		BMenu				*viewDataMenu;
		BMenu				*viewTypeMenu;
		BMenuItem			*editCut;
		BMenuItem			*editCopy;
		BMenuItem			*editDelete;
		BListView			*attribList;
		vector<Attribute*>	originalAttribs;
		int					attribCount;
	public:
							FileWindow(BRect frame, entry_ref *ref);
							// ... destructor missing ... memory waste
		void				MessageReceived(BMessage *message);
		bool				QuitRequested(void);
		void				Refresh(void);
		entry_ref			fileEntry;
		BString				fileName;
		BMenuItem			*editPaste;
};

#endif // FILEWINDOW_H
