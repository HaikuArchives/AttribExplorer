// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// Exception definition. Instances of this class are thrown by various
// methods in AttribExplorer's classes.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


#ifndef EXCEPTION_H
#define EXCEPTION_H

// BeOS headers
#include <String.h>

class Exception {
	public:
		BString		message;
		int			reason;
					Exception(BString message, int reason = 0)
						{ Exception::message = message; Exception::reason = reason; };
					Exception(int reason, BString message = "")
						{ Exception::message = message; Exception::reason = reason; };
};

#endif // EXCEPTION_H
