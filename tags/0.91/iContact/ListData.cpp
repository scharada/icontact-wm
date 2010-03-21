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
#include "FileUtils.h"
#include "Macros.h"

TCHAR szListCacheFn[MAX_PATH];
HANDLE hListItems = NULL;
DataItem * pListItems = NULL;
int listLength = 0;
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
    // Create a seperate heap for ListItems
    if (hListItems == NULL) {
        hListItems = HeapCreate(NULL, 0, 0);
    }
    if (pListItems != NULL) {
        HeapFree(hListItems, NULL, pListItems);
        pListItems = NULL;
    }

    BOOL result = true;
    if (listHBitmap)
        result = DeleteObject((HGDIOBJ)listHBitmap);
    listHBitmap = NULL;
    listNBitmapHeight = 0;
    listNBitmapWidth = 0;
    listLength = 0;
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
        int fixedListSize = 32; //TODO: dynamic?
        pListItems = (DataItem *)HeapAlloc(hListItems, NULL,
            fixedListSize * sizeof(DataItem));
        
        ASSERT(screen.fnPopulate != NULL);

        return screen.fnPopulate(parent, addDataItem, pSettings);
    }
}

void addDataItem(DataItem * data) {
    memcpy(&pListItems[listLength++], data, sizeof(DataItem));
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
    listLength = dwFileSize / sizeof(DataItem);
    pListItems = (DataItem *)HeapAlloc(hListItems, NULL,
        listLength * sizeof(DataItem));

    DWORD dwBytesRead;
    SetFilePointer(hCache, 0, NULL, FILE_BEGIN);
    ReadFile(hCache, pListItems, dwFileSize, &dwBytesRead, NULL);
    CBR(dwBytesRead == dwFileSize);
    listLength = dwBytesRead / sizeof(DataItem);

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
    return listLength;
}

DataItem GetCurrentItem(void) {
    return GetItem(listSelectedIndex);
}

DataItem GetItem(int index) {
    ASSERT(index > -1 && index < listLength);
    return pListItems[index];
}

bool CanSelectItem(int index) {
	if (index < 0 || index >= listLength)
		return false;

    int t = pListItems[index].type;
    return t == diListItem
        || t == diUrl
        || t == diPhone
        || t == diEmail
        || t == diDetailsButton
        || t == diEditButton
        || t == diCallButton
        || t == diSmsButton
        || t == diSaveContactButton;
}

DataItem SelectItem(int index) {
    listSelectedIndex = index;
    return GetCurrentItem();
}

int SelectPreviousItem(int defaultIndex, bool byGroup) {

    int index = listSelectedIndex > 0 
        ? listSelectedIndex - 1
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

    int index = listSelectedIndex >= 0 
        ? listSelectedIndex + 1
        : defaultIndex;
    
    while (
        index < listLength 
        && (
               (byGroup && !IsItemNewGroup(index))
            || !CanSelectItem(index)
        )
    )
        index++;

    listSelectedIndex = index < listLength ? index : -1;

    return listSelectedIndex;
}

int GetCurrentItemIndex(void) {
    return listSelectedIndex;
}

void UnselectItem(void) {
    listSelectedIndex = -1;
}

bool IsItemNewGroup(int index) {
    return index == 0
        || pListItems[index - 1].iGroup != pListItems[index].iGroup;
}

bool IsItemNewType(int index) {
    return index == 0
        || pListItems[index - 1].type != pListItems[index].type;
}

int CountSameTypeAs(int index) {
    DataItemType t = pListItems[index].type;

    int i = index + 1;
    while (i < listLength && pListItems[i].type == t)
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