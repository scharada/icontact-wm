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
#include "PhoneUtils.h"
#include "Macros.h"

#include <pimstore.h>

#include "windows.h"
#include "regext.h"
#include "tapi.h"
#include "snapi.h"

#include <cemapi.h>
#include <mapiutil.h>
#include <mapidefs.h>

#define MAX_COMMAND_LINE 200
#define MAX_LOADSTRING 100

// main functions

bool GetIDialerFilename(TCHAR * szFilename) {
    // generate path name for iContact
    ::GetModuleFileName(NULL, szFilename, MAX_PATH);
    TCHAR * pstr = _tcsrchr(szFilename, '\\');
    if (pstr) *(pstr) = '\0';
    pstr = _tcsrchr(szFilename, '\\');
    if (pstr) *(++pstr) = '\0';
    StringCchCat(szFilename, MAX_PATH, TEXT("iDialer\\iDialer.exe"));

    // test if file exists
    HANDLE h = CreateFile(szFilename, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, NULL, NULL);
    bool exists = h != INVALID_HANDLE_VALUE;
    if (exists) {
        CloseHandle(h);
    }
    return exists;
}

void Call(TCHAR * number, TCHAR * name) {
    PHONEMAKECALLINFO mci = {0};
    PROCESS_INFORMATION pi = {0};
    LRESULT lResult;
    TCHAR szFilename[MAX_PATH];

    bool iDialerExists = GetIDialerFilename(szFilename);
    if (iDialerExists) {
        CreateProcess(szFilename, number,
            NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
    }
    else {
        mci.cbSize = sizeof(mci);
        mci.dwFlags = 0;
        //mci.pszDestAddress = tszPhoneNumber;
        //mci.pszDestAddress = ActionsList[SubListCurrentAction].text;
        mci.pszDestAddress = number;
        mci.pszCalledParty = name;
        lResult = PhoneMakeCall(&mci);
    }
}

void SendSMS(TCHAR * number, TCHAR * name) {
    PROCESS_INFORMATION pi;
    TCHAR tszCommandLine[MAX_COMMAND_LINE];

    StringCchPrintf(tszCommandLine, MAX_COMMAND_LINE, 
        TEXT("-service \"SMS\" -to \"\\\"%s\\\" <%s>\""), name, number);

    CreateProcess(TEXT("tmail.exe"), tszCommandLine, 
        NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
}

void SendEMail(const TCHAR * account, TCHAR * to) {
    PROCESS_INFORMATION pi;
    TCHAR tszCommandLine[MAX_COMMAND_LINE];
    TCHAR tszAccountName[MAX_LOADSTRING];

    // If account == "", then get the last used email account name from the phone
    if (_tcslen(account) > 0) {
        StringCchPrintf(tszCommandLine, MAX_COMMAND_LINE, 
            TEXT("-service \"%s\" -to \"%s\""), account, to);
    }
    else {
        // This should be the name of the last used email account
        GetDefaultEmailAccount(tszAccountName);

        StringCchPrintf(tszCommandLine, MAX_COMMAND_LINE, 
            TEXT("-service \"%s\" -to \"%s\""), tszAccountName, to);
    }

    CreateProcess(TEXT("tmail.exe"), tszCommandLine, 
        NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
}

// http://blogs.msdn.com/windowsmobile/archive/2007/03/21/getting-started-with-mapi.aspx
void GetDefaultEmailAccount(TCHAR * tszAccountName) {
    StringCchCopy(tszAccountName, 32, TEXT("ActiveSync"));
    /*
    HRESULT hr;
    IMAPITable * ptbl;
    IMAPISession * pSession;
    SRowSet *prowset = NULL;
    SPropValue  *pval = NULL;
    SizedSPropTagArray (1, spta) = { 1, PR_DISPLAY_NAME };
    int index = 0;
   
    // Log onto MAPI
    hr = MAPILogonEx(0, NULL, NULL, 0, static_cast<LPMAPISESSION *>(&pSession));
    CHR(hr); // CHR will goto Error if FAILED(hr)
   
    // Get the table of accounts
    hr = pSession->GetMsgStoresTable(0, &ptbl);
    CHR(hr);
   
    // set the columns of the table we will query
    hr = ptbl->SetColumns ((SPropTagArray *) &spta, 0);
    CHR(hr);
   
    while (TRUE) {
        // Free the previous row
        FreeProws(prowset);
        prowset = NULL;
 
        hr = ptbl->QueryRows (1, 0, &prowset);
        if ((hr != S_OK) || (prowset == NULL) || (prowset->cRows == 0)) {
            break;
        }
 
        ASSERT(prowset->aRow[0].cValues == spta.cValues);
        pval = prowset->aRow[0].lpProps;
 
        ASSERT(pval[0].ulPropTag == PR_DISPLAY_NAME);
 
        if (0 != _tcscmp(pval[0].Value.lpszW, TEXT("SMS"))) {
            StringCchCopy(tszAccountName, MAX_LOADSTRING, pval[0].Value.lpszW);
            break;
        }
        //MessageBox(NULL, pval[0].Value.lpszW, TEXT("Message Store"), MB_OK);
    }
 
    pSession->Logoff(0, 0, 0);
 
Error:
    RELEASE_OBJ(ptbl);
    RELEASE_OBJ(pSession);
    FreeProws(prowset);
    */
}

void OpenURL(const TCHAR * url) {
    SHELLEXECUTEINFO ShExecInfo = {0};

    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = TEXT("open");
    ShExecInfo.lpFile = url;
    ShExecInfo.lpParameters = NULL;

    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL; 

    ShellExecuteEx(&ShExecInfo);
}

// http://groups.google.com/group/Google-Maps-for-mobile/browse_thread/thread/f41fb116f9faf2b/76aa7eb41a734984
/* TODO: this would be cool! Fix it!
void OpenGoogleMaps(CEOID ceoid) {
    PROCESS_INFORMATION pi;
    TCHAR tszArgs[MAX_COMMAND_LINE];

    TCHAR szGmapsPath[MAX_PATH];
    GetModuleFileName(NULL, szGmapsPath, MAX_PATH);
    TCHAR * pstr = _tcsrchr(szGmapsPath, '\\');
    if (pstr) *(++pstr) = '\0';
    StringCchCat(szGmapsPath, MAX_PATH, TEXT("..\\GoogleMaps\\GoogleMaps.exe"));

    StringCchPrintf(tszArgs, MAX_COMMAND_LINE, 
        TEXT("-CEOID %u"), ceoid);

    CreateProcess(szGmapsPath, tszArgs,
        NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
}
*/

void RunDialer() {
    PROCESS_INFORMATION pi;

    TCHAR szDialerPath[MAX_PATH];
    GetModuleFileName(NULL, szDialerPath, MAX_PATH);
    TCHAR * pstr = _tcsrchr(szDialerPath, '\\');
    if (pstr) *(++pstr) = '\0';
    StringCchCat(szDialerPath, MAX_PATH, TEXT("..\\iDialer\\iDialer.exe"));

    BOOL success = CreateProcess(szDialerPath, NULL,
        NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);

    if (!success) {
        CreateProcess(TEXT("cprog.exe"), NULL, 
            NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
    }
}

void AddContactByNumber(TCHAR * pNumber) {
    HRESULT hr = S_OK;
    IContact * pContact = NULL;
    IItem * pItem = NULL;
    IPOutlookApp2 * polApp;
	BSTR bstrNumber;

    hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IPOutlookApp2), (LPVOID*) &polApp);
    CHR(hr);
    hr = polApp->Logon(NULL);
    CHR(hr);

    hr = polApp->CreateItem(olContactItem, (IDispatch**)&pContact);
    CHR(hr);

	bstrNumber = ::SysAllocString(pNumber);
    pContact->put_MobileTelephoneNumber(bstrNumber);

    hr = ((IDispatch*)pContact)->QueryInterface(__uuidof(IItem), 
        (LPVOID*)&pItem);

    CHR(hr);

	hr = pItem->Edit(NULL);

Error:
    // cleanup
    RELEASE_OBJ(pContact);
    RELEASE_OBJ(polApp);
	::SysFreeString(bstrNumber);

    return;
}