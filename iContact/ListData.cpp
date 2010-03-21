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

#include <windows.h>
#include <vector>

#include "iContact.h"
#include "ListData.h"
#include "PhoneUtils.h"
#include "FileUtils.h"
#include "Macros.h"

TCHAR szListCacheFn[MAX_PATH];
std::vector<DataItem> vListItems;
int listSelectedIndex = -1;

HANDLE hCache = NULL;

HBITMAP listHBitmap = NULL;
UINT listNBitmapHeight = 0;
UINT listNBitmapWidth = 0;

HRESULT listLoadFromCache(DataItem * parent, ScreenDefinition screen, 
                 CSettings * pSettings, bool useCache);

HRESULT listPopulate(DataItem * parent, ScreenDefinition screen,
                     CSettings * pSettings);

// straight to memory
void addDataItem(DataItem * data);

// write to file hCache
void writeDataItem(DataItem * data);

void ListClear(void) {
    vListItems.clear();

    BOOL result = true;
    if (listHBitmap)
        result = DeleteObject((HGDIOBJ)listHBitmap);
    listHBitmap = NULL;
    listNBitmapHeight = 0;
    listNBitmapWidth = 0;
    listSelectedIndex = -1;
}

HRESULT ListLoad(DataItem * parent, ScreenDefinition screen, 
                 CSettings * pSettings, bool useCache) {

    // this list should be cached
    if (screen.filename != NULL) {
        return listLoadFromCache(parent, screen, pSettings, useCache);
    }

    // this list won't be cached
    else {
        ListClear();
        ASSERT(screen.fnPopulate != NULL);
        return screen.fnPopulate(parent, addDataItem, pSettings);
    }
}

void addDataItem(DataItem * data) {
    vListItems.push_back(*data);
}

void writeDataItem(DataItem * data) {
    DWORD dwNumberOfBytesWritten;
    WriteFile(hCache, data, sizeof(DataItem), &dwNumberOfBytesWritten, NULL);
}

HRESULT listLoadFromCache(DataItem * parent, ScreenDefinition screen,
                         CSettings * pSettings, bool useCache) {

    HRESULT hr;
    HANDLE hCache = INVALID_HANDLE_VALUE;

    GetCurDirFilename(szListCacheFn, screen.filename);

    if (useCache) {
        // Try to load data from cache
        hCache = CreateFile(szListCacheFn, GENERIC_READ, 0, NULL, 
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (hCache == INVALID_HANDLE_VALUE) {
        CHR(listPopulate(parent, screen, pSettings));

        hCache = CreateFile(szListCacheFn, GENERIC_READ, 0, NULL, 
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    DWORD dwFileSize = GetFileSize(hCache, NULL);

    ListClear();
    DWORD dwBytesRead;
    SetFilePointer(hCache, 0, NULL, FILE_BEGIN);
    DataItem data;
    while (ReadFile(hCache, &data, sizeof(DataItem), &dwBytesRead, NULL)) {
        CBR(dwBytesRead == sizeof(DataItem));
        vListItems.push_back(data);
    }

    hr = S_OK;

Error:
    CloseHandle(hCache);
    return hr;
}

HRESULT listPopulate(DataItem * parent, ScreenDefinition screen, 
                     CSettings * pSettings) {
    HRESULT hr;

    ASSERT(screen.fnPopulate != NULL);

    GetCurDirFilename(szListCacheFn, screen.filename);

    hCache = CreateFile(szListCacheFn, GENERIC_WRITE, 0, NULL, 
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CBR(hCache != INVALID_HANDLE_VALUE);

    hr = screen.fnPopulate(parent, writeDataItem, pSettings);
    CHR(hr);

    hr = S_OK;

Error:
    CloseHandle(hCache);
    hCache = NULL;
    return hr;
}

int GetItemCount(void) {
    return vListItems.size();
}

DataItem GetCurrentItem(void) {
    return GetItem(listSelectedIndex);
}

DataItem GetItem(int index) {
    ASSERT(index >= 0 && index < (int)vListItems.size());
    return vListItems[index];
}

bool CanSelectItem(int index) {
	if (index < 0 || index >= (int)vListItems.size())
		return false;

    int t = vListItems[index].type;
    return t == diListItem
        || t == diUrl
        || t == diPhone
        || t == diEmail
        || t == diDetailsButton
        || t == diEditButton
        || t == diCallButton
        || t == diSmsButton
        || t == diSaveContactButton
        || t == diCreateShortcutButton
		|| t == diRemoveShortcutButton;
}

DataItem SelectItem(int index) {
    listSelectedIndex = index;
    return GetCurrentItem();
}

int SelectFirstItem() {
	int index = 0;
    while (index < (int)vListItems.size() && !CanSelectItem(index))
        index++;

	// Select nothing (-1) if nothing can be selected
	listSelectedIndex = index < (int)vListItems.size() ? index : -1;

	return listSelectedIndex;
}

int SelectLastItem() {
	int index = vListItems.size();
    while (index >= 0 && !CanSelectItem(index))
        index--;

	// index will be -1 if nothing available; that's what we want.
	// -1 means select nothing.
	listSelectedIndex = index;

	return listSelectedIndex;
}

int SelectPreviousItem(int defaultIndex, bool byGroup) {

    int index 
		= listSelectedIndex == 0 ? 0
		: listSelectedIndex > 0 ? listSelectedIndex - 1
        : defaultIndex;
    
    while (
        index >= 0 
        && (
               (byGroup && !IsItemNewGroup(index))
            || !CanSelectItem(index)
        )
    )
        index--;

    listSelectedIndex = index >= 0 ? index : -1;

    return listSelectedIndex;
}

int SelectNextItem(int defaultIndex, bool byGroup) {

    int index 
		= listSelectedIndex == vListItems.size() - 1 ? vListItems.size() - 1
		: listSelectedIndex >= 0 ? listSelectedIndex + 1
        : defaultIndex;
    
    while (
        index < (int)vListItems.size() 
        && (
               (byGroup && !IsItemNewGroup(index))
            || !CanSelectItem(index)
        )
    )
        index++;

    listSelectedIndex = index < (int)vListItems.size() ? index : -1;

    return listSelectedIndex;
}

int GetCurrentItemIndex(void) {
    return listSelectedIndex;
}

void UnselectItem(void) {
    listSelectedIndex = -1;
}

bool IsItemNewGroup(int index) {
    ASSERT(index < (int)vListItems.size());

    if (index == 0)
        return vListItems[index].iGroup != 0;

    return vListItems[index - 1].iGroup != vListItems[index].iGroup;
}

bool IsItemNewType(int index) {
    ASSERT(index < (int)vListItems.size());

    if (index == 0)
        return true;

    return vListItems[index - 1].type != vListItems[index].type;
}

int CountSameTypeAs(int index, int limit) {
    ASSERT(index < (int)vListItems.size());
    DataItemType t = vListItems[index].type;

    int i = index + 1;
    while (i < (int)vListItems.size() && vListItems[i].type == t && i - index < limit)
        i++;

    return i - index;
}

HBITMAP GetHBitmap(DataItem * parent, ScreenDefinition screen, int size) {
    if (NULL == listHBitmap && NULL != screen.fnGetHBitmap) {
        listNBitmapWidth = size;
        listNBitmapHeight = size;
        screen.fnGetHBitmap(parent,
            &listHBitmap, &listNBitmapWidth, &listNBitmapHeight); 
    }
    return listHBitmap;
}

int GetHBitmapWidth(void) {
    return listNBitmapWidth;
}

int GetHBitmapHeight(void) {
    return listNBitmapHeight;
}