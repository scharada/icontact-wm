#include "stdafx.h"
#include "ListDataCallLog.h"
#include "windows.h"
#include "PhoneUtils.h"

TCHAR _getPastTime(FILETIME ftTime, TCHAR * pszSecondary, int strLength);
void _printDate(TCHAR * str, int strLength, FILETIME ft);
void _printDuration(TCHAR * str, int strLength, FILETIME ftStart, FILETIME ftEnd, Settings * pSettings);

ListDataCallLog::ListDataCallLog(Settings * pSettings) {
    this->_settings = pSettings;
    this->Populate();
}

void ListDataCallLog::Clear(void) {}

void ListDataCallLog::Populate(void) {
    HANDLE ph;
    DWORD lastEntryIndex;
    DWORD currentEntryIndex;
 	TCHAR pszSecondary[12];
    TCHAR pastTime;

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

		pastTime = _getPastTime(pEntry->ftStartTime, pszSecondary, 
            SECONDARY_TEXT_LENGTH);

		AddItem(i,
            pEntry->pszName ? pEntry->pszName
                : pEntry->pszNumber ? pEntry->pszNumber
                : this->_settings->unknown_string,
            pszSecondary,
            pastTime,
            i);

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
            StringCchCopy(this->_actionsList[c].label, SECONDARY_TEXT_LENGTH,
                this->_settings->type_string);
            StringCchCopy(this->_actionsList[c].text, PRIMARY_TEXT_LENGTH,
                pEntry->iom == IOM_MISSED 
                    ? this->_settings->missed_string
                    : pEntry->iom == IOM_INCOMING 
                    ? this->_settings->incoming_string
                    : this->_settings->outgoing_string
                );
            c++;

            if (pEntry->pszName) {
                this->_actionsList[c].id = c + 1;
                this->_actionsList[c].action = SLA_CALL;
                StringCchCopy(this->_actionsList[c].label, SECONDARY_TEXT_LENGTH,
                    this->_settings->number_string);
                StringCchCopy(this->_actionsList[c].text, PRIMARY_TEXT_LENGTH, 
                    pEntry->pszNumber);
                c++;
            }

            this->_actionsList[c].id = c + 1;
            this->_actionsList[c].action = SLA_TEXT;
            StringCchCopy(this->_actionsList[c].label, SECONDARY_TEXT_LENGTH,
                this->_settings->date_string);
            _printDate(this->_actionsList[c].text, PRIMARY_TEXT_LENGTH, 
                pEntry->ftStartTime);
            c++;

            if (pEntry->iom != IOM_MISSED) {
                this->_actionsList[c].id = c + 1;
                this->_actionsList[c].action = SLA_TEXT;
                StringCchCopy(this->_actionsList[c].label, SECONDARY_TEXT_LENGTH,
                    this->_settings->duration_string);
                _printDuration(this->_actionsList[c].text, PRIMARY_TEXT_LENGTH, 
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

void ListDataCallLog::ToggleFavorite(int index) {}

void ListDataCallLog::DisplayItem(int index) {
    // TODO: is this needed?
    this->PopulateDetailsFor(index);

    for (int i = 0; i < MAX_SUBLIST_ITEMS; i++) {
        if (this->_actionsList[i].action == -1)
            return;

        if (0 == wcscmp(this->_actionsList[i].label, 
            this->_settings->number_string)) {

            Call(this->_actionsList[i].text, 
                this->_items[index].szPrimaryText);
        }
    }
}

TCHAR _getPastTime(FILETIME ftTime, TCHAR * pszSecondary, int strLength) {

    // if ftTime somehow == 0, then get outta here
    if (!ftTime.dwHighDateTime && !ftTime.dwLowDateTime) {
        StringCchPrintf(pszSecondary, strLength, TEXT(""));
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
		StringCchPrintf(pszSecondary, strLength, TEXT("%02u:%02u"), 
            st.wHour, st.wMinute);
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
		StringCchPrintf(pszSecondary, strLength, TEXT("%02u:%02u"), 
            st.wHour, st.wMinute);
		return 'Y';
	}

	// calculate ftThisWeek
	uli.QuadPart -= 5184000000000;
	ftThisWeek.dwHighDateTime = uli.HighPart;
	ftThisWeek.dwLowDateTime = uli.LowPart;
	if (CompareFileTime(&ftTime, &ftThisWeek) >= 0) {
		FileTimeToLocalFileTime(&ftTime, &ft);
		FileTimeToSystemTime(&ft, &st);
		StringCchPrintf(pszSecondary, strLength, TEXT("%02u:%02u"), 
            st.wHour, st.wMinute);
		return st.wDayOfWeek + 1;
	}

	// Older than one week ago
	FileTimeToLocalFileTime(&ftTime, &ft);
	FileTimeToSystemTime(&ft, &st);
	StringCchPrintf(pszSecondary, strLength, TEXT("%u/%02u"),
        st.wMonth, st.wDay);
	return 'O';
}

void _printDate(TCHAR * str, int strLength, FILETIME ft) {
    FILETIME ftLocal;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    StringCchPrintf(str, strLength, TEXT("%d/%02d/%02d %d:%02d"), 
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
}

void _printDuration(TCHAR * str, int strLength, FILETIME ftStart, FILETIME ftEnd, 
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