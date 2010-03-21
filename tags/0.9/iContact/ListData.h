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

HRESULT ListLoadBitmap(int size);

void ListClear(void);
HRESULT ListLoad(DataItem * parent, ScreenDefinition screen,
                 CSettings * pSettings, bool useCache = false);

HBITMAP GetHBitmap(DataItem * parent, ScreenDefinition screen, int size);
int GetHBitmapWidth();
int GetHBitmapHeight();

// Methods dealing with main items
int GetItemCount(void);
int GetCurrentItemIndex();
DataItem GetCurrentItem();
DataItem GetItem(int index);
bool CanSelectItem(int index);
DataItem SelectItem(int index);
void UnselectItem();
void GetItemGroup(int index, TCHAR * pszGroup);
bool IsItemNewGroup(int index);
bool IsItemNewType(int index);
int CountSameTypeAs(int index);
int SelectPreviousItem(int defaultIndex, bool byGroup = false);
int SelectNextItem(int defaultIndex, bool byGroup = false);
