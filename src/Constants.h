// AttribExplorer alpha 1.76
// Copyright (C) 2001-2004 Gabriele Biffi <gabriele@biffuz.it>
//
// Various constants.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// Please keep code inside this limit! --------------------------------------+


#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINRECT_START_X				30
#define WINRECT_START_Y				30
#define WINRECT_WIDTH				400
#define WINRECT_HEIGHT				200
#define WINRECT_STEP_X				20
#define WINRECT_STEP_Y				20

#define MAX_CASCADED_WINDOWS		10

#define MSG_OPEN_NODE				'OPOF'
#define MSG_REFRESH_LIST			'RFSL'
#define MSG_KILL_ME					'KLME'
#define MSG_SELECTION_CHANGED		'SLCH'

#define MSG_EDIT_CUT				'ECUT'
#define MSG_EDIT_COPY				'ECPY'
#define MSG_EDIT_PASTE				'EPST'
#define MSG_EDIT_DELETE				'ERMV'
#define MSG_EDIT_SELECT_ALL			'SLAL'
#define MSG_EDIT_SELECT_NONE		'SLNO'
#define MSG_EDIT_INVERT_SELECTION	'SLIN'

#define MSG_DATA_VIEW_INTEL			'DX86'
#define MSG_DATA_VIEW_POWERPC		'DPPC'

#define MSG_DATA_VIEW_DEFAULT		'VDEF'
#define MSG_DATA_VIEW_HEX			'VHEX'
#define MSG_DATA_VIEW_STRING		'VSTR'
#define MSG_DATA_VIEW_C_STRING		'VCST'

#define MSG_TYPE_VIEW_DEFAULT		'VTDF'
#define MSG_TYPE_VIEW_HEX			'VTHX'
#define MSG_TYPE_VIEW_STRING		'VTST'

#define MSG_WINDOWS_SHOW_LIST		'WINL'
#define MSG_ACTIVATE_WINDOW			'AWIN'

#define COLUMN_NAME_WIDTH			120
#define COLUMN_TYPE_WIDTH			50
#define COLUMN_SIZE_WIDTH			30

#define kAppMIME					"application/x-vnd.Biffuz-AttribExplorer"
#define kAppName					"AttribExplorer"
#define kAppAuthor					"Gabriele Biffi"
#define kAppCopyright				"Copyright (C) 2001-2004 Gabriele Biffi"
#define kAppVersion					"alpha 1.76"
#define kAppURL						"http://www.biffuz.it/"
#define kAppEMail					"gabriele@biffuz.it"
#define kAttribType					"application/x-attribexplorer-attribute"

#endif // CONSTANTS_H
