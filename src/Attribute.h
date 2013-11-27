// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// Attribute class definition.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

// BeOS headers
#include <String.h>
#include <Message.h>

// Enum for read errors
typedef enum {
	ARE_NO_ERROR,
	ARE_NO_INFO,
	ARE_READ_ERROR,
	ARE_INITIALIZE_ERROR,
	ARE_UNEXPECTED_ERROR,
	ARE_UNKNOWN_ERROR
} AttribReadError;

// Attribute informations
// Note: in the constructors and in the "=" operator, the data will be copied
// in a new memory block, wich will be freed in the destructor.
class Attribute {
	private:
		// Copy methods
		void			Copy(const Attribute& src);
	public:
		// Constructors and destructor
						Attribute(BString name,
							type_code type,
							size_t size,
							unsigned char* data,
							AttribReadError error = ARE_NO_ERROR);
						Attribute(const Attribute &src);
						Attribute(const BMessage* const msg);
						~Attribute();
		// Copy
		Attribute&		operator =(const Attribute &src);
		void			CopyData(const unsigned char *src, const size_t size);
		// Turn attribute informations into a BMessage (very useful for
		// load/save and copy/paste functions)
		BMessage		*GetMessage(void);
		// Attribute informations
		BString			name;			// Name
		type_code		type;			// Type code
		size_t			size;			// Size of data (in bytes)
		unsigned char	*data;			// Pointer to data
		AttribReadError	error;			// Read error
};

#endif // ATTRIBUTE_H
