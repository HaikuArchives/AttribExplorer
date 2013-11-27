// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// WindowsList definition.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


#ifndef WINDOWSLIST_H
#define WINDOWSLIST_H

// Standard C++ Library headers
#include <vector>
using namespace std;

// BeOS headers
#include <ListView.h>
#include <Window.h>

// Project headers
#include "FileWindow.h"

class WindowsList : public BWindow {
	private:
		BListView 			*list;
		vector<FileWindow*>	*windows;
	public:
							WindowsList(vector<FileWindow*> *windows);
		void				Show(void);
		bool				QuitRequested(void);
		void				Refresh(void);
};

#endif // WINDOWSLIST_H
