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
#include "FileUtils.h"
#include "Macros.h"

#include <pimstore.h>

// dynamically loaded function
typedef LONG (* TAPIREQUESTMAKECALL)(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR);

#define MAX_COMMAND_LINE 200
#define MAX_LOADSTRING 100

// main functions

void Call(TCHAR * number, TCHAR * name) {
    TCHAR szFilename[MAX_PATH];
    PROCESS_INFORMATION pi;
    TCHAR szArguments[MAX_PATH];

	HINSTANCE hiCellCoreDll;
	TAPIREQUESTMAKECALL tapiRequestMakeCall;

    if (GetIDialerFilename(szFilename)) {
        StringCchPrintf(szArguments, MAX_PATH, TEXT("%s -name=%s"), number, name);
        CreateProcess(szFilename, szArguments,
            NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
    }
    else {
		hiCellCoreDll = LoadLibrary(TEXT("cellcore.dll"));
		if (!hiCellCoreDll) {
			// TODO: alert user?
			return;
		}

		tapiRequestMakeCall = (TAPIREQUESTMAKECALL)GetProcAddress(
			hiCellCoreDll, TEXT("tapiRequestMakeCallW"));

        tapiRequestMakeCall(number, NULL, name, NULL);
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
    
    //TODO: figure out why this function behaves badly
    StringCchPrintf(tszAccountName, MAX_LOADSTRING, TEXT("ActiveSync"));
    return;

    /*HRESULT hr;
    IMAPITable * ptbl;
    IMAPISession * pSession;
    SRowSet *prowset = NULL;
    SPropValue *pval = NULL;
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
 
        hr = ptbl->QueryRows(1, 0, &prowset);
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
Error:
    RELEASE_OBJ(ptbl);

    FreeProws(prowset);

    pSession->Logoff(0, 0, 0);
    RELEASE_OBJ(pSession);
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
 
    TCHAR szGmapsPath[MAX_PATH];
    TCHAR tszArgs[MAX_COMMAND_LINE];

    GetCurDirFilename(szGmapsPath, TEXT("..\\GoogleMaps\\GoogleMaps.exe"));
    StringCchPrintf(tszArgs, MAX_COMMAND_LINE, TEXT("-CEOID %u"), ceoid);

    CreateProcess(szGmapsPath, tszArgs,
        NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
}
*/

void RunDialer() {
    PROCESS_INFORMATION pi;

    TCHAR szDialerPath[MAX_PATH];

    if (GetIDialerFilename(szDialerPath)) {
        CreateProcess(szDialerPath, NULL,
            NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
    }

    else {
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
    HWND hWnd;

    hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IPOutlookApp2), (LPVOID*) &polApp);
    CHR(hr);

    hWnd = GetForegroundWindow();
    hr = polApp->Logon((long)hWnd);
    CHR(hr);

    hr = polApp->CreateItem(olContactItem, (IDispatch**)&pContact);
    CHR(hr);

	bstrNumber = ::SysAllocString(pNumber);
    pContact->put_MobileTelephoneNumber(bstrNumber);

    hr = ((IDispatch*)pContact)->QueryInterface(__uuidof(IItem), 
        (LPVOID*)&pItem);

    CHR(hr);

	hr = pItem->Edit(hWnd);

Error:
    // cleanup
    RELEASE_OBJ(pContact);
    RELEASE_OBJ(polApp);
	::SysFreeString(bstrNumber);

    return;
}
