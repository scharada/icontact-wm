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
#include <phone.h>

#include "iContact.h"
#include "CSettings.h"
#include "ListData.h"

// Define this when taking screenshots to show a missed call
//#define DEBUG_SCREENSHOTS

class ListDataCallLog : public ListData {
public:
    ListDataCallLog(CSettings *);
    void Release(void);
    HRESULT Populate(void);
    HRESULT PopulateDetailsFor(int);
    void GetItemGroup(int, TCHAR *);
    void ToggleFavorite();
    HRESULT DisplayItem();
    void EditItem();
};

TCHAR _getPastTime(FILETIME ftTime, TCHAR * pszSecondary = NULL, int strLength = 0);
void _printDate(TCHAR * str, int strLength, FILETIME ft);
void _printDuration(TCHAR * str, int strLength, FILETIME ftStart,
                    FILETIME ftEnd, CSettings * pSettings);