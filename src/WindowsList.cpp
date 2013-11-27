// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// The WindowsList class is a window wich shows up a small window with a
// list of currently opened files.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


// BeOS headers
#include <Application.h>
#include <Rect.h>
#include <ScrollView.h>

// Project headers
#include "Constants.h"
#include "WindowsList.h"
#include "Debug/DebugClient.h"

// WindowList constructor
WindowsList::WindowsList(vector<FileWindow*> *windows) :
	BWindow(BRect(490, 330, 639, 479), "Windows list", B_FLOATING_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE |
		B_ASYNCHRONOUS_CONTROLS)
{
	tracex("List", dbgMINOR, "Windows list constructor");

	BRect rect;

	WindowsList::windows = windows;

	rect = Bounds();
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	list = new BListView(rect, "WindowsList", B_MULTIPLE_SELECTION_LIST,
		B_FOLLOW_ALL_SIDES);
	list->SetInvocationMessage(new BMessage(MSG_ACTIVATE_WINDOW));
	list->SetTarget(be_app);
	AddChild(new BScrollView("ListScroll", list, B_FOLLOW_ALL_SIDES, 0, true,
		true, B_NO_BORDER));
}

void WindowsList::Refresh(void)
{
	tracex("List", dbgMINOR, "Refreshing windows list");
	BString caption;
	Lock();
	list->MakeEmpty();
	for (vector<FileWindow*>::iterator win = windows->begin();
		win < windows->end(); win++)
		list->AddItem(new BStringItem((*win)->fileName.String()));
	if (windows->size() > 1)
		caption << windows->size() << " windows";
	else
		caption = "1 window";  // windows->size() == 0 should never happen
	SetTitle(caption.String());
	Unlock();
}

void WindowsList::Show(void)
{
	tracex("List", dbgMINOR, "Showing windows list");
	Refresh();
	BWindow::Show();
}

bool WindowsList::QuitRequested(void)
{
	tracex("List", dbgMINOR, "Hiding windows list");
	Lock();
	Hide();
	Unlock();
	return false;
}
