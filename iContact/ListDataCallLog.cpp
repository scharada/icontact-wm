#include "stdafx.h"
#include "ListDataCallLog.h"
#include "windows.h"
#include "PhoneUtils.h"

wchar_t _getPastTime(FILETIME ftTime, wchar_t * pszSecondary);
void _printDate(wchar_t * str, FILETIME ft);
void _printDuration(wchar_t * str, FILETIME ftStart, FILETIME ftEnd, Settings * pSettings);

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
	wchar_t pszSecondary[12];
    wchar_t pastTime;

    if (FAILED(PhoneOpenCallLog(&ph)))
        return;

    if (FAILED(PhoneSeekCallLog(ph, CALLLOGSEEK_END, 0, &lastEntryIndex)))
        goto PopulateClose;

    if (FAILED(PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, 0, 
        &currentEntryIndex)))
        goto PopulateClose;

    this->_items = new Data[lastEntryIndex - currentEntryIndex];
    this->_arrayLength = lastEntryIndex - currentEntryIndex;

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    for (int i = (int)currentEntryIndex; i <= (int)lastEntryIndex; i++) {
        if (this->_listCounter >= this->_arrayLength)
            break;

        pEntry->pszNumber = 0;
        pEntry->pszName = 0;
        pEntry->pszNameType = 0;
        pEntry->pszNote = 0;

        if (FAILED(PhoneGetCallLogEntry(ph, pEntry)))
            goto PopulateCleanup;

		pastTime = _getPastTime(pEntry->ftStartTime, pszSecondary);

        buffer = pEntry->pszName ? pEntry->pszName
            : pEntry->pszNumber ? pEntry->pszNumber
            : L"unknown";

		AddItem(i, buffer, pszSecondary, pastTime, pEntry->pszNumber, i);

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
            wcsncpy(this->_actionsList[c].label, 
                this->_settings->type_string, 25);
            wcsncpy(this->_actionsList[c].text, 
                (pEntry->iom == IOM_MISSED 
                ? this->_settings->missed_string
                : pEntry->iom == IOM_INCOMING 
                ? this->_settings->incoming_string
                : this->_settings->outgoing_string),
                25);
            c++;

            if (pEntry->pszName) {
                this->_actionsList[c].id = c + 1;
                this->_actionsList[c].action = SLA_CALL;
                wcsncpy(this->_actionsList[c].label, 
                    this->_settings->number_string, 25);
                wcsncpy(this->_actionsList[c].text, pEntry->pszNumber, 25);
                c++;
            }

            this->_actionsList[c].id = c + 1;
            this->_actionsList[c].action = SLA_TEXT;
            wcsncpy(this->_actionsList[c].label, 
                this->_settings->date_string, 25);
            _printDate(this->_actionsList[c].text, pEntry->ftStartTime);
            c++;

            if (pEntry->iom != IOM_MISSED) {
                this->_actionsList[c].id = c + 1;
                this->_actionsList[c].action = SLA_TEXT;
                wcsncpy(this->_actionsList[c].label, 
                    this->_settings->duration_string, 25);
                _printDuration(this->_actionsList[c].text, 
                    pEntry->ftStartTime, pEntry->ftEndTime, this->_settings);
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

void ListDataCallLog::GetItemGroup(int index, wchar_t * pszGroup) {
    wchar_t g = this->_items[index].wcGroup;
	wsprintf(pszGroup, 
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

void ListDataCallLog::ToggleFavorite(int index) {}

void ListDataCallLog::DisplayItem(int index) {
    Call(this->_items[index].szExtra, this->_items[index].szPrimaryText);
}

wchar_t _getPastTime(FILETIME ftTime, wchar_t * pszSecondary) {

    // if ftTime somehow == 0, then get outta here
    if (!ftTime.dwHighDateTime && !ftTime.dwLowDateTime) {
        wsprintf(pszSecondary, L"");
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
		wsprintf(pszSecondary, L"%02u:%02u", st.wHour, st.wMinute);
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
		wsprintf(pszSecondary, L"%02u:%02u", st.wHour, st.wMinute);
		return 'Y';
	}

	// calculate ftThisWeek
	uli.QuadPart -= 5184000000000;
	ftThisWeek.dwHighDateTime = uli.HighPart;
	ftThisWeek.dwLowDateTime = uli.LowPart;
	if (CompareFileTime(&ftTime, &ftThisWeek) >= 0) {
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
		wsprintf(pszSecondary, L"%02u:%02u", st.wHour, st.wMinute);
		return st.wDayOfWeek + 1;
	}

	// Older than one week ago
	FileTimeToLocalFileTime(&ftTime, &ft);
	FileTimeToSystemTime(&ft, &st);
	wsprintf(pszSecondary, L"%u/%02u", st.wMonth, st.wDay);
	return 'O';
}

void _printDate(wchar_t * str, FILETIME ft) {
    FILETIME ftLocal;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    wsprintf(str, L"%d/%02d/%02d %d:%02d", 
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
}

void _printDuration(wchar_t * str, FILETIME ftStart, FILETIME ftEnd, 
                    Settings * pSettings) {
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
        wsprintf(str, L"%d %s", duration, pSettings->seconds_string);
    }
}