// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// Attribute class. Used to store name, type, data, and (if occured) reading
// errors of a single attribute.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


// Standard C headers
#include <malloc.h>

// Project headers
#include "Attribute.h"
#include "Exception.h"

// Class constructors and destructor

Attribute::Attribute(BString name,
	type_code type,
	size_t size,
	unsigned char* data,
	AttribReadError error = ARE_NO_ERROR)
{
	Attribute::data = NULL;
	Attribute::name = name;
	Attribute::type = type;
	CopyData(data, size);
	Attribute::error = error;
}

Attribute::Attribute(const Attribute& src)
{
	data = NULL;
	Copy(src);
}

Attribute::Attribute(const BMessage* const msg)
{
	unsigned char *buffer;
	size_t num_bytes;

	name = "";
	type = 0;
	size = 0;
	data = NULL;
	error = ARE_INITIALIZE_ERROR;

	if (msg->FindString("name", &name) != B_OK)
		throw new Exception("name not found");

	if (msg->FindInt32("type", 0, (int32*)&type) != B_OK)
		throw new Exception("type not found");

	if (msg->FindInt32("size", 0, (int32*)&size) != B_OK)
		throw new Exception("size not found");

	if (size > 0) {
		if (msg->FindData("data", type, (const void**)&buffer,
			(ssize_t*)&num_bytes) != B_OK)
			throw new Exception("data not found");
		if (num_bytes < size)
			size = num_bytes;
		CopyData(buffer, size);
	}

	Attribute::error = ARE_NO_ERROR;
}

Attribute::~Attribute()
{
	if (data)
		delete[] data;
}

// Copy
void Attribute::CopyData(const unsigned char *src, const size_t size)
{
	if (src == data)
		return;
	if (data)
		delete[] data;
	if ((src == NULL) || (size == 0)) {
		data = NULL;
		Attribute::size = 0;
		return;
	}
	data = new unsigned char[size];
	memcpy((void*)data, (void*)src, size);
	Attribute::size = size;
}

void Attribute::Copy(const Attribute& src)
{
	if (&src == this)
		return;
	Attribute::name = src.name;
	Attribute::type = src.type;
	CopyData(src.data, src.size);
	Attribute::error = src.error;
}

Attribute& Attribute::operator =(const Attribute& src)
{
	Copy(src);
	return *this;
}

// Turn into a BMessage
BMessage* Attribute::GetMessage(void)
{
	BMessage *msg = new BMessage();
	msg->AddString("name", name);
	msg->AddInt32("type", type);
	msg->AddInt32("size", size);
	if ((size > 0) && (data != NULL))
		msg->AddData("data", type, data, size);
	return msg;
}
