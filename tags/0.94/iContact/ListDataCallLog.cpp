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

#define EDB
#include <windows.h>
#include <windbase.h>

#include "ListDataCallLog.h"
#include "PhoneUtils.h"
#include "Macros.h"

// dynamically loaded functions
typedef HRESULT (*PHONEOPENCALLLOG)(HANDLE *ph);
typedef HRESULT (*PHONEGETCALLLOGENTRY)(HANDLE h, PCALLLOGENTRY pentry);
typedef HRESULT (*PHONESEEKCALLLOG)(HANDLE hdb, CALLLOGSEEK seek, DWORD iRecord, LPDWORD piRecord);
typedef HRESULT (*PHONECLOSECALLLOG)(HANDLE h);

#include <phone.h>


////////////////////////////////{


#define MAKEPROP(n,t)    ((n<<16)|CEVT_##t)
#define PROPID_CALLER_ID	MAKEPROP(0x06, LPWSTR)
#define PROPID_CALLER_NAME  MAKEPROP(0x07, LPWSTR)
#define PROPID_CALLER_TYPE  MAKEPROP(0x0a, LPWSTR)
#define PROPID_START_TIME	MAKEPROP(0x02, FILETIME)
#define PROPID_END_TIME		MAKEPROP(0x03, FILETIME)
#define PROPID_IOM			MAKEPROP(0x04, I4)

#define DB_VOL_FN		 TEXT("pim.vol")
#define CALLLOG_DB_NAME  TEXT("clog.db")
////////////////////////////////}


// benchmark the long call log loading function
//#define BENCHMARK_LOAD_TIME

UINT _getDaySerial(const SYSTEMTIME * st);
void _fillSystemTimeFromSerial(SYSTEMTIME * st, UINT userial);

UINT _getPastTime(FILETIME ftTime, TCHAR * pszSecondary = NULL, int strLength = 0);
void _printDate(TCHAR * str, int strLength, FILETIME ft);
void _printDuration(TCHAR * str, int strLength, FILETIME ftStart,
                    FILETIME ftEnd, CSettings * pSettings);

// *************************************************
// These functions are for the main Recents list
// *************************************************
CEGUID guid = {0};

HRESULT RecentsPopulate(DataItem * parent, void (*adder)(DataItem*),
                        CSettings * pSettings) {
    HRESULT hr = S_OK;

    CEOID oid = 0;
    HANDLE hdb = 0;
    CEPROPVAL * pRecord = NULL;

    DataItem dataPrev = {0};
    DataItem data = {0};

    data.isFavorite = false;
    data.type = diListItem;

#ifdef BENCHMARK_LOAD_TIME
    DWORD tTime1 = ::GetTickCount();
#endif
    
    // these are for detecting repeating items
    TCHAR pszHash1[PRIMARY_TEXT_LENGTH*2] = {0};
    TCHAR pszHash2[PRIMARY_TEXT_LENGTH*2] = {0};
    int repeats = 0;

    COLORREF rgbPrimary = RGB(255, 255, 255);

	if (!guid.Data1) {
		CeMountDBVolEx(&guid, DB_VOL_FN, NULL, OPEN_EXISTING);
	}

	SORTORDERSPECEX sort = {0};
	sort.wVersion = SORTORDERSPECEX_VERSION;
	sort.wNumProps = 1;
	sort.wKeyFlags = 0;
	sort.rgPropID[0] = PROPID_START_TIME;
	sort.rgdwFlags[0] = CEDB_SORT_DESCENDING;

    hdb = CeOpenDatabaseInSession(NULL, &guid, &oid, CALLLOG_DB_NAME,
		&sort, CEDB_AUTOINCREMENT, NULL);

    if (hdb == INVALID_HANDLE_VALUE) {
        //DWORD error = GetLastError();
        goto Error;
    }
    
	// This is not needed with CEDB_AUTOINCREMENT flag set
	//oid = CeSeekDatabaseEx(hdb, CEDB_SEEK_BEGINNING, 0, 0, 0);

#ifdef BENCHMARK_LOAD_TIME
    DWORD tTime2 = ::GetTickCount();
#endif

	DWORD  dwBufSize = 0;
    WORD   wNumProps;

	TCHAR * pszName;
	TCHAR * pszNumber;
	TCHAR * pszType;
	FILETIME ftStart;
	WORD iom;

	WORD i;
    for (i = 0; i < 300; i++) {
        oid = CeReadRecordPropsEx(hdb, CEDB_ALLOWREALLOC, &wNumProps, NULL,
			(LPBYTE *)&pRecord, &dwBufSize, NULL);
		if (!oid)
			break;

		pszName = NULL;
		pszNumber = NULL;
		pszType = NULL;
		iom = 0;

		for (WORD iProp = 0; iProp < wNumProps; iProp++) {
			switch(pRecord[iProp].propid) {
				case PROPID_CALLER_ID:
					pszNumber = pRecord[iProp].val.lpwstr;
					break;

				case PROPID_CALLER_NAME:
					pszName = pRecord[iProp].val.lpwstr;
					break;

				case PROPID_CALLER_TYPE:
					pszType = pRecord[iProp].val.lpwstr;
					break;

				case PROPID_START_TIME:
					ftStart = pRecord[iProp].val.filetime;

					// we'll store the oid as the low order word of the call time,
					// this, along with the approximate index, i, should lead us
					// directly to the call (even if another call has been placed since)
					data.oId = ftStart.dwLowDateTime;

					data.iGroup = _getPastTime(ftStart, data.szSecondaryText, 
						SECONDARY_TEXT_LENGTH);
					break;

				case PROPID_IOM:
					iom = pRecord[iProp].val.iVal;
			        data.isMissed = (iom & 3) == 0;
					break;
			}
		}

        // populate the list Data structure
        data.ID = i;

        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH,  
            (pszName ? pszName
                : pszNumber ? pszNumber
                : pSettings->unknown_string)
             );

#ifdef DEBUG_SCREENSHOTS
         data.isMissed = pEntry->iom == IOM_MISSED || i == 3;
#endif

        // calculate the hash for this entry, to check for duplicates
        // type (TCHAR)IOM and pastTime will be something like 0, 1, 2, 
        // so instead, make it '0', '1', '2'
        pszHash2[0] = data.iGroup + 'A';
        pszHash2[1] = (TCHAR)iom + 'A';
        pszHash2[2] = 0;
        if (pszName)
            StringCchCat(pszHash2, PRIMARY_TEXT_LENGTH*2, pszName);
        if (pszNumber)
            StringCchCat(pszHash2, PRIMARY_TEXT_LENGTH*2, pszNumber);

        // this is a repeated item
        if (0 == _tcscmp(pszHash2, pszHash1)) {
            repeats++;
        }

        // this is not a repeated item
        else {
            // the previous item was repeated
            if (repeats > 0) {
                int len = _tcslen(dataPrev.szPrimaryText);
                StringCchPrintf(
                    &dataPrev.szPrimaryText[len], PRIMARY_TEXT_LENGTH - len,
                    TEXT(" [%d]"), repeats + 1);

                repeats = 0;
            }

            // actually write the data to the cache file
            if (i > 0)
                adder(&dataPrev); 

            // Shift data down into dataPrev
            memcpy(&dataPrev, &data, sizeof(DataItem));
        }

        StringCchCopy(pszHash1, PRIMARY_TEXT_LENGTH*2, pszHash2); 
    }

    // add the last list item
	if (oid)
		adder(&data); 

Error:
    // more cleanup
	if (pRecord)
		LocalFree(pRecord);
	if (hdb != 0 && hdb == INVALID_HANDLE_VALUE)
	    CloseHandle(hdb);

	//CeUnmountDBVol(&guid);
	//guid.Data1 = NULL;

#ifdef BENCHMARK_LOAD_TIME
    DWORD tTime3 = ::GetTickCount();
    TCHAR tszTimes[1024];
	StringCchPrintf(tszTimes, 1024, TEXT("open:%d, iterate:%d, per entry:%d"),
		(tTime2-tTime1), (tTime3-tTime2), (tTime3-tTime1)/i);
    MessageBox(NULL, tszTimes, TEXT("Benchmark"), 0);
#endif

    return hr;
}


HRESULT RecentsPopulateOLD(DataItem * parent, void (*adder)(DataItem*),
                        CSettings * pSettings) {
    HRESULT hr = S_OK;
    HANDLE ph = 0;
    DWORD lastEntryIndex = 0;
    DWORD currentEntryIndex = 0;
    CALLLOGENTRY *pEntry = NULL;

    DataItem dataPrev = {0};
    DataItem data = {0};

    data.isFavorite = false;
    data.type = diListItem;

#ifdef BENCHMARK_LOAD_TIME
    DWORD tTime1 = ::GetTickCount();
#endif
    
    // these are for detecting repeating items
    TCHAR pszHash1[PRIMARY_TEXT_LENGTH*2] = {0};
    TCHAR pszHash2[PRIMARY_TEXT_LENGTH*2] = {0};
    int repeats = 0;

    COLORREF rgbPrimary = RGB(255, 255, 255);

	// dynamically load the call log access functions
	HINSTANCE hiPhoneDll = LoadLibrary(TEXT("phone.dll"));
	if (!hiPhoneDll) {
		// TODO: alert user?
		hr = E_FAIL;
		goto Error;
	}
	PHONEOPENCALLLOG PhoneOpenCallLog = (PHONEOPENCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneOpenCallLog"));
	PHONEGETCALLLOGENTRY PhoneGetCallLogEntry = (PHONEGETCALLLOGENTRY)GetProcAddress(
		hiPhoneDll, TEXT("PhoneGetCallLogEntry"));
	PHONESEEKCALLLOG PhoneSeekCallLog = (PHONESEEKCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneSeekCallLog"));
	PHONECLOSECALLLOG PhoneCloseCallLog = (PHONECLOSECALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneCloseCallLog"));


    hr = PhoneOpenCallLog(&ph);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_END, 0, &lastEntryIndex);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, 0, &currentEntryIndex);
    CHR(hr);

#ifdef BENCHMARK_LOAD_TIME
    DWORD tTime2 = ::GetTickCount();
#endif

	pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    for (int i = (int)currentEntryIndex; i <= (int)lastEntryIndex; i++) {
        pEntry->pszNumber = NULL;
        pEntry->pszName = NULL;
        pEntry->pszNameType = NULL;
        pEntry->pszNote = NULL;

        hr = PhoneGetCallLogEntry(ph, pEntry);
        CHR(hr);

        // populate the list Data structure
        data.ID = i;
        data.isMissed = pEntry->iom == IOM_MISSED;

        // we'll store the oid as the low order word of the call time,
        // this, along with the approximate index, i, should lead us
        // directly to the call (even if another call has been placed since)
        data.oId = pEntry->ftStartTime.dwLowDateTime;

        data.iGroup = _getPastTime(pEntry->ftStartTime, data.szSecondaryText, 
            SECONDARY_TEXT_LENGTH);

        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH,  
            (pEntry->pszName ? pEntry->pszName
                : pEntry->pszNumber ? pEntry->pszNumber
                : pSettings->unknown_string)
             );

#ifdef DEBUG_SCREENSHOTS
         data.isMissed = pEntry->iom == IOM_MISSED || i == 3;
#endif

        // calculate the hash for this entry, to check for duplicates
        // type (TCHAR)IOM and pastTime will be something like 0, 1, 2, 
        // so instead, make it 'A', 'B', 'C'
        pszHash2[0] = data.iGroup + 'A';
        pszHash2[1] = (TCHAR)pEntry->iom + 'A';
        pszHash2[2] = 0;
        if (pEntry->pszName)
            StringCchCat(pszHash2, PRIMARY_TEXT_LENGTH*2, pEntry->pszName);
        if (pEntry->pszNumber)
            StringCchCat(pszHash2, PRIMARY_TEXT_LENGTH*2, pEntry->pszNumber);

        // this is a repeated item
        if (0 == _tcscmp(pszHash2, pszHash1)) {
            repeats++;
        }

        // this is not a repeated item
        else {
            // the previous item was repeated
            if (repeats > 0) {
                int len = _tcslen(dataPrev.szPrimaryText);
                StringCchPrintf(
                    &dataPrev.szPrimaryText[len], PRIMARY_TEXT_LENGTH - len,
                    TEXT(" [%d]"), repeats + 1);

                repeats = 0;
            }

            // actually write the data to the cache file
            if (i > 0)
                adder(&dataPrev); 

            // Shift data down into dataPrev
            memcpy(&dataPrev, &data, sizeof(DataItem));
        }

        StringCchCopy(pszHash1, PRIMARY_TEXT_LENGTH*2, pszHash2);

		// cleanup
		if (pEntry->pszNumber) delete pEntry->pszNumber;
		if (pEntry->pszName) delete pEntry->pszName;
		if (pEntry->pszNameType) delete pEntry->pszNameType;
		if (pEntry->pszNote) delete pEntry->pszNote;    
    }

    // add the last list item
    adder(&data); 

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

#ifdef BENCHMARK_LOAD_TIME
    DWORD tTime3 = ::GetTickCount();
    TCHAR tszTimes[1024];
	StringCchPrintf(tszTimes, 1024, TEXT("open:%d, iterate:%d, per entry:%d"), (tTime2-tTime1), (tTime3-tTime2), (tTime3-tTime1)/lastEntryIndex);
    MessageBox(NULL, tszTimes, TEXT("Benchmark"), 0);
#endif

    return hr;
}

HRESULT RecentsGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                        CSettings * pSettings) {
    
    StringCchCopy(buffer, cchDest, pSettings->recents_string);
    return S_OK;
}

HRESULT RecentsGetGroup(DataItem * data, TCHAR * buffer, int cchDest,
                        CSettings * pSettings) {

    UINT g = data->iGroup;

    SYSTEMTIME stToday;
    GetLocalTime(&stToday);
    UINT uToday = _getDaySerial(&stToday);

    if (uToday - g == 0) {
    	StringCchCopy(buffer, cchDest, pSettings->today_string);
    }
    else if (uToday - g == 1) {
    	StringCchCopy(buffer, cchDest, pSettings->yesterday_string);
    }
    else if (uToday - g <= 7) {
        g = g % 7;
	    StringCchCopy(buffer, cchDest, 
		      g == 0 ? pSettings->sunday_string
		    : g == 1 ? pSettings->monday_string
		    : g == 2 ? pSettings->tuesday_string
		    : g == 3 ? pSettings->wednesday_string
		    : g == 4 ? pSettings->thursday_string
		    : g == 5 ? pSettings->friday_string
		    : pSettings->saturday_string
	    );
    }
    else {
        SYSTEMTIME st;
        _fillSystemTimeFromSerial(&st, g);
        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st,
            NULL, buffer, cchDest);
    }

    return S_OK;
}

HRESULT RecentsClick(DataItem * data, float x,
                     int * newScreen, CSettings * pSettings) {
    *newScreen = 5;
    return S_OK;
}

// *************************************************
// These functions are for a recent details screen
// *************************************************

HRESULT RecentDetailsGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                        CSettings * pSettings) {

    //TODO: move this code and the duplicated code from RecentDetailsPopulate
    // both into a utility function
    HRESULT hr = S_OK;
    HANDLE ph = 0;
    DWORD lastEntryIndex = 0;
    DWORD currentEntryIndex = 0;
    IOM iom;
	
	// dynamically load the call log access functions
	HINSTANCE hiPhoneDll = LoadLibrary(TEXT("phone.dll"));
	if (!hiPhoneDll) {
		// TODO: alert user?
		hr = E_FAIL;
		goto Error;
	}
	PHONEOPENCALLLOG PhoneOpenCallLog = (PHONEOPENCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneOpenCallLog"));
	PHONEGETCALLLOGENTRY PhoneGetCallLogEntry = (PHONEGETCALLLOGENTRY)GetProcAddress(
		hiPhoneDll, TEXT("PhoneGetCallLogEntry"));
	PHONESEEKCALLLOG PhoneSeekCallLog = (PHONESEEKCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneSeekCallLog"));
	PHONECLOSECALLLOG PhoneCloseCallLog = (PHONECLOSECALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneCloseCallLog"));

    // Find the proper call log entry the user clicked on
    hr = PhoneOpenCallLog(&ph);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_END, 0, &lastEntryIndex);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, parent->ID, 
        &currentEntryIndex);
    CHR(hr);

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    hr = PhoneGetCallLogEntry(ph, pEntry);
    CHR(hr);

    // make sure the call time is right by verifying the oid
    // (or the low-order word of the date of the call)
    while (currentEntryIndex++ < lastEntryIndex
        && pEntry->ftStartTime.dwLowDateTime != parent->oId) {
        // cleanup
		if (pEntry->pszNumber) delete pEntry->pszNumber;
		if (pEntry->pszName) delete pEntry->pszName;
		if (pEntry->pszNameType) delete pEntry->pszNameType;
		if (pEntry->pszNote) delete pEntry->pszNote;

        hr = PhoneGetCallLogEntry(ph, pEntry);
        CHR(hr);
    }

    CBR(pEntry->ftStartTime.dwLowDateTime == parent->oId);

    // save the information for the first call
    iom = pEntry->iom;

    // Set the title of the details screen
    StringCchCopy(buffer, cchDest, 
          iom == IOM_MISSED   ? pSettings->missed_string
        : iom == IOM_INCOMING ? pSettings->incoming_string
        :                       pSettings->outgoing_string);

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

HRESULT RecentDetailsPopulate(DataItem * parent, void (*adder)(DataItem*),
                              CSettings * pSettings) {
    HRESULT hr = S_OK;
    HANDLE ph = 0;
    DWORD lastEntryIndex = 0;
    DWORD currentEntryIndex = 0;

    // Save the original call
    IOM iom;
    TCHAR pszName[PRIMARY_TEXT_LENGTH] = {0};
    TCHAR pszNumber[PRIMARY_TEXT_LENGTH] = {0};
    UINT pastTime = 0;

    // Initialize the data item that we're going to write
    DataItem data = {0};
    data.ID = parent->ID;
    data.iGroup = 0;
    data.isFavorite = false;
    data.isMissed = false;
    data.oId = parent->oId;
    data.type = diText;
	
	// dynamically load the call log access functions
	HINSTANCE hiPhoneDll = LoadLibrary(TEXT("phone.dll"));
	if (!hiPhoneDll) {
		// TODO: alert user?
		hr = E_FAIL;
		goto Error;
	}
	PHONEOPENCALLLOG PhoneOpenCallLog = (PHONEOPENCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneOpenCallLog"));
	PHONEGETCALLLOGENTRY PhoneGetCallLogEntry = (PHONEGETCALLLOGENTRY)GetProcAddress(
		hiPhoneDll, TEXT("PhoneGetCallLogEntry"));
	PHONESEEKCALLLOG PhoneSeekCallLog = (PHONESEEKCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneSeekCallLog"));
	PHONECLOSECALLLOG PhoneCloseCallLog = (PHONECLOSECALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneCloseCallLog"));

    // Find the proper call log entry the user clicked on
    hr = PhoneOpenCallLog(&ph);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_END, 0, &lastEntryIndex);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, data.ID, 
        &currentEntryIndex);
    CHR(hr);

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    hr = PhoneGetCallLogEntry(ph, pEntry);
    CHR(hr);

    // make sure the call time is right by verifying the oid
    // (or the low-order word of the date of the call)
    while (currentEntryIndex++ < lastEntryIndex
        && pEntry->ftStartTime.dwLowDateTime != data.oId) {
        // cleanup
		if (pEntry->pszNumber) delete pEntry->pszNumber;
		if (pEntry->pszName) delete pEntry->pszName;
		if (pEntry->pszNameType) delete pEntry->pszNameType;
		if (pEntry->pszNote) delete pEntry->pszNote;

        hr = PhoneGetCallLogEntry(ph, pEntry);
        CHR(hr);
    }

    CBR(pEntry->ftStartTime.dwLowDateTime == data.oId);

    // save the information for the first call
    iom = pEntry->iom;
    pastTime = _getPastTime(pEntry->ftStartTime);
    if (pEntry->pszName)
        StringCchCopy(pszName, PRIMARY_TEXT_LENGTH, pEntry->pszName);
    if (pEntry->pszNumber)
        StringCchCopy(pszNumber, PRIMARY_TEXT_LENGTH, pEntry->pszNumber);

    // Set the title of the details screen
    /* this->_currentDetailTitle =
          iom == IOM_MISSED   ? pSettings->missed_string
        : iom == IOM_INCOMING ? pSettings->incoming_string
        :                       pSettings->outgoing_string;
    */

    bool known = false;

    if (pEntry->pszName) {
        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pEntry->pszName);
        adder(&data); 
        known = true;
    }

    if (pEntry->pszNumber) {
        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pEntry->pszNumber);
        adder(&data); 
        known = true;
    }

    if (!known) {
        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->unknown_string);
        adder(&data); 
    }

    do {
        if (pEntry->ftStartTime.dwHighDateTime > 0) {
            StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH,
                pSettings->date_string);
            _printDate(data.szPrimaryText, PRIMARY_TEXT_LENGTH, 
                pEntry->ftStartTime);
            adder(&data);
        }

        if (pEntry->iom != IOM_MISSED) {
            StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH,
                pSettings->duration_string);
            _printDuration(data.szPrimaryText, PRIMARY_TEXT_LENGTH, 
                pEntry->ftStartTime, pEntry->ftEndTime, pSettings);
            adder(&data);
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
        data.szSecondaryText[0] = 0;

        // Button to return call
        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->returncall_string);
        data.type = diCallButton;
        adder(&data);

        // Button to send SMS
        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->sms_string);
        data.type = diSmsButton;
        adder(&data);

        // Button to add Contact
        if (_tcslen(pszName) == 0 || _tcscmp(pszName, pszNumber) == 0) {
            StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->savecontact_string);
            data.type = diSaveContactButton;
            adder(&data);
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

HRESULT RecentDetailsClick(DataItem * data, float x, int * newScreen,
                           CSettings * pSettings) {

    HRESULT hr = S_OK;
    HANDLE ph = 0;
    DWORD lastEntryIndex = 0;
    DWORD currentEntryIndex = 0;

    // Save the original call
    TCHAR pszName[PRIMARY_TEXT_LENGTH] = {0};
    TCHAR pszNumber[PRIMARY_TEXT_LENGTH] = {0};
	
	// dynamically load the call log access functions
	HINSTANCE hiPhoneDll = LoadLibrary(TEXT("phone.dll"));
	if (!hiPhoneDll) {
		// TODO: alert user?
		hr = E_FAIL;
		goto Error;
	}
	PHONEOPENCALLLOG PhoneOpenCallLog = (PHONEOPENCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneOpenCallLog"));
	PHONEGETCALLLOGENTRY PhoneGetCallLogEntry = (PHONEGETCALLLOGENTRY)GetProcAddress(
		hiPhoneDll, TEXT("PhoneGetCallLogEntry"));
	PHONESEEKCALLLOG PhoneSeekCallLog = (PHONESEEKCALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneSeekCallLog"));
	PHONECLOSECALLLOG PhoneCloseCallLog = (PHONECLOSECALLLOG)GetProcAddress(
		hiPhoneDll, TEXT("PhoneCloseCallLog"));

    // Find the proper call log entry the user clicked on
    hr = PhoneOpenCallLog(&ph);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_END, 0, &lastEntryIndex);
    CHR(hr);

    hr = PhoneSeekCallLog(ph, CALLLOGSEEK_BEGINNING, data->ID, 
        &currentEntryIndex);
    CHR(hr);

	CALLLOGENTRY *pEntry = new CALLLOGENTRY();
    //cbSize MUST be set before passing the struct in to the function!
    // Refer to the doc.
    pEntry->cbSize = sizeof(CALLLOGENTRY);

    hr = PhoneGetCallLogEntry(ph, pEntry);
    CHR(hr);

    // make sure the call time is right by verifying the oid
    // (or the low-order word of the date of the call)
    while (currentEntryIndex++ < lastEntryIndex
        && pEntry->ftStartTime.dwLowDateTime != data->oId) {
        // cleanup
		if (pEntry->pszNumber) delete pEntry->pszNumber;
		if (pEntry->pszName) delete pEntry->pszName;
		if (pEntry->pszNameType) delete pEntry->pszNameType;
		if (pEntry->pszNote) delete pEntry->pszNote;

        hr = PhoneGetCallLogEntry(ph, pEntry);
        CHR(hr);
    }

    CBR(pEntry->ftStartTime.dwLowDateTime == data->oId);

    switch (data->type) {
        case diCallButton:
            Call(pEntry->pszNumber, pEntry->pszName);
            *newScreen = -3;
            break;

        case diSmsButton:
            SendSMS(pEntry->pszNumber, pEntry->pszName);
            *newScreen = -3;
            break;

        case diSaveContactButton:
            AddContactByNumber(pEntry->pszNumber);
            *newScreen = -1;
            break;
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

// *************************************************
// Utility functions
// *************************************************

// http://www.hermetic.ch/cal_stud/jdn.htm
UINT _getDaySerial(const SYSTEMTIME * st) {
    int y = st->wYear;
    int m = st->wMonth;
    int d = st->wDay;

    // we'll modify the official julian date to instead be
    // "days elapsed since Jan 1, 1995" (since that's a sunday)
    long jul = ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
          ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
          ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
          d - 32075 - 2449719;

    return jul;
}

// http://www.hermetic.ch/cal_stud/jdn.htm
void _fillSystemTimeFromSerial(SYSTEMTIME * st, UINT userial) {
    UINT jd = userial + 2449719;
    UINT l, n, i, j, d, m, y;

    l = jd + 68569;
    n = ( 4 * l ) / 146097;
    l = l - ( 146097 * n + 3 ) / 4;
    i = ( 4000 * ( l + 1 ) ) / 1461001;
    l = l - ( 1461 * i ) / 4 + 31;
    j = ( 80 * l ) / 2447;
    d = l - ( 2447 * j ) / 80;
    l = j / 11;
    m = j + 2 - ( 12 * l );
    y = 100 * ( n - 49 ) + i + l;

    st->wHour = 0;
    st->wMinute = 0;
    st->wSecond = 0;
    st->wDayOfWeek = 0;
    st->wYear = y;
    st->wMonth = m;
    st->wDay = d;
}

UINT _getPastTime(FILETIME ftTime, TCHAR * pszSecondary, int strLength) {
    FILETIME ftLocal;
    SYSTEMTIME st;

    // if ftTime somehow == 0, then get outta here
    if (!ftTime.dwHighDateTime && !ftTime.dwLowDateTime) {
        if (pszSecondary)
            pszSecondary[0] = 0;
        return 0;
    }
    
    FileTimeToLocalFileTime(&ftTime, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    UINT u = _getDaySerial(&st);

    if (pszSecondary)
        ::GetTimeFormat(LOCALE_USER_DEFAULT, 
            TIME_NOSECONDS, &st,
            NULL, pszSecondary, strLength);

	return u;
}

// prints the date and time, like: 2008/04/02 23:59
// in the user's current locale
void _printDate(TCHAR * str, int strLength, FILETIME ft) {
    FILETIME ftLocal;
    SYSTEMTIME st;

    FileTimeToLocalFileTime(&ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    ::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st,
        NULL, str, strLength);
    StringCchCat(str, strLength, TEXT(" "));
    int len = _tcslen(str);
    ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, 
        &st, NULL, &str[len], strLength - len);
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
