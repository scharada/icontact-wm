/*******************************************************************
This file is part of iContact.

iContact is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

iContact is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with iContact.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************/

#pragma once

#include "CSettings.h"

#define SZ_APP_NAME             TEXT("iContact")

#define IDI_ICONTACT	        101

#define PRIMARY_TEXT_LENGTH     64
#define SECONDARY_TEXT_LENGTH   16

enum DataItemType {
    diNothing, diListItem, diText, diUrl,
    diName, diCompany, diPhone, diEmail,
    diDetailsButton, diEditButton, diCallButton, 
    diSmsButton, diSaveContactButton,
};

/// List DATA
struct DataItem {
	int		        ID;
	LONG    	    oId;
	DataItemType    type;
	TCHAR           szPrimaryText[PRIMARY_TEXT_LENGTH];
	TCHAR		    szSecondaryText[SECONDARY_TEXT_LENGTH];
    UINT            iGroup;
    bool            isFavorite;
    bool            isMissed;
};

struct ScreenDefinition {
    TCHAR * filename;
    int parent;
    bool hasMenus;
    HRESULT (*fnPopulate)(DataItem * parent, void (*adder)(DataItem*),
        CSettings * pSettings);
    HRESULT (*fnGetTitle)(DataItem * parent, TCHAR * buffer, int cchDest,
        CSettings * pSettings);
    HRESULT (*fnGetGroup)(DataItem * data, TCHAR * buffer, int cchDest,
        CSettings * pSettings);
    HRESULT (*fnClick)(DataItem * data, float x, int * newScreen,
        CSettings * pSettings);
    HRESULT (*fnAdd)(void);
    HRESULT (*fnToggleFavorite)(DataItem * data, CSettings * pSettings);
    HRESULT (*fnGetHBitmap)(DataItem * data, HBITMAP * phBitmap, 
                              UINT * puWidth, UINT * puHeight);
};

struct CmdLineArg {
    TCHAR * arg;
    WPARAM wparam;
};

enum TransitionType {
    ttSlideLeft,
    ttSlideRight,
    ttKeyboardExpand,
    ttKeyboardShrink,
};

enum PopupType {
    ptKeyboard,
};

struct decodeUINT {                             // Structure associates
    UINT Code;                                  // messages 
                                                // with a function. 
    LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
};

struct HistoryItem {
    int screen;
    int scrolled;
    int selectedIndex;
    DataItem data;
};
