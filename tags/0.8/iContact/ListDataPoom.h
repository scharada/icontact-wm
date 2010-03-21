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

#include <pimstore.h>

#include "CSettings.h"
#include "ListData.h"

class ListDataPoom : public ListData {
private:
    IPOutlookApp2 * polApp;
    IFolder * pCurrFldr;
    IPOutlookItemCollection * pItemCol;
	bool _bOnlyFavorites;

    void _construct(CSettings *, bool);

    HRESULT _initPoom();

    HRESULT _loadBitmap(int size);

public:

    ListDataPoom(CSettings *);
	ListDataPoom(CSettings *, bool);

    void Release(void);
    HRESULT Populate(void);
    HRESULT PopulateDetailsFor(int);

    void ToggleFavorite();
    void AddItem();
    HRESULT DisplayItem();
    void EditItem();
};
