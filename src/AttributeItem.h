// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// AttributeItem definition.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


#ifndef ATTRIBUTEITEM_H
#define ATTRIBUTEITEM_H

// BeOS headers
#include <ListItem.h>
#include <String.h>

// Project headers
#include "Attribute.h"

// Enum for errors
typedef enum {
	AE_NO_ERROR				= ARE_NO_ERROR,
	AE_NO_INFO				= ARE_NO_INFO,
	AE_READ_ERROR			= ARE_READ_ERROR,
	AE_INITIALIZE_ERROR		= ARE_INITIALIZE_ERROR,
	AE_UNEXPECTED_ERROR		= ARE_UNEXPECTED_ERROR,
	AE_UNKNOWN_ERROR		= ARE_UNKNOWN_ERROR,
	AE_UNHANDLED_TYPE,
	AE_UNKNOWN_TYPE
} AttributeError;

// Enum for data view styles
typedef enum {
	DV_DEFAULT_VIEW,
	DV_HEX_VIEW,
	DV_STRING_VIEW,
	DV_C_STRING_VIEW
} DataView;

// Enum for type view styles
typedef enum {
	TV_DEFAULT_VIEW,
	TV_HEX_VIEW,
	TV_STRING_VIEW
} TypeView;

// Enum for data endianess. DE_DEFAULT is set to be the same as the current
// platform
typedef enum {
	#ifdef __INTEL__
		DE_DEFAULT = 0,
	#else
		DE_DEFAULT = 1,
	#endif
	DE_INTEL = 0,
	DE_POWERPC = 1
} DataEndian;

// Attrbute list item
class AttributeItem : public BListItem {
	private:

		// Private static methods --------------------------------------------

		static BString	GetTypeAsString(type_code type);
		static void		ConvHex(const unsigned char *data, BString &out,
							size_t len = 1);
		static void		ConvString(const unsigned char *data, BString &out,
							size_t len = 1);
		static void		ConvCString(const unsigned char *data, BString &out,
							size_t len = 1);

		// Initialization ----------------------------------------------------

		void			Initialize(Attribute *data,
							AttributeError error = AE_NO_ERROR,
							DataEndian dataEndian = DE_DEFAULT,
							DataView dataView = DV_DEFAULT_VIEW,
							TypeView typeView = TV_DEFAULT_VIEW);

		// Attribute informations --------------------------------------------

		Attribute		*attrib;
		Attribute		*original;

		// How informations are going to represented -------------------------

		DataEndian		dataEndian;
		DataView		dataView;
		TypeView		typeView;

		// String representations of attribute informations ------------------

		BString			typeString;
		BString			dataString;

		// Status informations -----------------------------------------------

		AttributeError	error;
		bool			initialized;

	public:

		// Constructors, destructor, and drawing -----------------------------

						AttributeItem(Attribute *data,
							AttributeError error = AE_NO_ERROR,
							DataEndian dataEndian = DE_DEFAULT,
							DataView dataView = DV_DEFAULT_VIEW,
							TypeView typeView = TV_DEFAULT_VIEW);
						~AttributeItem();
		void			DrawItem(BView *owner, BRect frame,
							bool complete = false);

		// Public static methods ---------------------------------------------

		static BString			GetErrorString(AttributeError error);
		static BString			GetTypeString(type_code type,
									TypeView typeView = TV_DEFAULT_VIEW);
		static bool				IsTypeKnown(type_code type);
		static bool				IsTypeHandled(type_code type);
		static size_t			GetTypeSize(type_code type,
									bool *fixed = NULL);

		// Getter methods ----------------------------------------------------

		// Status
		inline bool				IsInitialized(void)
									{ return initialized; };
		inline AttributeError	GetError(void)
									{ return error; };

		// Attribute informations
		inline Attribute*		GetAttribute(void)
									{ return attrib; };
		inline Attribute*		GetOriginal(void)
									{ return original; };

		// Information awareness
		inline size_t			GetTypeSize(bool *fixed = NULL)
									{ return GetTypeSize(attrib->type, fixed); };
		inline bool				IsTypeKnown(void)
									{ return IsTypeKnown(attrib->type); };
		inline bool				IsTypeHandled(void)
									{ return IsTypeHandled(attrib->type); };
		
		// Representation style
		inline TypeView			GetTypeViewStyle(void)
									{ return typeView; };
		inline DataView			GetDataViewStyle(void)
									{ return dataView; };
		inline DataEndian		GetDataEndian(void)
									{ return dataEndian; };

		// Representation
		inline BString			GetErrorString(void)
									{ return GetErrorString(error); };
		inline BString			GetTypeString(void)
									{ return GetTypeString(attrib->type, typeView); };
		BString					GetDataString(void);

		// Setter methods ----------------------------------------------------

		bool					SetTypeViewStyle(TypeView typeView);
		bool					SetEndian(DataEndian endian);
		bool					SetDataViewStyle(DataView dataView);
};

#endif // ATTRIBUTEITEM_H
