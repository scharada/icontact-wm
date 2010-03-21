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

#define MAX_LIST_ITEMS 2000
#define MAX_SUBLIST_ITEMS 20

#define PRIMARY_TEXT_LENGTH 64
#define SECONDARY_TEXT_LENGTH 16

enum DataItemType {
    diNothing, diCategory, diText, diUrl,
    diName, diCompany, diPhone, diSms, diEmail,
    diDetailsButton, diEditButton, diCallButton, 
    diSmsButton, diSaveContactButton,
};

/// List DATA
struct Data {
	int		    ID;
	LONG    	oId;
	TCHAR       szPrimaryText[PRIMARY_TEXT_LENGTH];
	TCHAR		szSecondaryText[10];
    TCHAR       wcGroup;
	int		    nPrimaryTextLength;
	int			nSecondaryTextLength;
    bool        isFavorite;
    COLORREF    rgbPrimaryText;
};

struct DataDetail {
	TCHAR           text[PRIMARY_TEXT_LENGTH];
	TCHAR	        label[SECONDARY_TEXT_LENGTH];
    TCHAR           arg1[PRIMARY_TEXT_LENGTH];
    TCHAR           arg2[PRIMARY_TEXT_LENGTH];
	DataItemType    type;
};

class ListData {
protected:
    CSettings * _settings;
    bool _canFavorite;
    bool _canAdd;

    Data * _items;
    int _arrayLength;
    int _listCounter;
    int _currentItemIndex;

    HBITMAP _hBitmap;
    UINT _nBitmapHeight;
    UINT _nBitmapWidth;

    DataDetail * _detailItems;
    int _currentDetailIndex;
    int _itemDetailCount;
    const TCHAR * _currentDetailTitle;

    void _addDetail(DataItemType, 
        const TCHAR * text, 
        const TCHAR * label = NULL, 
        const TCHAR * arg1 = NULL, 
        const TCHAR * arg2 = NULL);

    bool _addListItem(int id, 
        const TCHAR * tszPrimary, 
        const TCHAR * tszSecondary, 
        TCHAR tszGroup, 
        LONG oId, 
        COLORREF rgbPrimary);

    virtual HRESULT _loadBitmap (int size);

public:

    ListData();
    ListData(CSettings * pSettings);
    ~ListData(void);

    // Overridable methods
    virtual void Release();
    virtual HRESULT Populate();

    virtual void ToggleFavorite();
    virtual void AddItem();
    virtual HRESULT DisplayItem();
    virtual void EditItem();

    virtual HRESULT PopulateDetailsFor(int id);
    virtual void GetItemGroup(int index, TCHAR * pszGroup);

    // Property accessors
    bool CanAdd(void);
    bool CanFavorite(void);
    HBITMAP GetHBitmap(int size);
    int GetHBitmapWidth();
    int GetHBitmapHeight();

    // Methods dealing with main items
    int GetItemCount(void);
    int GetCurrentItemIndex();
    Data GetCurrentItem();
    Data GetItem(int index);
    Data SelectItem(int index);
    Data SelectPreviousItem(int defaultIndex, bool byGroup = false);
    Data SelectNextItem(int defaultIndex, bool byGroup = false);
    bool IsItemNewGroup(int index);
    void UnselectItem();

    // Methods dealing with item details
    HRESULT PopulateDetails();
    int GetItemDetailCount(void);
    int GetCurrentDetailIndex(void);
    const TCHAR * GetCurrentDetailTitle(void);
    bool SelectDetail(int index);
    void IncrementDetailIndex(int by = 1);
    HRESULT PerformCurrentDetailAction(int x);
    DataDetail GetItemDetail(int index);
};
