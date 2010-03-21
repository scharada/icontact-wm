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

#include <stdafx.h>
#include "iContact.h"
#include "ListData.h"
#include "PhoneUtils.h"

ListData::ListData() {
    this->_detailItems = new DataDetail[MAX_SUBLIST_ITEMS];
    this->_arrayLength = 0;
    this->_currentDetailIndex = -1;
    this->_currentItemIndex = -1;
    this->_itemDetailCount = 0;
    this->_canFavorite = false;
    this->_canAdd = false;
    this->_hBitmap = NULL;
    this->_nBitmapHeight = 0;
    this->_nBitmapWidth = 0;
	this->_listCounter = 0;
}

ListData::ListData(CSettings * pSettings) {
    this->_settings = pSettings;
}

ListData::~ListData(void) {
    this->Release();
    delete [] this->_items;
    delete [] this->_detailItems;
}

Data ListData::GetCurrentItem() {
    return this->_items[this->_currentItemIndex];
}

Data ListData::GetItem(int index) {
    return this->_items[index];
}

Data ListData::SelectItem(int index) {
    this->_currentItemIndex = index;
    return this->_items[index];
}

Data ListData::SelectPreviousItem(int defaultIndex, bool byGroup) {
    int index = this->_currentItemIndex;

	if (index < 0) {
		index = defaultIndex;
	}
    else if (index == 0) {
    }
    else if (byGroup) {
        // Jump to the previous group
        index--;
        while (index > 0 && 
            !this->IsItemNewGroup(index))
            index--;
    }
    else {
		index = max(0, index - 1);
	}

    this->_currentItemIndex = index;
    return this->_items[index];
}

Data ListData::SelectNextItem(int defaultIndex, bool byGroup) {
    int index = this->_currentItemIndex;

    if (index < 0) {
		index = defaultIndex;
	}
    else if (byGroup) {
        // Jump to the next group
        int max = this->_listCounter - 1;

        if (index < max) {
            index++;
            while (index < max && 
                !this->IsItemNewGroup(index))
                index++;
        }
    }
	else {
		index = min(this->_listCounter - 1, index + 1);
	}

    this->_currentItemIndex = index;
    return this->_items[index];
}

int ListData::GetCurrentItemIndex() {
    return this->_currentItemIndex;
}

void ListData::UnselectItem() {
    this->_currentItemIndex = -1;
    if (this->_hBitmap)
        DeleteObject((HGDIOBJ)this->_hBitmap);
    this->_hBitmap = NULL;
}

void ListData::GetItemGroup(int index, TCHAR * pszGroup) {
    pszGroup[0] = this->_items[index].wcGroup;
    pszGroup[1] = 0;
}

bool ListData::IsItemNewGroup(int index) {
	if (index == 0)
		return true;

    return this->_items[index - 1].wcGroup != this->_items[index].wcGroup;
}

int ListData::GetCurrentDetailIndex() {
    return this->_currentDetailIndex;
}

const TCHAR * ListData::GetCurrentDetailTitle() {
    return this->_currentDetailTitle;
}

HRESULT ListData::PerformCurrentDetailAction(int pos) {
    HRESULT hr = S_OK;
    DataDetail detail = this->_detailItems[this->_currentDetailIndex];
    TCHAR * itemText = this->_items[this->_currentItemIndex].szPrimaryText;

    // We have to clear out the bitmap here because 
    // EditItem or DisplayItem could change it
    if (this->_hBitmap)
        ::DeleteObject((HGDIOBJ)this->_hBitmap);
    this->_hBitmap = NULL;

    switch (detail.type) {
        case diEditButton:
            this->EditItem();
		    break;

        case diDetailsButton:
            hr = this->DisplayItem();
		    break;

        case diCallButton:
            Call(detail.arg1, detail.arg2);
            break;

        case diSmsButton:
            SendSMS(detail.arg1, detail.arg2);
            break;

        case diSaveContactButton:
            AddContactByNumber(detail.arg1);
            break;

        case diPhone:
            if (pos > 1)
                SendSMS(detail.text, itemText);
            else 
                Call(detail.text, itemText);

		    break;

        case diEmail:
            SendEMail(this->_settings->email_account, detail.text);
		    break;

        case diUrl:
            OpenURL(detail.text);
            break;

        default:
            hr = ERROR_NO_MATCH;
    }

    return hr;
}

bool ListData::SelectDetail(int index) {
    if (index == -1) {
        this->_currentDetailIndex = index;
        return true;
    }

    if (index >= this->_itemDetailCount) {
        return false;
    }

    DataDetail detail = this->_detailItems[index];
    DataItemType dit = detail.type;

    if (dit == diEditButton
        || dit == diDetailsButton
        || dit == diCallButton
        || dit == diSmsButton
        || dit == diSaveContactButton
        || dit == diPhone
        || dit == diEmail
        || dit == diUrl) {
        this->_currentDetailIndex = index;
        return true;
    }

    return false;
    
}

void ListData::IncrementDetailIndex(int by) {
    int max = this->_itemDetailCount;
        
    // there may be no actions; in this case just don't do anything
    if (!max)
        return;

    // This turns "-1" into "max-1" to fix %
    by += max;

    int index = (this->_currentDetailIndex + by) % max;
    int count = 1;
    DataItemType type = this->_detailItems[index].type;

    // These types cannot be "selected"
    while (count++ < max && 
        (type == diNothing
        || type == diCompany
        || type == diCategory
        || type == diText
        || type == diName)) {

        index = (index + by) % max;
        type = this->_detailItems[index].type;
    }

    if (count <= max)
        this->_currentDetailIndex = index;
}

DataDetail ListData::GetItemDetail(int index) {
    return this->_detailItems[index];
}

int ListData::GetItemCount() {
    return this->_listCounter;
}

int ListData::GetItemDetailCount() {
    return this->_itemDetailCount;
}

bool ListData::_addListItem(int id, const TCHAR * tszPrimary, 
    const TCHAR * tszSecondary, TCHAR wcGroup, LONG oId, COLORREF rgbPrimary) {

    int i = this->_listCounter;

    if (i >= this->_arrayLength)
        return false;

    this->_items[i].ID = id;
    this->_items[i].oId = oId;
    this->_items[i].wcGroup = wcGroup;
    this->_items[i].rgbPrimaryText = rgbPrimary;

    if (tszPrimary)
        StringCchCopy(this->_items[i].szPrimaryText, PRIMARY_TEXT_LENGTH, 
        tszPrimary);

    if (tszSecondary)
        StringCchCopy(this->_items[i].szSecondaryText, 10, tszSecondary);

    this->_items[i].nPrimaryTextLength 
        = _tcslen(this->_items[i].szPrimaryText);

	this->_items[i].nSecondaryTextLength 
        = _tcslen(this->_items[i].szSecondaryText);
	
    this->_listCounter++;
    return true;
}

void ListData::_addDetail(DataItemType type, const TCHAR * text, 
    const TCHAR * label, const TCHAR * arg1, const TCHAR * arg2) {

    int c = this->_itemDetailCount;
    if (c == MAX_SUBLIST_ITEMS)
        return;

    if (type == diNothing) {
        this->_itemDetailCount++;
        this->_detailItems[c].type = type;
        return;
    }

    else if (text == NULL || _tcslen(text) == 0) {
        return;
    }

    else {
        this->_itemDetailCount++;
        this->_detailItems[c].type = type;

        // primary text
        StringCchCopy(this->_detailItems[c].text, 
            PRIMARY_TEXT_LENGTH, text);

        // label
        if (label == NULL || _tcslen(label) == 0)
            this->_detailItems[c].label[0] = 0;

        else 
            StringCchCopy(this->_detailItems[c].label, 
                SECONDARY_TEXT_LENGTH, label);

        // arg1
        if (arg1 == NULL || _tcslen(arg1) == 0)
            this->_detailItems[c].arg1[0] = 0;

        else 
            StringCchCopy(this->_detailItems[c].arg1, 
                PRIMARY_TEXT_LENGTH, arg1);

        // arg2
        if (arg2 == NULL || _tcslen(arg2) == 0)
            this->_detailItems[c].arg2[0] = 0;

        else 
            StringCchCopy(this->_detailItems[c].arg2, 
                PRIMARY_TEXT_LENGTH, arg2);
    }
}

HRESULT ListData::PopulateDetails() {
    return this->PopulateDetailsFor(this->_currentItemIndex);
}

HBITMAP ListData::GetHBitmap(int size) {
    if (NULL == this->_hBitmap)
        this->_loadBitmap(size);

    return this->_hBitmap;
}

int ListData::GetHBitmapWidth() {
    return this->_nBitmapWidth;
}

int ListData::GetHBitmapHeight() {
    return this->_nBitmapHeight;
}

void ListData::Release(void) {}
bool ListData::CanFavorite(void) { return this->_canFavorite; }
bool ListData::CanAdd(void) { return this->_canAdd; }
void ListData::ToggleFavorite() {}
HRESULT ListData::DisplayItem() { return S_OK; }
void ListData::EditItem() {}
void ListData::AddItem() {}
HRESULT ListData::_loadBitmap (int size) { return S_OK; }
HRESULT ListData::Populate(void) { return S_OK; }
HRESULT ListData::PopulateDetailsFor(int id) { return S_OK; }