#include "stdafx.h"
#include "PhoneUtils.h"

#include "windows.h"
#include "regext.h"
#include "tapi.h"
#include "tsp.h"
#include "snapi.h"

#ifdef callvmail

#define EXIT_ON_NULL(_p) if (_p == NULL) { hr = E_OUTOFMEMORY; goto FuncExit; }
#define EXIT_ON_FALSE(_f) if (!(_f)) { hr = E_FAIL; goto FuncExit; }

#define TAPI_API_LOW_VERSION   0x00020000
#define TAPI_API_HIGH_VERSION  0x00020000

#define CAPS_BUFFER_SIZE    512

#define MAX_REG_SZ_CHARS 50

// function prototypes
HRESULT SHGetPhoneNumber(LPTSTR, UINT, UINT);
HRESULT SHReadLineAddressCaps(LPTSTR, UINT, PDWORD, UINT);

#endif

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

void RunDialer() {
    PROCESS_INFORMATION pi;

    CreateProcess(TEXT("cprog.exe"), TEXT(""), 
        NULL, NULL, FALSE, NULL, NULL, NULL, NULL, &pi);
}

#ifdef callvmail

void CallVmail() {
    TCHAR szVal[MAX_REG_SZ_CHARS];
    szVal[0] = '\0';

    HRESULT success;

    success = RegistryGetString(HKEY_CURRENT_USER, 
        TEXT("Software\\Microsoft\\Vmail"), TEXT("PhoneNumber1"), szVal,
        MAX_REG_SZ_CHARS); 

    if (SUCCEEDED(success) && _tcslen(szVal) > 5) {
        Call(szVal, TEXT("Voicemail"));
        return;
    }

    success = RegistryGetString(HKEY_CURRENT_USER, 
        TEXT("Software\\Microsoft\\Vmail"), TEXT("UserProvidedNumber1"), szVal,
        MAX_REG_SZ_CHARS); 

    if (SUCCEEDED(success) && _tcslen(szVal) > 5) {
        Call(szVal, TEXT("Voicemail"));
        return;
    }

    success = SHGetPhoneNumber(szVal, MAX_REG_SZ_CHARS, 1);
    if (SUCCEEDED(success)) {
        Call(szVal, TEXT("Voicemail"));
        return;
    }
}

// utility functions
 
// http://blogs.msdn.com/windowsmobile/archive/2004/11/28/271110.aspx
HRESULT SHReadLineAddressCaps(LPTSTR szNumber, UINT cchNumber, PDWORD pdwCallFwdModes, UINT nLineNumber) {
    HRESULT  hr = E_FAIL;
    LRESULT  lResult = 0;
    HLINEAPP hLineApp;
    DWORD    dwNumDevs;
    DWORD    dwAPIVersion = TAPI_API_HIGH_VERSION;
    LINEINITIALIZEEXPARAMS liep;

    DWORD dwTAPILineDeviceID;
    const DWORD dwAddressID = nLineNumber - 1;

    liep.dwTotalSize = sizeof(liep);
    liep.dwOptions   = LINEINITIALIZEEXOPTION_USEEVENT;

    if (SUCCEEDED(lineInitializeEx(&hLineApp, 0, 0, TEXT("ExTapi_Lib"), &dwNumDevs, &dwAPIVersion, &liep))) {

        BYTE* pCapBuf = NULL;
        DWORD dwCapBufSize = CAPS_BUFFER_SIZE;
        LINEEXTENSIONID  LineExtensionID;
        LINEDEVCAPS*     pLineDevCaps = NULL;
        LINEADDRESSCAPS* placAddressCaps = NULL;

        pCapBuf = new BYTE[dwCapBufSize];
        EXIT_ON_NULL(pCapBuf);

        pLineDevCaps = (LINEDEVCAPS*)pCapBuf;
        pLineDevCaps->dwTotalSize = dwCapBufSize;

        // Get TSP Line Device ID
        dwTAPILineDeviceID = 0xffffffff;
        for (DWORD dwCurrentDevID = 0 ; dwCurrentDevID < dwNumDevs ; dwCurrentDevID++) {
            if (0 == lineNegotiateAPIVersion(hLineApp, dwCurrentDevID, TAPI_API_LOW_VERSION, TAPI_API_HIGH_VERSION,
                &dwAPIVersion, &LineExtensionID))
            {
                lResult = lineGetDevCaps(hLineApp, dwCurrentDevID, dwAPIVersion, 0, pLineDevCaps);

                if (dwCapBufSize < pLineDevCaps->dwNeededSize) {
                    delete[] pCapBuf;
                    dwCapBufSize = pLineDevCaps->dwNeededSize;
                    pCapBuf = new BYTE[dwCapBufSize];
                    EXIT_ON_NULL(pCapBuf);

                    pLineDevCaps = (LINEDEVCAPS*)pCapBuf;
                    pLineDevCaps->dwTotalSize = dwCapBufSize;

                    lResult = lineGetDevCaps(hLineApp, dwCurrentDevID, dwAPIVersion, 0, pLineDevCaps);
                }

                if ((0 == lResult) &&
                    (0 == _tcscmp((TCHAR*)((BYTE*)pLineDevCaps+pLineDevCaps->dwLineNameOffset), CELLTSP_LINENAME_STRING)))
                {
                    dwTAPILineDeviceID = dwCurrentDevID;
                    break;
                }
            }
        }

        placAddressCaps = (LINEADDRESSCAPS*)pCapBuf;
        placAddressCaps->dwTotalSize = dwCapBufSize;

        lResult = lineGetAddressCaps(hLineApp, dwTAPILineDeviceID, dwAddressID, dwAPIVersion, 0, placAddressCaps);

        if (dwCapBufSize < placAddressCaps->dwNeededSize) {
            delete[] pCapBuf;
            dwCapBufSize = placAddressCaps->dwNeededSize;
            pCapBuf = new BYTE[dwCapBufSize];
            EXIT_ON_NULL(pCapBuf);

            placAddressCaps = (LINEADDRESSCAPS*)pCapBuf;
            placAddressCaps->dwTotalSize = dwCapBufSize;

            lResult = lineGetAddressCaps(hLineApp, dwTAPILineDeviceID, dwAddressID, dwAPIVersion, 0, placAddressCaps);
        }

        if (0 == lResult) {
            if (szNumber) {
                szNumber[0] = _T('\0');

                EXIT_ON_FALSE(0 != placAddressCaps->dwAddressSize);

                // A non-zero dwAddressSize means a phone number was found
                ASSERT(0 != placAddressCaps->dwAddressOffset);
                PWCHAR tsAddress = (WCHAR*)(((BYTE*)placAddressCaps)+placAddressCaps->dwAddressOffset);

                StringCchCopy(szNumber, cchNumber, tsAddress);
            }

            // Record the allowed forwarding modes
            if (pdwCallFwdModes)
            {
                *pdwCallFwdModes = placAddressCaps->dwForwardModes;
            }

            hr = S_OK;
        }

        delete[] pCapBuf;
    } // End if ()

FuncExit:
    lineShutdown(hLineApp);

    return hr;
}

// szNumber - Out Buffer for the phone number
// cchNumber - size of sznumber in characters
// nLineNumber - which phone line (1 or 2) to get the number for
HRESULT SHGetPhoneNumber(LPTSTR szNumber, UINT cchNumber, UINT nLineNumber) {
    return SHReadLineAddressCaps(szNumber, cchNumber, NULL, nLineNumber);
}

#endif