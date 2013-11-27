// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// FileWindow class. One instance of this class is created for each opened
// file.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


// Standard C headers
#include <alloc.h>

// BeOS headers
#include <Alert.h>
#include <Application.h>
#include <Clipboard.h>
#include <fs_attr.h>
#include <MenuBar.h>
#include <Path.h>
#include <ScrollView.h>

// Project headers
#include "AttributeItem.h"
#include "Constants.h"
#include "Exception.h"
#include "FileWindow.h"
#include "Main.h"
#include "Debug/DebugClient.h"

// +-------------------------------------------------------------------------+
// | CONSTRUCTOR                                                             |
// +-------------------------------------------------------------------------+

// Window constructor
FileWindow::FileWindow(BRect frame, entry_ref *ref) :
	BWindow(frame, "Attributes", B_DOCUMENT_WINDOW, B_NOT_ZOOMABLE |
		B_ASYNCHRONOUS_CONTROLS)
{
	BMenu *nodeMenu, *windowsMenu;
	BString windowTitle;
	BMenuItem *menuItem;
	BMenuBar *menuBar;
	BRect rect;

	// Get the file name
	fileEntry = *ref;
	BEntry *entry = new BEntry(&fileEntry, true);
	BPath *path = new BPath();
	entry->GetPath(path);
	fileName = path->Path();

	tracex("Window", dbgMAJOR, "  Window " << (void*)this << " ("
		<< fileName.String() << ") constructor");

	// Set window title
	windowTitle = "Attributes of ";
	windowTitle += fileName.String();
	SetTitle(windowTitle.String());

	// Begin creating menus
	menuBar = new BMenuBar(BRect(0, 0, 10, 10), "MenuBar");

	// Builds the Node menu
	nodeMenu = new BMenu("Node");

	menuItem = new BMenuItem("Open...", new BMessage(MSG_OPEN_NODE), 'O');
	menuItem->SetTarget(be_app);
	nodeMenu->AddItem(menuItem);

	nodeMenu->AddItem(new BMenuItem("Close ", new BMessage(B_QUIT_REQUESTED),
		'W'));

	nodeMenu->AddItem(new BMenuItem("Refresh", new BMessage(MSG_REFRESH_LIST),
		'R'));

	nodeMenu->AddSeparatorItem();

	menuItem = new BMenuItem("About...", new BMessage(B_ABOUT_REQUESTED));
	menuItem->SetTarget(be_app);
	nodeMenu->AddItem(menuItem);

	menuBar->AddItem(nodeMenu);

	// Builds the Edit menu
	editMenu = new BMenu("Edit");

	editCut = new BMenuItem("Cut", new BMessage(MSG_EDIT_CUT), 'X');
	editMenu->AddItem(editCut);

	editCopy = new BMenuItem("Copy", new BMessage(MSG_EDIT_COPY), 'C');
	editMenu->AddItem(editCopy);

	editPaste = new BMenuItem("Paste", new BMessage(MSG_EDIT_PASTE), 'V');
	editMenu->AddItem(editPaste);

	editDelete = new BMenuItem("Delete", new BMessage(MSG_EDIT_DELETE));
	editMenu->AddItem(editDelete);

	editMenu->AddSeparatorItem();

	editMenu->AddItem(new BMenuItem("Select all",
		new BMessage(MSG_EDIT_SELECT_ALL)));

	editMenu->AddItem(new BMenuItem("Select none",
		new BMessage(MSG_EDIT_SELECT_NONE)));

	editMenu->AddItem(new BMenuItem("Invert selection",
		new BMessage(MSG_EDIT_INVERT_SELECTION)));

	menuBar->AddItem(editMenu);

	// Builds the data view menu
	viewDataMenu = new BMenu("Data");

	menuItem = new BMenuItem("Intel (little endian)",
		new BMessage(MSG_DATA_VIEW_INTEL));
	menuItem->SetEnabled(false);
	viewDataMenu->AddItem(menuItem);

	menuItem = new BMenuItem("PowerPC (big endian)",
		new BMessage(MSG_DATA_VIEW_POWERPC));
	menuItem->SetEnabled(false);
	viewDataMenu->AddItem(menuItem);

	viewDataMenu->AddSeparatorItem();

	viewDataMenu->AddItem(new BMenuItem("Default",
		new BMessage(MSG_DATA_VIEW_DEFAULT)));

	viewDataMenu->AddItem(new BMenuItem("Hex dump",
		new BMessage(MSG_DATA_VIEW_HEX)));

	viewDataMenu->AddItem(new BMenuItem("String",
		new BMessage(MSG_DATA_VIEW_STRING)));

	viewDataMenu->AddItem(new BMenuItem("C String",
		new BMessage(MSG_DATA_VIEW_C_STRING)));

	// Builds the type view menu
	viewTypeMenu = new BMenu("Type");

	viewTypeMenu->AddItem(new BMenuItem("Default",
		new BMessage(MSG_TYPE_VIEW_DEFAULT)));

	viewTypeMenu->AddItem(new BMenuItem("Hex dump",
		new BMessage(MSG_TYPE_VIEW_HEX)));

	viewTypeMenu->AddItem(new BMenuItem("String",
		new BMessage(MSG_TYPE_VIEW_STRING)));

	viewTypeMenu->SetRadioMode(true);

	// Builds the View style menu
	viewMenu = new BMenu("View style");

	viewMenu->AddItem(viewDataMenu);

	viewMenu->AddItem(viewTypeMenu);

	menuBar->AddItem(viewMenu);

	// Builds the Windows menu
	windowsMenu = new BMenu("Windows");

	menuItem = new BMenuItem("Show list",
		new BMessage(MSG_WINDOWS_SHOW_LIST));
	menuItem->SetTarget(be_app);
	windowsMenu->AddItem(menuItem);

	windowsMenu->AddSeparatorItem();

	menuItem = new BMenuItem("Close all", new BMessage(B_QUIT_REQUESTED),
		'Q');
	menuItem->SetTarget(be_app);
	windowsMenu->AddItem(menuItem);

	menuBar->AddItem(windowsMenu);

	// Finished building menus
	AddChild(menuBar);

	// Creates the main list view
	rect = Bounds();
	rect.top += menuBar->Frame().Height() + 1;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	attribList = new BListView(rect, "AttributesList",
		B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	attribList->SetSelectionMessage(new BMessage(MSG_SELECTION_CHANGED));
	AddChild(new BScrollView("ListScroll", attribList, B_FOLLOW_ALL_SIDES, 0,
		true, true, B_NO_BORDER));

	// Enables Edit -> Paste
	editPaste->SetEnabled(((AttribExplorerApp*)be_app)->pasteAllowed);

	// Creates the list of attributes
	attribCount = 0;
	Refresh();
	SelectionChanged();
}

// +-------------------------------------------------------------------------+
// | MESSAGES                                                                |
// +-------------------------------------------------------------------------+

// Message received
void FileWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {

		// File dropped on window
		case B_SIMPLE_DATA:
			be_app_messenger.SendMessage(message);
			break;
		
		// Refresh list
		case MSG_REFRESH_LIST: {
			BString msg = "Warning!\n\n";
			msg += fileName;
			msg += "\n\nIf you refresh the attributes list, you won't be "
				"able to restore the original attributes. Are you sure you "
				"want to do this?";
			BAlert *alert = new BAlert("Refresh list", msg.String(),
				"Yes", "No", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			if (alert->Go() == 0)
				Refresh();
			} break;

		// Edit -> Cut
		case MSG_EDIT_CUT:
			try {
				CopyNodeAttribs();
				RemoveNodeAttribs();
			}
			catch (Exception &exception) {
				ShowErrorMessage(exception.message);
			}
			break;

		// Edit -> Copy
		case MSG_EDIT_COPY:
			try {
				CopyNodeAttribs();
			}
			catch (Exception &exception) {
				ShowErrorMessage(exception.message);
			}
			break;

		// Edit -> Paste
		case MSG_EDIT_PASTE:
			try {
				PasteNodeAttribs();
			}
			catch (Exception &exception) {
				ShowErrorMessage(exception.message);
			}
			break;

		// Edit -> Delete
		case MSG_EDIT_DELETE:
			try {
				RemoveNodeAttribs();
			}
			catch (Exception &exception) {
				ShowErrorMessage(exception.message);
			}
			break;

		// Edit -> Select all
		case MSG_EDIT_SELECT_ALL:
			if (attribCount > 0)		
				attribList->Select(0, attribCount - 1, false);
			break;
		
		// Edit -> Select none
		case MSG_EDIT_SELECT_NONE:
			attribList->DeselectAll();
			break;

		// Edit -> Select all
		case MSG_EDIT_INVERT_SELECTION:
			if (attribCount > 0) {
				for (int i = 0; i < attribCount; i++) {
					if (attribList->ItemAt(i)->IsSelected())
						attribList->Deselect(i);
					else
						attribList->Select(i, true);
				}
			}
			break;

		// Change data endianess
		case MSG_DATA_VIEW_INTEL:
		case MSG_DATA_VIEW_POWERPC:
			if (attribCount > 0) {
				int i = 0, selected;
				AttributeItem *item;
				DataEndian endian;
				switch (message->what) {
					case MSG_DATA_VIEW_INTEL: endian = DE_INTEL; break;
					case MSG_DATA_VIEW_POWERPC: endian = DE_POWERPC; break;
					default: endian = DE_DEFAULT; break;
				}
				viewDataMenu->ItemAt(0)->SetMarked(endian == DE_INTEL);
				viewDataMenu->ItemAt(1)->SetMarked(endian == DE_POWERPC);
				while ((selected = attribList->CurrentSelection(i++)) > -1) {
					item = (AttributeItem *)(attribList->ItemAt(selected));
					item->SetEndian(endian);
					attribList->InvalidateItem(selected);
				}
			}
			break;

		// Change data view style
		case MSG_DATA_VIEW_DEFAULT:
		case MSG_DATA_VIEW_HEX:
		case MSG_DATA_VIEW_STRING:
		case MSG_DATA_VIEW_C_STRING:
			if (attribCount > 0) {
				int i = 0, selected, index;
				AttributeItem *item;
				DataView view;
				switch (message->what) {
					case MSG_DATA_VIEW_HEX:
						view = DV_HEX_VIEW;
						index = 4;
						break;
					case MSG_DATA_VIEW_STRING:
						view = DV_STRING_VIEW;
						index = 5;
						break;
					case MSG_DATA_VIEW_C_STRING:
						view = DV_C_STRING_VIEW;
						index = 6;
						break;
					default:
						view = DV_DEFAULT_VIEW;
						index = 3;
						break;
				}
				for (int i = 3; i < 7; i++)
					viewDataMenu->ItemAt(i)->SetMarked(i == index);
				while ((selected = attribList->CurrentSelection(i++)) > -1) {
					item = (AttributeItem *)(attribList->ItemAt(selected));
					item->SetDataViewStyle(view);
					attribList->InvalidateItem(selected);
				}
			}
			break;

		// Change type view style
		case MSG_TYPE_VIEW_DEFAULT:
		case MSG_TYPE_VIEW_HEX:
		case MSG_TYPE_VIEW_STRING:
			if (attribCount > 0) {
				AttributeItem *item;
				int i = 0, selected;
				TypeView view;
				switch (message->what) {
					case MSG_TYPE_VIEW_HEX: view = TV_HEX_VIEW; break;
					case MSG_TYPE_VIEW_STRING: view = TV_STRING_VIEW; break;
					default: view = TV_DEFAULT_VIEW; break;
				}
				while ((selected = attribList->CurrentSelection(i++)) > -1) {
					item = (AttributeItem *)(attribList->ItemAt(selected));
					item->SetTypeViewStyle(view);
					attribList->InvalidateItem(selected);
				}
			}
			break;

		// Selection changed
		case MSG_SELECTION_CHANGED:
			SelectionChanged();
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

// Window is told to close
bool FileWindow::QuitRequested(void)
{
	BMessage quit_msg(MSG_KILL_ME);
	quit_msg.AddPointer("window", this);
	be_app_messenger.SendMessage(&quit_msg);
	return false;
}

// +-------------------------------------------------------------------------+
// | ATTRIBUTES READING                                                      |
// +-------------------------------------------------------------------------+

// Updates the list of attributes
void FileWindow::Refresh(void)
{
	tracex("Window", dbgMAJOR, "  Window " << (void*)this << " (" <<
		fileName.String() << "): refreshing attributes list");

	char attrName[B_ATTR_NAME_LENGTH];
	unsigned char *buffer = NULL;
	AttribReadError error;
	attr_info attrInfo;
	BNode *node;

	// Clear original attributes list
	for (unsigned i = 0; i < originalAttribs.size(); i++)
		delete originalAttribs[i];
	originalAttribs.clear();

	// Clear existing list
	ClearList();

	// Get the BNode of the file
	try {
		node = GetNode();
	}
	catch (Exception &exception) {
		attribCount = 0;
		Lock();
		attribList->AddItem(new BStringItem(exception.message.String()));
		Unlock();
		return;
	}

	// Read attributes from the node
	while (node->GetNextAttrName(attrName) == B_OK) {
		error = ARE_NO_ERROR;
		attrInfo.size = 0;
		attrInfo.type = 0;
		if (node->GetAttrInfo(attrName, &attrInfo) == B_OK) {
			buffer = new unsigned char[attrInfo.size];
			if (!node->ReadAttr(attrName, 0, 0, (void *)buffer,	attrInfo.size)) {
				attrInfo.size = 0;
				error = ARE_READ_ERROR;
				delete[] buffer;
				buffer = NULL;
			}
		}
		else
			error = ARE_NO_INFO;
		originalAttribs.push_back(new Attribute(attrName, attrInfo.type,
			attrInfo.size, buffer, error));
		if (buffer != NULL)
			delete[] buffer;
	}

	delete node;
	BuildList();
}

// +-------------------------------------------------------------------------+
// | ATTRIBUTES WRITING                                                      |
// +-------------------------------------------------------------------------+

// Removes selected node's attribute
void FileWindow::RemoveNodeAttribs(void)
{
	if (attribCount == 0)
		return;

	BNode *node = GetNode();
	AttributeItem *item;
	status_t res;
	int32 index;

	while ((index = attribList->CurrentSelection(0)) > -1) {
		item = (AttributeItem*)attribList->ItemAt(index);
		res = node->RemoveAttr(item->GetAttribute()->name.String());
		if (res != B_OK) {
			delete node;
			throw new Exception("Error removing attribute");
		}
		attribList->RemoveItem(index);
		delete item;
	}
	delete node;
}

// Writes a single node's attributes
void FileWindow::WriteNodeAttrib(const Attribute *attrib)
{
	BNode *node = GetNode();
	size_t res;
	res = node->WriteAttr(attrib->name.String(), attrib->type, 0,
		(void*)attrib->data, attrib->size);
	delete node;
	if (res != attrib->size)
		throw new Exception("Error writing attribute");
}

// +-------------------------------------------------------------------------+
// | LIST HANDLING                                                           |
// +-------------------------------------------------------------------------+

// Clears existing list
// We need this because BListItem::MakeEmpty() doesn't frees items
void FileWindow::ClearList(void)
{
	Lock();
	if (attribCount == 0) {
		if (attribList->CountItems() == 1)
			delete (BStringItem*)attribList->RemoveItem((int32)0);
	}
	else {		
		for (int i = 0; i < attribCount; i++)
			delete (AttributeItem*)attribList->RemoveItem((int32)0);
	}
	attribCount = 0;
	attribList->MakeEmpty();
	Unlock();
}

// Builds on-screen list based on original attributes
void FileWindow::BuildList(void)
{
	Lock();
	viewMenu->SetEnabled(false);
	ClearList();
	if (originalAttribs.size() > 0) {
		for (vector<Attribute*>::iterator attrib = originalAttribs.begin();
			attrib < originalAttribs.end(); attrib++)
			attribList->AddItem(new AttributeItem(*attrib));
	}
	else
		attribList->AddItem(new BStringItem("File has no attributes."));
	attribCount = originalAttribs.size();
	Unlock();
}

// +-------------------------------------------------------------------------+
// | CLIPBOARD                                                               |
// +-------------------------------------------------------------------------+

// Copies selected attributes into the system clipboard
void FileWindow::CopyNodeAttribs(void)
{
	if (attribCount == 0)
		return;

	int index, i = 0, skipped = 0;
	AttributeItem *item;
	BMessage *clip;

	if (be_clipboard->Lock()) {
		be_clipboard->Clear();
		if ((clip = be_clipboard->Data()) != NULL) {

			while ((index = attribList->CurrentSelection(i++)) > -1) {
				item = (AttributeItem*)attribList->ItemAt(index);
				if (item->GetAttribute()->error != ARE_NO_ERROR)
					skipped++;
				else {
				
					// Ok, the Be Book says we should add raw data, with
					// B_MIME_TYPE type, and with the name matching the data's
					// MIME type. So, I first made this:
					//  clip->AddData(kAttribType, B_MIME_TYPE,
					//	  item->GetAttribute(), sizeof(Attribute));
					// but I think it's stupid, so I changed it.
					
					clip->AddMessage(kAttribType,
						item->GetAttribute()->GetMessage());
				}
			}

			be_clipboard->Commit();
		}
		else {
			be_clipboard->Unlock();
			throw new Exception("Can't get clipboard data");
		}
		be_clipboard->Unlock();
		if (skipped > 0)
			(new BAlert("CopyAttribSkip", "Some of the selected "
				"attributes couldn't be copied due to some read error.", "OK",
				NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
	}
	else
		throw new Exception("Can't lock clipboard");
}

// Paste attributes from the system clipboard
void FileWindow::PasteNodeAttribs(void)
{
	BMessage *clip, *data = new BMessage();
	vector<Attribute*>::iterator it;
	vector<Attribute*> attribs;
	Attribute *attrib;
	BString msg;
	BNode *node;
	size_t res;
	int i = 0;
	
	// Collect attributes from clipboard
	if (be_clipboard->Lock()) {
		if ((clip = be_clipboard->Data()) != NULL) {
			Lock();
			while (clip->FindMessage(kAttribType, i++, data) == B_OK) {
				try {
					attrib = new Attribute(data);
					attribs.push_back(attrib);
				}
				catch (Exception &exception) {
					BString txt = "Error copying attribute from "
						"clipboard: ";
					txt += exception.message;
					ShowErrorMessage(txt.String());
				}
			}
			Unlock();
			if (attribs.size() == 0) {
				ShowErrorMessage("Nothing to paste.");
				return;
			}
		}
		else {
			be_clipboard->Unlock();
			throw new Exception("Can't get clipboard data");
		}
		be_clipboard->Unlock();
	}
	else
		throw new Exception("Can't lock clipboard");

	// Adds attributes to the file, then to the list
	node = GetNode();
	for (it = attribs.begin(); it < attribs.end(); it++) {
		bool okToAdd = true;

		// TODO: place all this stuff in a function like:
		// AddNodeAttrid(Attribute *attrib)
		// This should be used also in load-attributes-from-file function.

		// Make sure there isn't another attribute with same name
		for (i = 0; i < attribList->CountItems(); i++) {
			if (((AttributeItem*)(attribList->ItemAt(i)))->GetAttribute()->name
				== (*it)->name)
			{
				msg = "You're going to paste an attribute named \n\n";
				msg += (*it)->name;
				msg += "\n\nSince there's already one attribute with the "
					"same name, if you continue the existing attribute will "
					"be replaced with this one. Otherwise, you can skip this "
					"attribute and proceed with the next.";
				if ((new BAlert("PasteExists", msg.String(), "Continue",
					"Skip", NULL))->Go() == 0)
				{
					AttributeItem *attrib =
						(AttributeItem*)(attribList->RemoveItem(i));
					delete attrib;
					// No need to remove phisically from file, it will be
					// clobbered when we write the new one
				}
				else
					okToAdd = false;
			}
		}

		// Write the attribute
		if (okToAdd) {
			res = node->WriteAttr((*it)->name.String(), (*it)->type, 0,
				(void*)(*it)->data, (*it)->size);
			if (res != (*it)->size)
				ShowErrorMessage("Error writing attribute");
			else {
				attribList->AddItem(new AttributeItem(*it));
			}
		}
		
		// Frees memory used by attribute
		delete *it;

	}
	delete node;
}

// +-------------------------------------------------------------------------+
// | MISCELLANEOUS                                                           |
// +-------------------------------------------------------------------------+

// Turns the file into a BNode
BNode* FileWindow::GetNode(void)
{
	BNode *node = new BNode(&fileEntry);
	if (node->InitCheck() != B_OK)
		throw new Exception("Error creating node.");
	return node;
}

// Shows an error message
void FileWindow::ShowErrorMessage(BString msg)
{
	BAlert *alert = new BAlert("Error", msg.String(), "OK", NULL, NULL,
		B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->Go();
}

// User's selection changed
void FileWindow::SelectionChanged(void)
{
	AttributeItem *item;
	int selected = attribList->CurrentSelection(0);
	if ((selected > -1) && (attribCount > 0)) {

		// Only one attribute is selected
		if (attribList->CurrentSelection(1) == -1) {
			int dataIndex, typeIndex, endianIndex;
			item = (AttributeItem *)(attribList->ItemAt(selected));
			switch (item->GetDataViewStyle()) {
				case DV_HEX_VIEW: dataIndex = 4; break;
				case DV_STRING_VIEW: dataIndex = 5; break;
				case DV_C_STRING_VIEW: dataIndex = 6; break;
				default: dataIndex = 3;
			}
			switch (item->GetTypeViewStyle()) {
				case TV_HEX_VIEW: typeIndex = 1; break;
				case TV_STRING_VIEW: typeIndex = 2; break;
				default: typeIndex = 0;
			}
			switch (item->GetDataEndian()) {
				case DE_INTEL: endianIndex = 0; break;
				case DE_POWERPC: endianIndex = 1; break;
				default: endianIndex = (DE_DEFAULT == DE_INTEL ?
					0 : 1);
			}
			for (int i = 0; i < 7; i++)
				viewDataMenu->ItemAt(i)->SetMarked(false);
				viewDataMenu->ItemAt(endianIndex)->SetMarked(true);
				viewDataMenu->ItemAt(dataIndex)->SetMarked(true);
				viewTypeMenu->ItemAt(typeIndex)->SetMarked(true);
		}

		// More than one attribute are selected
		else {
			for (int i = 0; i < 7; i++)
				viewDataMenu->ItemAt(i)->SetMarked(false);
			BMenuItem *markedItem = viewTypeMenu->FindMarked();
			if (markedItem != NULL)
				markedItem->SetMarked(false);
		}

		viewMenu->SetEnabled(true);
		editCut->SetEnabled(true);
		editCopy->SetEnabled(true);
		editDelete->SetEnabled(true);
	}
	else {      // No items selected
		viewMenu->SetEnabled(false);
		editCut->SetEnabled(false);
		editCopy->SetEnabled(false);
		editDelete->SetEnabled(false);
	}
}
