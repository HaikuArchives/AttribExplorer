// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// AttributeItem class. This class stores and displays a single file attribute
// and "reminds" the original attribute, if it was modified.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


// Standard C headers
#include <alloc.h>
#include <stdio.h>

// BeOS headers
#include <Font.h>
#include <Point.h>
#include <Region.h>
#include <TypeConstants.h>
#include <View.h>

// Project headers
#include "AttributeItem.h"
#include "Constants.h"
#include "Debug/DebugClient.h"

// Define colors. We define our colors because ui_color() seems to report the
// wrong ones :-(
#ifndef _USE_UI_COLORS
	const rgb_color SelectionColor = {216, 216, 216, 255};
	const rgb_color UnselTextColor = {0, 0, 0, 255};
	const rgb_color SelTextColor = {0, 0, 0, 255};
#else
	const rgb_color SelectionColor =
		ui_color(B_MENU_SELECTION_BACKGROUND_COLOR);
	const rgb_color UnselTextColor =
		ui_color(B_MENU_ITEM_TEXT_COLOR);
	const rgb_color SelTextColor =
		ui_color(B_MENU_SELECTED_ITEM_TEXT_COLOR);
#endif
const rgb_color ErrTextColor = {255, 0, 0, 255};

// +-------------------------------------------------------------------------+
// | CONSTRUCTORS AND INITIALIZATION                                         |
// +-------------------------------------------------------------------------+

// Class constructor
AttributeItem::AttributeItem(Attribute* data,
	AttributeError error = AE_NO_ERROR,
	DataEndian dataEndian = DE_DEFAULT,
	DataView dataView = DV_DEFAULT_VIEW,
	TypeView typeView = TV_DEFAULT_VIEW)
{
	initialized = false;
	Initialize(data, error, dataEndian, dataView, typeView);
}

// Class destructor
AttributeItem::~AttributeItem()
{
	delete attrib;
	delete original;
}

// Initialize the object
void AttributeItem::Initialize(Attribute* data,
	AttributeError error = AE_NO_ERROR,
	DataEndian dataEndian = DE_DEFAULT,
	DataView dataView = DV_DEFAULT_VIEW,
	TypeView typeView = TV_DEFAULT_VIEW)
{
	if (initialized) {
		delete attrib;
		delete original;
	}

	tracex("Attrib", dbgMAJOR, "    Attribute " << data->name.String() <<
		" constructor");

	attrib = new Attribute(*data);
	original = new Attribute(*data);

	AttributeItem::error = (AttributeError)attrib->error;
	typeString = GetTypeString();
	if ((dataView == DV_DEFAULT_VIEW) || (dataView == DV_HEX_VIEW) ||
		(dataView == DV_STRING_VIEW) || (dataView == DV_C_STRING_VIEW))
		AttributeItem::dataView = dataView;
	else
		AttributeItem::dataView = DV_DEFAULT_VIEW;
	if ((typeView == TV_DEFAULT_VIEW) || (typeView == TV_HEX_VIEW) ||
		(typeView == TV_STRING_VIEW))
		AttributeItem::typeView = typeView;
	else
		AttributeItem::typeView = TV_DEFAULT_VIEW;
	if ((dataEndian == DE_INTEL) || (dataEndian == DE_POWERPC))
		AttributeItem::dataEndian = dataEndian;
	else
		AttributeItem::dataEndian = DE_DEFAULT;

	// Check wich error applies
	if ((error == AE_NO_ERROR) || (error == AE_UNKNOWN_TYPE) ||
		(error == AE_UNHANDLED_TYPE)) {
		if (IsTypeKnown()) {
			if (!IsTypeHandled())
				AttributeItem::error = AE_UNHANDLED_TYPE;
		}
		else
			AttributeItem::error = AE_UNKNOWN_TYPE;
	}

	// Generate data string
	/*if ((data != NULL) && (size > 0)) {
		AttributeItem::data = (unsigned char *)malloc(size);
		AttributeItem::size = size;
		memcpy((void *)AttributeItem::data, (void *)data, size);
	}
	else {
		AttributeItem::data = NULL;
		AttributeItem::size = 0;
	}*/
	dataString = GetDataString();

	initialized = true;
}

// +-------------------------------------------------------------------------+
// | DRAWING                                                                 |
// +-------------------------------------------------------------------------+

// Draws the item
void AttributeItem::DrawItem(BView *owner, BRect frame, bool complete = false)
{
	if (!initialized)
		return;

	BRect nameFrame, typeFrame, sizeFrame, dataFrame;
	font_height fontHeight;
	char sizeString[11]; // Enough for almost 10 Gb of data...
	int baseline;

	// Calculates the name, type, size, and data frames
	nameFrame = BRect(0, frame.top, COLUMN_NAME_WIDTH - 1, frame.bottom);
	typeFrame = BRect(nameFrame.right + 1, frame.top, nameFrame.right +
		COLUMN_TYPE_WIDTH, frame.bottom);
	sizeFrame = BRect(typeFrame.right + 1, frame.top, typeFrame.right +
		COLUMN_SIZE_WIDTH, frame.bottom);
	dataFrame = BRect(sizeFrame.right + 1, frame.top, frame.right,
		frame.bottom);

	// Calculates the text baseline
	owner->GetFontHeight(&fontHeight);
	baseline = (int)(frame.top + (frame.Height() + fontHeight.ascent -
		fontHeight.descent) / 2 + 1);

	// Draw selection rectangle
	owner->SetDrawingMode(B_OP_COPY);
	owner->SetHighColor(IsSelected() ? SelectionColor : owner->ViewColor());
	owner->FillRect(frame);
	owner->SetDrawingMode(B_OP_OVER);

	// Draw attribute name
	owner->MovePenTo(nameFrame.left + 2, baseline);
	owner->SetHighColor(IsSelected() ? SelTextColor : UnselTextColor);
	owner->DrawString(attrib->name.String());

	// Draw attribute type
	owner->MovePenTo(typeFrame.left, baseline);
	owner->DrawString(typeString.String());

	// Draw attribute size
	if (attrib->data != NULL)
		sprintf(sizeString, "%lu", attrib->size);
	else
		sprintf(sizeString, "-");
	owner->MovePenTo(sizeFrame.left, baseline);
	owner->DrawString(sizeString);

	// Draw attribute data
	owner->MovePenTo(dataFrame.left, baseline);
	owner->SetHighColor(error == AE_NO_ERROR ? (IsSelected() ? SelTextColor :
		UnselTextColor) : ErrTextColor);
	owner->DrawString(dataString.String());
}

// +-------------------------------------------------------------------------+
// | PRIVATE STATIC METHODS                                                  |
// +-------------------------------------------------------------------------+

// Returns the given type as a string
BString AttributeItem::GetTypeAsString(type_code type)
{
	char ret[sizeof(type_code) + 1];
	#ifdef __INTEL__
		ret[0] = (type >> 24) % 0x100;
		ret[1] = (type >> 16) % 0x100;
		ret[2] = (type >> 8) % 0x100;
		ret[3] = type % 0x100;
	#elif defined(__POWERPC__)
		ret[0] = type % 0x100;
		ret[1] = (type >> 8) % 0x100;
		ret[2] = (type >> 16) % 0x100;
		ret[3] = (type >> 24) % 0x100;
	#endif
	ret[4] = 0;
	return ret;
}

// Returns a string of hex values of the given data
void AttributeItem::ConvHex(const unsigned char *data, BString &out,
	size_t len = 1)
{
	const char hexDigit[17] = "0123456789ABCDEF";
	char hex[4] = "xx ";
	size_t i = 0;

	out = "";

	while (i < len) {
		hex[0] = hexDigit[data[i] / 16];
		hex[1] = hexDigit[data[i] % 16];
		out += hex;
		i++;
	}
}

// Returns a string of the given data
void AttributeItem::ConvString(const unsigned char *data, BString &out,
	size_t len = 1)
{
	BString ch = "";
	size_t i = 0;

	out = "";

	while (i < len) {
		if ((data[i] >= 32) && (data[i] <= 127))
			out += data[i];
		else
			out += '.';
		i++;
	}
}

// Returns a string of C-style characters of the given data
void AttributeItem::ConvCString(const unsigned char *data, BString &out,
	size_t len = 1)
{
	char octalDigit[9] = "01234567";
	char octal[5] = "\\xxx";
	BString ch = "";
	size_t i = 0;

	out = "";

	while (i < len) {
		if ((data[i] >= 32) && (data[i] <= 127)) {
			switch (data[i]) {
				case '\\': out += "\\""\\"; break;
				case '\'': out += "\\""\'"; break;
				case '\"': out += "\\""\""; break;
				case '%': out += "%%"; break;
				default: out += data[i];
			}
		}
		else {
			switch (data[i]) {
				case '\n': out += "\\""n"; break;
				case '\t': out += "\\""t"; break;
				case '\v': out += "\\""v"; break;
				case '\b': out += "\\""b"; break;
				case '\r': out += "\\""r"; break;
				case '\f': out += "\\""f"; break;
				case '\a': out += "\\""a"; break;
				default: {
					octal[1] = octalDigit[data[i] / 64];
					octal[2] = octalDigit[(data[i] % 64) / 8];
					octal[3] = octalDigit[data[i] % 8];
					out += octal;
				}
			}
		}
		i++;
	}
}

// +-------------------------------------------------------------------------+
// | PUBLIC STATIC METHODS                                                   |
// +-------------------------------------------------------------------------+

// Returns the description of the given error
BString AttributeItem::GetErrorString(AttributeError error)
{
	switch (error) {
		case AE_NO_ERROR: return "No error"; break;
		case AE_UNHANDLED_TYPE: return "Type not handled"; break;
		case AE_UNKNOWN_TYPE: return "Unknown type"; break;
		case AE_NO_INFO: return "Can't get attribute information"; break;
		case AE_READ_ERROR: return "Reading error"; break;
		case AE_UNEXPECTED_ERROR: return "Unexpected error"; break;
		default: return "Unknown error";
	}
}

// Returns the description of the given type
BString AttributeItem::GetTypeString(type_code type,
	TypeView typeView = TV_DEFAULT_VIEW)
{
	switch (typeView) {
		case TV_HEX_VIEW: {
			char ret[sizeof(type_code) * 2 + 1];
			sprintf(ret, "%08X", (unsigned int)type);
			return ret;
			} break;
		case TV_STRING_VIEW:
			return GetTypeAsString(type);
			break;
		default:
			switch (type) {
				case B_ANY_TYPE: return "any"; break;
				case B_BOOL_TYPE: return "bool"; break;
				case B_CHAR_TYPE: return "char"; break;
				case B_COLOR_8_BIT_TYPE: return "8-bit bitmap"; break;
				case B_DOUBLE_TYPE: return "double"; break;
				case B_FLOAT_TYPE: return "float"; break;
				case B_GRAYSCALE_8_BIT_TYPE: return "grayscale bitmap"; break;
				case B_INT64_TYPE: return "int64"; break;
				case B_INT32_TYPE: return "int32"; break;
				case B_INT16_TYPE: return "int16"; break;
				case B_INT8_TYPE: return "int8"; break;
				case B_MESSAGE_TYPE: return "BMessage"; break;
				case B_MESSENGER_TYPE: return "BMessenger"; break;
				case B_MIME_TYPE: return "MIME"; break;
				case B_MONOCHROME_1_BIT_TYPE: return "1-bit bitmap"; break;
				case B_OBJECT_TYPE: return "object pointer"; break;
				case B_OFF_T_TYPE: return "off_t"; break;
				case B_PATTERN_TYPE: return "pattern"; break;
				case B_POINTER_TYPE: return "pointer"; break;
				case B_POINT_TYPE: return "BPoint"; break;
				case B_RAW_TYPE: return "raw data"; break;
				case B_RECT_TYPE: return "BRect"; break;
				case B_REF_TYPE: return "entry_ref"; break;
				case B_RGB_32_BIT_TYPE: return "truecolor bitmap"; break;
				case B_RGB_COLOR_TYPE: return "rgb_color"; break;
				case B_SIZE_T_TYPE: return "size_t"; break;
				case B_SSIZE_T_TYPE: return "ssize_t"; break;
				case B_STRING_TYPE: return "string"; break;
				case B_TIME_TYPE: return "time_t"; break;
				case B_UINT64_TYPE: return "uint64"; break;
				case B_UINT32_TYPE: return "uint32"; break;
				case B_UINT16_TYPE: return "uint16"; break;
				case B_UINT8_TYPE: return "uint8"; break;
				case B_MEDIA_PARAMETER_TYPE:
					return "Media Kit parameter";
					break;
				case B_MEDIA_PARAMETER_WEB_TYPE:
					return "Media Kit parameter web";
					break;
				case B_MEDIA_PARAMETER_GROUP_TYPE:
					return "Media Kit parameter group";
					break;
				default:
					return GetTypeAsString(type);
			}
	}
}

// Checks if the given type is known or not
bool AttributeItem::IsTypeKnown(type_code type)
{
	return (type == B_ANY_TYPE) ||
		(type == B_BOOL_TYPE) ||
		(type == B_CHAR_TYPE) ||
		(type == B_COLOR_8_BIT_TYPE) ||
		(type == B_DOUBLE_TYPE) ||
		(type == B_FLOAT_TYPE) ||
		(type == B_GRAYSCALE_8_BIT_TYPE) ||
		(type == B_INT64_TYPE) ||
		(type == B_INT32_TYPE) ||
		(type == B_INT16_TYPE) ||
		(type == B_INT8_TYPE) ||
		(type == B_MESSAGE_TYPE) ||
		(type == B_MESSENGER_TYPE) ||
		(type == B_MIME_TYPE) ||
		(type == B_MONOCHROME_1_BIT_TYPE) ||
		(type == B_OBJECT_TYPE) ||
		(type == B_OFF_T_TYPE) ||
		(type == B_PATTERN_TYPE) ||
		(type == B_POINTER_TYPE) ||
		(type == B_POINT_TYPE) ||
		(type == B_RAW_TYPE) ||
		(type == B_RECT_TYPE) ||
		(type == B_REF_TYPE) ||
		(type == B_RGB_32_BIT_TYPE) ||
		(type == B_RGB_COLOR_TYPE) ||
		(type == B_SIZE_T_TYPE) ||
		(type == B_SSIZE_T_TYPE) ||
		(type == B_STRING_TYPE) ||
		(type == B_TIME_TYPE) ||
		(type == B_UINT64_TYPE) ||
		(type == B_UINT32_TYPE) ||
		(type == B_UINT16_TYPE) ||
		(type == B_UINT8_TYPE) ||
		(type == B_MEDIA_PARAMETER_TYPE) ||
		(type == B_MEDIA_PARAMETER_WEB_TYPE) ||
		(type == B_MEDIA_PARAMETER_GROUP_TYPE);
}

// Checks if the given type is handled or not
bool AttributeItem::IsTypeHandled(type_code type)
{
	return (type == B_BOOL_TYPE) ||
		(type == B_CHAR_TYPE) ||
		(type == B_DOUBLE_TYPE) ||
		(type == B_FLOAT_TYPE) ||
		(type == B_INT64_TYPE) ||
		(type == B_INT32_TYPE) ||
		(type == B_INT16_TYPE) ||
		(type == B_INT8_TYPE) ||
		(type == B_MIME_TYPE) ||
		(type == B_OBJECT_TYPE) ||
		(type == B_OFF_T_TYPE) ||
		(type == B_POINTER_TYPE) ||
		(type == B_POINT_TYPE) ||
		(type == B_RAW_TYPE) ||
		(type == B_RECT_TYPE) ||
		(type == B_RGB_COLOR_TYPE) ||
		(type == B_SIZE_T_TYPE) ||
		(type == B_SSIZE_T_TYPE) ||
		(type == B_STRING_TYPE) ||
		(type == B_TIME_TYPE) ||
		(type == B_UINT64_TYPE) ||
		(type == B_UINT32_TYPE) ||
		(type == B_UINT16_TYPE) ||
		(type == B_UINT8_TYPE);
}

// Returns the number of bytes expected for the given type
size_t AttributeItem::GetTypeSize(type_code type, bool *fixed = NULL)
{
	bool empty;
	size_t ret;

	if (fixed == NULL)
		fixed = &empty;

	if (!IsTypeHandled(type) || (type == B_RAW_TYPE)) {
		*fixed = false;
		return 0;
	}

	*fixed = true;
	switch (type) {
		case B_BOOL_TYPE:
			ret = sizeof(bool);
			break;
		case B_CHAR_TYPE:
		case B_INT8_TYPE:
		case B_UINT8_TYPE:
			ret = sizeof(char);
			break;
		case B_STRING_TYPE:
		case B_MIME_TYPE:
			ret = 1;
			*fixed = false;
			break;
		case B_INT16_TYPE:
		case B_UINT16_TYPE:
			ret = sizeof(short int);
			break;
		case B_INT32_TYPE:
		case B_SSIZE_T_TYPE:
		case B_UINT32_TYPE:
		case B_SIZE_T_TYPE:
			ret = sizeof(long);
			break;
		case B_INT64_TYPE:
		case B_OFF_T_TYPE:
		case B_TIME_TYPE:
		case B_UINT64_TYPE:
			ret = sizeof(long long);
			break;
		case B_FLOAT_TYPE:
			ret = sizeof(float);
			break;
		case B_DOUBLE_TYPE:
			ret = sizeof(double);
			break;
		case B_OBJECT_TYPE:
		case B_POINTER_TYPE:
			ret = sizeof(void*);
			break;
		case B_POINT_TYPE:
			ret = sizeof(BPoint);
			break;
		case B_RECT_TYPE:
			ret = sizeof(BRect);
			break;
		case B_RGB_COLOR_TYPE:
			ret = sizeof(rgb_color);
			break;
		default:
			*fixed = false;
			ret = 0;
	}

	return ret;
}

// +-------------------------------------------------------------------------+
// | GETTER METHODS                                                          |
// +-------------------------------------------------------------------------+

// Endian-swaps data if required
/*int16 AttributeItem::EndianSwap(int16 number)
{
	bool swap;
	#ifdef __INTEL__
		swap = (isBigEndian*/

// Returns the data in string format
BString AttributeItem::GetDataString(void)
{
	if (error != AE_NO_ERROR) {
		if (error != AE_UNKNOWN_TYPE && error != AE_UNHANDLED_TYPE)
			return GetErrorString();
		else {
			if (dataView != DV_HEX_VIEW && dataView != DV_STRING_VIEW &&
				dataView != DV_C_STRING_VIEW)
				return GetErrorString();
		} 
	}
	if ((attrib->data == NULL) || (attrib->size == 0))
		return "<empty>";

	unsigned typeSize;
	char outs[256];
	BString out;
	bool fixed;

	typeSize = GetTypeSize(&fixed);
	if ((typeSize > attrib->size) && (dataView == DV_DEFAULT_VIEW))
		return "<not enough data>";

	switch (dataView) {

		case DV_HEX_VIEW:
			ConvHex(attrib->data, out, attrib->size);
			break;

		case DV_STRING_VIEW:
			ConvString(attrib->data, out, attrib->size);
			break;

		case DV_C_STRING_VIEW:
			ConvCString(attrib->data, out, attrib->size);
			break;

		default: {
			outs[0] = 0;
			switch (attrib->type) {
				case B_BOOL_TYPE:
					if ((bool)*attrib->data)
						out = "true";
					else
						out = "false";
					break;
				case B_CHAR_TYPE:
					ConvCString(attrib->data, out, 1);
					break;
				case B_STRING_TYPE:
				case B_MIME_TYPE:
					ConvString(attrib->data, out, attrib->size - 1);
					break;
				case B_RAW_TYPE:
					ConvHex(attrib->data, out, attrib->size);
					break;
				case B_INT8_TYPE:
					sprintf(outs, "%d", *(int8 *)attrib->data);
					break;
				case B_INT16_TYPE:
					sprintf(outs, "%hd", *(int16 *)attrib->data);
					break;
				case B_INT32_TYPE:
				case B_SSIZE_T_TYPE:
					sprintf(outs, "%ld", *(int32 *)attrib->data);
					break;
				case B_INT64_TYPE:
				case B_OFF_T_TYPE:
				case B_TIME_TYPE:
					sprintf(outs, "%Ld", *(int64 *)attrib->data);
					break;
				case B_UINT8_TYPE:
					sprintf(outs, "%u", *(int8 *)attrib->data);
					break;
				case B_UINT16_TYPE:
					sprintf(outs, "%hu", *(int16 *)attrib->data);
					break;
				case B_UINT32_TYPE:
				case B_SIZE_T_TYPE:
					sprintf(outs, "%lu", *(int32 *)attrib->data);
					break;
				case B_UINT64_TYPE:
					sprintf(outs, "%Lu", *(int64 *)attrib->data);
					break;
				case B_FLOAT_TYPE:
					sprintf(outs, "%f", *(float *)attrib->data);
					break;
				case B_DOUBLE_TYPE:
					sprintf(outs, "%lf", *(double *)attrib->data);
					break;
				case B_OBJECT_TYPE:
				case B_POINTER_TYPE:
					sprintf(outs, "%p", (unsigned char *)(*attrib->data));
					break;
				case B_POINT_TYPE:
					sprintf(outs, "X: %g, Y: %g", (*((BPoint *)attrib->data)).x,
						(*((BPoint *)attrib->data)).y);
					break;
				case B_RECT_TYPE:
					sprintf(outs, "left: %g, top: %g, right: %g, bottom: %g",
						(*((BRect *)attrib->data)).left,
						(*((BRect *)attrib->data)).top,
						(*((BRect *)attrib->data)).right,
						(*((BRect *)attrib->data)).bottom);
					break;
				case B_RGB_COLOR_TYPE:
					sprintf(outs, "red: %u, green: %u, blue: %u, alpha: %u",
						(*((rgb_color *)attrib->data)).red,
						(*((rgb_color *)attrib->data)).green,
						(*((rgb_color *)attrib->data)).blue,
						(*((rgb_color *)attrib->data)).alpha);
					break;
				default:
					error = AE_UNEXPECTED_ERROR;
					out = GetErrorString();
			}
			if (outs[0] != 0)
				out = outs;
		}
	}

	if (fixed && (typeSize < attrib->size))
		out += " <more>";

	return out;
}

// +-------------------------------------------------------------------------+
// | SETTER METHODS                                                          |
// +-------------------------------------------------------------------------+

// Type's view style
bool AttributeItem::SetTypeViewStyle(TypeView typeView)
{
	if (typeView == AttributeItem::typeView)
		return true;
	if ((typeView == TV_DEFAULT_VIEW) || (typeView == TV_HEX_VIEW) ||
		(typeView == TV_STRING_VIEW)) {
		AttributeItem::typeView = typeView;
		typeString = GetTypeString();
		return true;
	}
	else
		return false;
}

// Data endianess
bool AttributeItem::SetEndian(DataEndian dataEndian)
{
	if (dataEndian == AttributeItem::dataEndian)
		return true;
	if ((dataEndian == DE_INTEL) || (dataEndian == DE_POWERPC)) {
		AttributeItem::dataEndian = dataEndian;
		return true;
	}
	else
		return false;
}

// Data's view style
bool AttributeItem::SetDataViewStyle(DataView dataView)
{
	if (dataView == AttributeItem::dataView)
		return true;
	if ((dataView == DV_DEFAULT_VIEW) || (dataView == DV_HEX_VIEW) ||
		(dataView == DV_STRING_VIEW) || (dataView == DV_C_STRING_VIEW)) {
		AttributeItem::dataView = dataView;
		dataString = GetDataString();
		return true;
	}
	else
		return false;
}
