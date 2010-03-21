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

#include "stdafx.h"
#include "macros.h"
#include "resourceppc.h"
#include <pimstore.h>

// These functions are for categories
HRESULT PoomCategoriesPopulate(DataItem * parent, void (*adder)(DataItem*),
                     CSettings * pSettings);
HRESULT PoomCategoriesGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                     CSettings * pSettings);
HRESULT PoomCategoriesClick(DataItem * data, float x, 
                  int * newScreen, CSettings * pSettings);

// These functions are for the main list of contacts
HRESULT PoomPopulate(DataItem * parent, void (*adder)(DataItem*),
                     CSettings * pSettings);
HRESULT PoomGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                     CSettings * pSettings);
HRESULT PoomGetGroup(DataItem * data, TCHAR * buffer, int cchDest,
                     CSettings * pSettings);
HRESULT PoomClick(DataItem * data, float x, 
                  int * newScreen, CSettings * pSettings);
HRESULT PoomAddItem();

// These functions are for a contact details screen
HRESULT PoomDetailsPopulate(DataItem * parent, void (*adder)(DataItem*),
                            CSettings * pSettings);
HRESULT PoomDetailsGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                     CSettings * pSettings);
HRESULT PoomDetailsClick(DataItem * data, float x,
                         int * newScreen, CSettings * pSettings);
HRESULT PoomDetailsToggleFavorite(DataItem * data, CSettings * pSettings);

HRESULT PoomDetailsLoadBitmap(DataItem * data, HBITMAP * phBitmap,
                              UINT * puWidth, UINT * puHeight);
