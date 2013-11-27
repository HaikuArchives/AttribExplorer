// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// Main file. Includes main() function and AttribExplorer application class
// implementation.
// The main AttribExplorerApp's purpose is to open and close windows.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


// BeOS headers
#include <Alert.h>
#include <Clipboard.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <Roster.h>
#include <String.h>

// Project headers
#include "Constants.h"
#include "FileWindow.h"
#include "Main.h"
#include "Debug/DebugClient.h"

int main(int, char**)
{
	AttribExplorerApp *myApp = new AttribExplorerApp();
	myApp->Run();
	return 0;
}

// Application constructor
AttribExplorerApp::AttribExplorerApp() :
	BApplication(kAppMIME)
{
	tracex("App", dbgMINOR, "Main application object constructor");

	nextWindowLocation	= 0;
	got_refs_at_startup = false;
	pasteAllowed = false;

	filePanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE |
		B_DIRECTORY_NODE | B_SYMLINK_NODE);
	filePanel->Window()->SetTitle("Open node");
	list = new WindowsList(&windows);
}

// Application is ready to run
void AttribExplorerApp::ReadyToRun(void)
{
	tracex("App", dbgMAJOR, "Application running");

	if (!got_refs_at_startup) {
		tracex("App", dbgMINOR, "No refs received at starturp, "
			"opening file panel");
		filePanel->Show();
	}

	// Force updating of Paste menu items
	be_app_messenger.SendMessage(B_CLIPBOARD_CHANGED);
	be_clipboard->StartWatching(be_app_messenger);
}

// File references received, open a new window for each file
void AttribExplorerApp::RefsReceived(BMessage *message)
{
	if (IsLaunching() == true) {
		tracex("App", dbgMINOR, "Got refs at startup");
		got_refs_at_startup = true;
	}

	FileWindow *win;
	int index = 0;
	entry_ref ref;
	
	while (message->FindRef("refs", index++, &ref) == B_OK) 
	{
		tracex("App", dbgMINOR, "Received ref, searching window");
		win = NULL;	

		// Check if ref has already got his own window
		for (vector<FileWindow*>::iterator it = windows.begin();
			it < windows.end(); it++) {
			if ((*it)->fileEntry == ref)
				win = *it;
		}

		if (win != NULL) {
			// Activate existing window
			tracex("App", dbgMAJOR, "Found window " << (void*)win <<
				", activating");
			win->Activate();
		}
		else {
			// Creates a new window
			tracex("App", dbgMAJOR, "Window not found, building a new one");
			BRect frame;	
			frame = BRect(WINRECT_START_X, WINRECT_START_Y, WINRECT_WIDTH +
				WINRECT_START_X, WINRECT_HEIGHT + WINRECT_START_Y);
			frame.OffsetBy(WINRECT_STEP_X * nextWindowLocation,
				WINRECT_STEP_Y * nextWindowLocation);
			win = new FileWindow(frame, &ref);
			windows.push_back(win);
			win->Show();
			if (++nextWindowLocation == MAX_CASCADED_WINDOWS)
				nextWindowLocation = 0;
		}
	}

	list->Refresh();
}

// Message received
void AttribExplorerApp::MessageReceived(BMessage *message)
{
	switch (message->what) {
		
		// File panel canceled
		case B_CANCEL:
			tracex("App", dbgMINOR, "File panel canceled");
			if (windows.size() == 0) {
				tracex("App", dbgMAJOR, "No more windows, quitting");
				PostMessage(B_QUIT_REQUESTED);
			}
			break;

		// Window closed
		case MSG_KILL_ME: {
			FileWindow *win;
			if (message->FindPointer("window", (void**)&win) == B_OK) {
				tracex("App", dbgMAJOR, "Closing window " << (void*)win <<
					" (" << win->fileName.String() << ")");
				for (vector<FileWindow*>::iterator it = windows.begin();
					it < windows.end(); it++) {
					if (*it == win)
						windows.erase(it);
				}
				win->Lock();
				win->Quit();
				if (windows.size() == 0 && !filePanel->IsShowing()) {
					tracex("App", dbgMAJOR, "No more windows, quitting");
					PostMessage(B_QUIT_REQUESTED);
				}
				list->Refresh();
			}
			} break;

		// Menuitem "Open" selected
		case MSG_OPEN_NODE:
			tracex("App", dbgMINOR, "Opening file panel");
			if (filePanel->IsShowing())
				filePanel->Window()->Activate();
			else
				filePanel->Window()->Show();
			break;

		// Menuitem "Show windows list" selected
		case MSG_WINDOWS_SHOW_LIST:
			if (list->IsHidden())
				list->Show();
			else
				list->Activate();
			break;

		// Request to activate window
		case MSG_ACTIVATE_WINDOW: {
			tracex("App", dbgMINOR, "Window activation requested");
			FileWindow *win = NULL;
			int32 index;
			if (message->FindInt32("index", 0, &index) == B_OK) {
				if (index < (int)windows.size())
					win = windows[index];
			}
			if (win != NULL) {
				tracex("App", dbgMINOR, "Window " << (void*)win <<
					" found, activating");
				win->Activate();
			}
			else
				tracex("App", dbgMINOR, "Bad window index");
			} break;

		// System clipboard's data changed
		case B_CLIPBOARD_CHANGED: {
			tracex("App", dbgMINOR, "Clipboard data changed");
			BMessage *clip, *data = new BMessage();
			pasteAllowed = false;
			if (be_clipboard->Lock()) {
				if ((clip = be_clipboard->Data()) != NULL)
					pasteAllowed = (clip->FindMessage(kAttribType, data) == B_OK);
				be_clipboard->Unlock();
			}
			delete data;
			for (vector<FileWindow*>::iterator win = windows.begin();
				win < windows.end(); win++)
				(*win)->editPaste->SetEnabled(pasteAllowed);
			} break;

		default:
			BApplication::MessageReceived(message);
	}
}

// About requested
void AttribExplorerApp::AboutRequested(void)
{
	char *url = kAppURL;
	BString about = kAppName " " kAppVersion "\n"
					kAppCopyright "\n"
					"E-mail: " kAppEMail "\n"
					"Web site: " kAppURL "\n\n"
					"This program is distribuited under the terms of the GNU "
					"General Public License versione 2, as published by the "
					"Free Software Foundation. See the file COPYING for "
					"details.\n\n"
					"Compiled: " __TIME__ " " __DATE__ ;

	#ifdef DEBUG
		about += "\n\nDebug version.";
	#endif

	if ((new BAlert("FileAttribInfo", about.String(), "Web site", "OK"))->Go() == 0)
		be_roster->Launch("text/html", 1, (char**)&url);
}

// Quit requested
bool AttribExplorerApp::QuitRequested(void)
{
	tracex("App", dbgMAJOR, "Quitting application");

	be_clipboard->StopWatching(be_app_messenger);
	delete filePanel;

	// Remove FileWindows manually, since they have QuitRequested() 
	// returning 'false' instead of the usual 'true'
	while(CountWindows() > 0)
	{
		BWindow *win = WindowAt(0);
		if (win != NULL)
		{
			win->Lock();
			win->Quit();
		}
	}

	return BApplication::QuitRequested();
}
