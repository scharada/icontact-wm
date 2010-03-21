#include "stdafx.h"
#include "ListDataCallLog.h"
#include "windows.h"
#include "PhoneUtils.h"

void _fillPastTime(FILETIME ftTime, wchar_t * pszPastTime, wchar_t * pszSecondary);
void _printDate(wchar_t * str, FILETIME ft);
void _printDuration(wchar_t * str, FILETIME ftStart, FILETIME ftEnd);

ListDataCallLog::ListDataCallLog(Settings * pSettings) {
    this->_settings = pSettings;
    this->Populate();
}

void ListDataCallLog::Clear(void) {}

void ListDataCallLog::Populate(void) {
    HANDLE ph;
    DWORD lastEntryIndex;
    DWORD currentEntryIndex;
    wchar_t * buffer;
    wchar_t pszPastTime[20];
	wchar_t pszSecondary[12];
    wchar_t * pszCategories = L"";

    if (FAILED(PhoneOpenCallLog(&ph)))
        return;

    if (FAILED(PhoneSeekCallLog(ph, CALLLOGSEEK_END, 0, &lastEntryIndex)))
        goto PopulateClose;

    if (FAILED(PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, 0, 
        &currentEntryIndex)))
        goto PopulateClose;

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    for (int i = (int)currentEntryIndex; i <= (int)lastEntryIndex; i++) {
        pEntry->pszNumber = 0;
        pEntry->pszName = 0;
        pEntry->pszNameType = 0;
        pEntry->pszNote = 0;

        if (FAILED(PhoneGetCallLogEntry(ph, pEntry)))
            goto PopulateCleanup;

		_fillPastTime(pEntry->ftStartTime, pszPastTime, pszSecondary);

        buffer = pEntry->pszName ? pEntry->pszName
            : pEntry->pszNumber ? pEntry->pszNumber
            : L"unknown";

		AddItem(i, buffer, pszSecondary, pszCategories, 
            pszPastTime, pEntry->pszNumber, i, false);

		// cleanup
		if (pEntry->pszNumber) delete pEntry->pszNumber;
		if (pEntry->pszName) delete pEntry->pszName;
		if (pEntry->pszNameType) delete pEntry->pszNameType;
		if (pEntry->pszNote) delete pEntry->pszNote;    
    }

PopulateCleanup:
    // memory mgmt
    delete pEntry;

PopulateClose:
    PhoneCloseCallLog(ph);

    return;
}

int ListDataCallLog::PopulateDetailsFor(int id) {
    int c = 0;
    HANDLE ph;
    DWORD currentEntryIndex;

    if (FAILED(PhoneOpenCallLog(&ph)))
        return 0;

    if (FAILED(PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, 0, 
        &currentEntryIndex)))
        return 0;

    this->InitDetailData();

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    for (int i = (int)currentEntryIndex; i <= id; i++) {
        if (FAILED(PhoneGetCallLogEntry(ph, pEntry)))
            return 0;

        if (i == id) {
            this->_actionsList[c].id = c + 1;
            this->_actionsList[c].action = SLA_TEXT;
            wcsncpy(this->_actionsList[c].label, L"Type", 25);
            wcsncpy(this->_actionsList[c].text, 
                (pEntry->iom == IOM_MISSED ? L"Missed"
                : pEntry->iom == IOM_INCOMING ? L"Incoming"
                : L"Outgoing"),
                25);
            c++;

            if (pEntry->pszName) {
                this->_actionsList[c].id = c + 1;
                this->_actionsList[c].action = SLA_CALL;
                wcsncpy(this->_actionsList[c].label, L"Number", 25);
                wcsncpy(this->_actionsList[c].text, pEntry->pszNumber, 25);
                c++;
            }

            this->_actionsList[c].id = c + 1;
            this->_actionsList[c].action = SLA_TEXT;
            wcsncpy(this->_actionsList[c].label, L"Date", 25);
            _printDate(this->_actionsList[c].text, pEntry->ftStartTime);
            c++;

            if (pEntry->iom != IOM_MISSED) {
                this->_actionsList[c].id = c + 1;
                this->_actionsList[c].action = SLA_TEXT;
                wcsncpy(this->_actionsList[c].label, L"Duration", 25);
                _printDuration(this->_actionsList[c].text, pEntry->ftStartTime, pEntry->ftEndTime);
                c++;
            }
        }

		// cleanup
		delete pEntry->pszNumber;
		delete pEntry->pszName;
		delete pEntry->pszNameType;
		delete pEntry->pszNote;    
    }
    
    this->_actionsNumber = c;

    // more cleanup
    delete pEntry;
    PhoneCloseCallLog(ph);

    return 1;
}

void ListDataCallLog::AddToFavorites() {
}

void ListDataCallLog::RemoveFromFavorites() {
}

void ListDataCallLog::DisplayItem(int index) {
    Call(this->_items[index].szExtra, this->_items[index].szPrimaryText);
}

void _fillPastTime(FILETIME ftTime, wchar_t * pszPastTime, wchar_t * pszSecondary) {

    // if ftTime somehow == 0, then get outta here
    if (!ftTime.dwHighDateTime && !ftTime.dwLowDateTime) {
        wsprintf(pszPastTime, L"Unknown");
        wsprintf(pszSecondary, L"");
        return;
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
		wsprintf(pszPastTime, L"Today");
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
		wsprintf(pszSecondary, L"%02u:%02u", st.wHour, st.wMinute);
		return;
	}

	// calculate ftYesterday
	ULARGE_INTEGER uli;
	uli.HighPart = ftToday.dwHighDateTime;
	uli.LowPart = ftToday.dwLowDateTime;
	uli.QuadPart -= 864000000000;  // 100-nanosecond blocks in one day
	ftYesterday.dwHighDateTime = uli.HighPart;
	ftYesterday.dwLowDateTime = uli.LowPart;
	if (CompareFileTime(&ftTime, &ftYesterday) >= 0) {
		wsprintf(pszPastTime, L"Yesterday");
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
		wsprintf(pszSecondary, L"%02u:%02u", st.wHour, st.wMinute);
		return;
	}

	// calculate ftThisWeek
	uli.QuadPart -= 5184000000000;
	ftThisWeek.dwHighDateTime = uli.HighPart;
	ftThisWeek.dwLowDateTime = uli.LowPart;
	if (CompareFileTime(&ftTime, &ftThisWeek) >= 0) {
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
		wsprintf(pszPastTime, 
			  st.wDayOfWeek == 0 ? L"Sunday"
			: st.wDayOfWeek == 1 ? L"Monday"
			: st.wDayOfWeek == 2 ? L"Tuesday"
			: st.wDayOfWeek == 3 ? L"Wednesday"
			: st.wDayOfWeek == 4 ? L"Thursday"
			: st.wDayOfWeek == 5 ? L"Friday"
			: L"Saturday"
		);
		wsprintf(pszSecondary, L"12:38");
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
		wsprintf(pszSecondary, L"%02u:%02u", st.wHour, st.wMinute);
		return;
	}

	// Older than one week ago
	wsprintf(pszPastTime, L"Older");
	FileTimeToLocalFileTime(&ftTime, &ft);
	FileTimeToSystemTime(&ft, &st);
	wsprintf(pszSecondary, L"%u/%02u", st.wMonth, st.wDay);
	return;
}

void _printDate(wchar_t * str, FILETIME ft) {
    FILETIME ftLocal;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    wsprintf(str, L"%d/%02d/%02d %d:%02d", 
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
}

void _printDuration(wchar_t * str, FILETIME ftStart, FILETIME ftEnd) {
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
        wsprintf(str, L"%d:%02d:%02d", hours, minutes, duration);
    }
    else if (duration > 60) {
        minutes = (int)duration / 60;
        duration -= minutes * 60;
        wsprintf(str, L"%d:%02d", minutes, duration);
    }
    else {
        wsprintf(str, L"%d seconds", duration);
    }
}