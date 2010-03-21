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

#include "stdafx.h"
#include "ListDataCallLog.h"
#include "windows.h"
#include "PhoneUtils.h"
#include "Macros.h"

ListDataCallLog::ListDataCallLog(CSettings * pSettings) {
    this->_settings = pSettings;
    this->Populate();
}

void ListDataCallLog::Release(void) {}

HRESULT ListDataCallLog::Populate(void) {
    HRESULT hr = S_OK;
    HANDLE ph = 0;
    DWORD lastEntryIndex = 0;
    DWORD currentEntryIndex = 0;
    TCHAR pszPrimary[PRIMARY_TEXT_LENGTH];
 	TCHAR pszSecondary[SECONDARY_TEXT_LENGTH];
    TCHAR pastTime;
    
    // these are for detecting repeating items
    TCHAR pszHash1[PRIMARY_TEXT_LENGTH*2] = {0};
    TCHAR pszHash2[PRIMARY_TEXT_LENGTH*2] = {0};
    int repeats = 0;

    COLORREF rgbPrimary = RGB(255, 255, 255);

    hr = PhoneOpenCallLog(&ph);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_END, 0, &lastEntryIndex);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, 0, 
        &currentEntryIndex);
    CHR(hr);

    this->_arrayLength = lastEntryIndex - currentEntryIndex;
    this->_items = new Data[this->_arrayLength];

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    for (int i = (int)currentEntryIndex; i <= (int)lastEntryIndex; i++) {
        if (this->_listCounter >= this->_arrayLength)
            break;

        pEntry->pszNumber = NULL;
        pEntry->pszName = NULL;
        pEntry->pszNameType = NULL;
        pEntry->pszNote = NULL;

        hr = PhoneGetCallLogEntry(ph, pEntry);
        CHR(hr);

		pastTime = _getPastTime(pEntry->ftStartTime, pszSecondary, 
            SECONDARY_TEXT_LENGTH);

        StringCchCopy(pszPrimary, PRIMARY_TEXT_LENGTH,  
            (pEntry->pszName ? pEntry->pszName
                : pEntry->pszNumber ? pEntry->pszNumber
                : this->_settings->unknown_string)
             );

        // calculate the hash for this entry, to check for duplicates
        // type (TCHAR)IOM and pastTime will be something like 0, 1, 2, 
        // so instead, make it 'A', 'B', 'C'
        pszHash2[0] = pastTime + 'A';
        pszHash2[1] = (TCHAR)pEntry->iom + 'A';
        pszHash2[2] = 0;
        if (pEntry->pszName)
            StringCchCat(pszHash2, PRIMARY_TEXT_LENGTH*2, pEntry->pszName);
        if (pEntry->pszNumber)
            StringCchCat(pszHash2, PRIMARY_TEXT_LENGTH*2, pEntry->pszNumber);

        // this is a repeated item
        if (_tcscmp(pszHash2, pszHash1) == 0) {
            repeats++;
        }

        // this is not a repeated item
        else {
            // the previous item was repeated
            if (repeats > 0) {
                Data * first = &this->_items[this->_listCounter - 1]; 
                StringCchPrintf(
                    &first->szPrimaryText[first->nPrimaryTextLength], 
                    PRIMARY_TEXT_LENGTH, TEXT(" [%d]"), repeats+1);
                first->nPrimaryTextLength = _tcslen(first->szPrimaryText);
                repeats = 0;
            }

            rgbPrimary = pEntry->iom == IOM_MISSED 
                ? this->_settings->rgbListItemMissedText 
                : this->_settings->rgbListItemText;

#ifdef DEBUG_SCREENSHOTS
            rgbPrimary = pEntry->iom == IOM_MISSED || i == 3
                ? this->_settings->rgbListItemMissedText 
                : this->_settings->rgbListItemText;
#endif
		    this->_addListItem(i, pszPrimary, pszSecondary, 
                pastTime, i, rgbPrimary);
        }

        StringCchCopy(pszHash1, PRIMARY_TEXT_LENGTH*2, pszHash2);

		// cleanup
		if (pEntry->pszNumber) delete pEntry->pszNumber;
		if (pEntry->pszName) delete pEntry->pszName;
		if (pEntry->pszNameType) delete pEntry->pszNameType;
		if (pEntry->pszNote) delete pEntry->pszNote;    
    }

Error:
    // more cleanup
    if (NULL != pEntry) {
        if (NULL != pEntry->pszNumber)
            delete pEntry->pszNumber;
        if (NULL != pEntry->pszName)
            delete pEntry->pszName;
        if (NULL != pEntry->pszNameType)
            delete pEntry->pszNameType;
        if (NULL != pEntry->pszNote)
            delete pEntry->pszNote;
        delete pEntry;
    }

    hr = PhoneCloseCallLog(ph);

    return hr;
}

HRESULT ListDataCallLog::PopulateDetailsFor(int index) {
    HRESULT hr = S_OK;
    HANDLE ph = 0;
    DWORD currentEntryIndex = 0;
    int id = this->_items[index].ID;

    // Save the original call
    IOM iom;
    TCHAR pszName[PRIMARY_TEXT_LENGTH] = {0};
    TCHAR pszNumber[PRIMARY_TEXT_LENGTH] = {0};
    TCHAR pastTime = 0;

    hr = PhoneOpenCallLog(&ph);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, id, 
        &currentEntryIndex);
    CHR(hr);

    this->_itemDetailCount = 0;
    this->_currentItemIndex = index;

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    hr = PhoneGetCallLogEntry(ph, pEntry);
    CHR(hr);

    // save the information for the first call
    iom = pEntry->iom;
    pastTime = _getPastTime(pEntry->ftStartTime);
    if (pEntry->pszName)
        StringCchCopy(pszName, PRIMARY_TEXT_LENGTH, pEntry->pszName);
    if (pEntry->pszNumber)
        StringCchCopy(pszNumber, PRIMARY_TEXT_LENGTH, pEntry->pszNumber);

    // Set the title of the details screen
    this->_currentDetailTitle =
          iom == IOM_MISSED   ? this->_settings->missed_string
        : iom == IOM_INCOMING ? this->_settings->incoming_string
        :                       this->_settings->outgoing_string;

    if (pEntry->pszName)
        this->_addDetail(diText, pszName);

    if (pEntry->pszNumber)
        this->_addDetail(diText, pszNumber);

    if (this->_itemDetailCount == 0)
        this->_addDetail(diText, this->_settings->unknown_string);

    do {
        if (pEntry->ftStartTime.dwHighDateTime > 0) {
            int c = this->_itemDetailCount;

            this->_detailItems[c].type = diText;
            StringCchCopy(this->_detailItems[c].label, SECONDARY_TEXT_LENGTH,
                this->_settings->date_string);
            _printDate(this->_detailItems[c].text, PRIMARY_TEXT_LENGTH, 
                pEntry->ftStartTime);
            
            this->_itemDetailCount++;
        }

        if (pEntry->iom != IOM_MISSED) {
            int c = this->_itemDetailCount;
            
            this->_detailItems[c].type = diText;
            StringCchCopy(this->_detailItems[c].label, SECONDARY_TEXT_LENGTH,
                this->_settings->duration_string);
            _printDuration(this->_detailItems[c].text, PRIMARY_TEXT_LENGTH, 
                pEntry->ftStartTime, pEntry->ftEndTime, this->_settings);

            this->_itemDetailCount++;
        }

	    if (pEntry->pszNumber) delete pEntry->pszNumber;
	    if (pEntry->pszName) delete pEntry->pszName;
	    if (pEntry->pszNameType) delete pEntry->pszNameType;
	    if (pEntry->pszNote) delete pEntry->pszNote;

        hr = PhoneGetCallLogEntry(ph, pEntry);
        CHR(hr);
    
    // this determines if the next entry is a duplicate entry
    // TODO: make this not ugly
    } while (NULL != pEntry 
        && iom == pEntry->iom
        && (pEntry->pszNumber == NULL && pszNumber[0] == 0 || pEntry->pszNumber != NULL && _tcscmp(pEntry->pszNumber, pszNumber) == 0)
        && (pEntry->pszName == NULL && pszName[0] == 0 || pEntry->pszName != NULL && _tcscmp(pEntry->pszName, pszName) == 0)
        && pastTime == _getPastTime(pEntry->ftStartTime));


    if (_tcslen(pszNumber) > 0) {
        // Button to return call
        this->_addDetail(diCallButton, this->_settings->returncall_string, 
            NULL, pszNumber, pszName);

        // Button to send SMS
        this->_addDetail(diSmsButton, this->_settings->sms_string, 
            NULL, pszNumber, pszName);

        // Button to add Contact
        if (_tcslen(pszName) == 0 || _tcscmp(pszName, pszNumber) == 0) {
            this->_addDetail(diSaveContactButton, 
                this->_settings->savecontact_string, NULL, pszNumber);
        }
    }

Error:
    // more cleanup
    if (NULL != pEntry) {
        if (NULL != pEntry->pszNumber)
            delete pEntry->pszNumber;
        if (NULL != pEntry->pszName)
            delete pEntry->pszName;
        if (NULL != pEntry->pszNameType)
            delete pEntry->pszNameType;
        if (NULL != pEntry->pszNote)
            delete pEntry->pszNote;
        delete pEntry;
        pEntry = NULL;
    }

    hr = PhoneCloseCallLog(ph);

    return hr;
}

void ListDataCallLog::GetItemGroup(int index, TCHAR * pszGroup) {
    TCHAR g = this->_items[index].wcGroup;
	StringCchCopy(pszGroup, 10, 
		  g == 1 ? this->_settings->sunday_string
		: g == 2 ? this->_settings->monday_string
		: g == 3 ? this->_settings->tuesday_string
		: g == 4 ? this->_settings->wednesday_string
		: g == 5 ? this->_settings->thursday_string
		: g == 6 ? this->_settings->friday_string
		: g == 7 ? this->_settings->saturday_string
        : g == 'T' ? this->_settings->today_string
        : g == 'Y' ? this->_settings->yesterday_string
        : this->_settings->older_string
	);
}

void ListDataCallLog::ToggleFavorite() {}

HRESULT ListDataCallLog::DisplayItem() { return S_OK; }

void ListDataCallLog::EditItem() {}

TCHAR _getPastTime(FILETIME ftTime, TCHAR * pszSecondary, int strLength) {

    // if ftTime somehow == 0, then get outta here
    if (!ftTime.dwHighDateTime && !ftTime.dwLowDateTime) {
        if (pszSecondary)
            pszSecondary[0] = 0;
        return 0;
    }

    FILETIME ftToday;		// midnight today (but UTC)
	FILETIME ftYesterday;	// midnight yesterday (but UTC)
	FILETIME ftThisWeek;	// midnight one week ago (but UTC)

	// temporary vars
	SYSTEMTIME st;
	FILETIME ft;

	// calculate ftToday
	GetLocalTime(&st);
	st.wMinute = 0;
	st.wSecond = 0;
	st.wHour = 0;
	SystemTimeToFileTime(&st, &ft);
	LocalFileTimeToFileTime(&ft, &ftToday);
	if (CompareFileTime(&ftTime, &ftToday) >= 0) {
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
        if (pszSecondary)
            ::GetTimeFormat(LOCALE_USER_DEFAULT, 
                TIME_NOSECONDS, &st,
                NULL, pszSecondary, strLength);
		return 'T';
	}

	// calculate ftYesterday
	ULARGE_INTEGER uli;
	uli.HighPart = ftToday.dwHighDateTime;
	uli.LowPart = ftToday.dwLowDateTime;
	uli.QuadPart -= 864000000000;  // 100-nanosecond blocks in one day
	ftYesterday.dwHighDateTime = uli.HighPart;
	ftYesterday.dwLowDateTime = uli.LowPart;
	if (CompareFileTime(&ftTime, &ftYesterday) >= 0) {
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
        if (pszSecondary)
            ::GetTimeFormat(LOCALE_USER_DEFAULT, 
                TIME_NOSECONDS, 
                &st, NULL, pszSecondary, strLength);
		return 'Y';
	}

	// calculate ftThisWeek
	uli.QuadPart -= 5184000000000;
	ftThisWeek.dwHighDateTime = uli.HighPart;
	ftThisWeek.dwLowDateTime = uli.LowPart;
	if (CompareFileTime(&ftTime, &ftThisWeek) >= 0) {
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
        if (pszSecondary)
            ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, 
                &st, NULL, pszSecondary, strLength);
        return st.wDayOfWeek + 1;
	}

	// Older than one week ago
	FileTimeToLocalFileTime(&ftTime, &ft);
	FileTimeToSystemTime(&ft, &st);
    if (pszSecondary)
        ::GetDateFormat(LOCALE_USER_DEFAULT, NULL, &st,
            TEXT("M/dd"), pszSecondary, strLength);
	return 'O';
}

// prints the date and time, like: 2008/04/02 23:59
// in the user's current locale
void _printDate(TCHAR * str, int strLength, FILETIME ft) {
    FILETIME ftLocal;
    SYSTEMTIME st;
    TCHAR buffer[10];

    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    ::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st,
        NULL, str, strLength);
    ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, 
        &st, NULL, buffer, 10);
    StringCchCat(str, strLength, TEXT(" "));
    StringCchCat(str, strLength, buffer);
}

void _printDuration(TCHAR * str, int strLength, FILETIME ftStart, 
    FILETIME ftEnd, CSettings * pSettings) {
    LARGE_INTEGER liStart;
    LARGE_INTEGER liEnd;
    LONGLONG duration;
    int hours;
    int minutes;

    liStart.HighPart = ftStart.dwHighDateTime;
    liStart.LowPart = ftStart.dwLowDateTime;
    liEnd.HighPart = ftEnd.dwHighDateTime;
    liEnd.LowPart = ftEnd.dwLowDateTime;
    duration = liEnd.QuadPart - liStart.QuadPart;
    duration /= 10000000; //100 nanosecond intervals -> seconds

    if (duration > 3600) {
        hours = (int)duration / 3600;
        duration -= hours * 3600;
        minutes = (int)duration / 60;
        duration -= minutes * 60;
        StringCchPrintf(str, strLength, TEXT("%d:%02d:%02d"), 
            hours, minutes, duration);
    }
    else if (duration > 60) {
        minutes = (int)duration / 60;
        duration -= minutes * 60;
        StringCchPrintf(str, strLength, TEXT("%d:%02d"), minutes, duration);
    }
    else {
        StringCchPrintf(str, strLength, TEXT("%d "), duration);
        StringCchCat(str, strLength, pSettings->seconds_string);
    }
}
