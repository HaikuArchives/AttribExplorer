// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// Main header and AttribExplorerApp definition.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


#ifndef MAIN_H
#define MAIN_H

// Standard C++ Library headers
#include <vector>
using namespace std;

// BeOS headers
#include <Application.h>
#include <FilePanel.h>

// Project headers
#include "FileWindow.h"
#include "WindowsList.h"

class AttribExplorerApp : public BApplication {
	private:
		int					nextWindowLocation;
		BFilePanel			*filePanel;
		bool				got_refs_at_startup;
		vector<FileWindow*>	windows;
		WindowsList			*list;
	public:
							AttribExplorerApp(void);
		void				ReadyToRun(void);
		void				AboutRequested(void);
		void				RefsReceived(BMessage *message);
		void				MessageReceived(BMessage *message);
		bool				QuitRequested(void);
		bool				pasteAllowed;
};

#endif // MAIN_H
