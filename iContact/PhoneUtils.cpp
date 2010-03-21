#include "stdafx.h"
#include "PhoneUtils.h"
#include "Macros.h"

#include <pimstore.h>

#include "windows.h"
#include "regext.h"
#include "tapi.h"
#include "snapi.h"

#define MAX_COMMAND_LINE 200

// main functions

void Call(TCHAR * number, TCHAR * name) {
    PHONEMAKECALLINFO mci = {0};
    LRESULT lResult;

    mci.cbSize = sizeof(mci);
    mci.dwFlags = 0;
    //mci.pszDestAddress = tszPhoneNumber;
    //mci.pszDestAddress = ActionsList[SubListCurrentAction].text;
    mci.pszDestAddress = number;
    mci.pszCalledParty = name;
    lResult = PhoneMakeCall(&mci);
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
    TCHAR tszAccountName[64];

    // If account == "", then get the last used email account name from the phone
    if (_tcslen(account) > 0) {
        StringCchPrintf(tszCommandLine, MAX_COMMAND_LINE, 
            TEXT("-service \"%s\" -to \"%s\""), account, to);
    }
    else {
        // This should be the name of the last used email account
        RegistryGetString(
            SN_MESSAGINGLASTEMAILACCOUNTNAME_ROOT, 
            SN_MESSAGINGLASTEMAILACCOUNTNAME_PATH, 
            SN_MESSAGINGLASTEMAILACCOUNTNAME_VALUE, 
            tszAccountName, 64);

        StringCchPrintf(tszCommandLine, MAX_COMMAND_LINE, 
            TEXT("-service \"%s\" -to \"%s\""), tszAccountName, to);
    }

    CreateProcess(TEXT("tmail.exe"), tszCommandLine, 
        NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
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

    hr = ((IDispatch*)pContact)->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

	hr = pItem->Edit(NULL);

Error:
    // cleanup
    RELEASE_OBJ(pContact);
    RELEASE_OBJ(polApp);
	::SysFreeString(bstrNumber);

    return;
}