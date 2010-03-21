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
#include <initguid.h>

#include "GraphicFunctions.h"
#include "ListDataPoom.h"
#include "PhoneUtils.h"
#include "ShortcutUtils.h"
#include "FileUtils.h"

IPOutlookApp2 *             polApp;
IFolder *                   pCurrFldr;
IPOutlookItemCollection *   pItemCol;
const TCHAR *               tszCategory;
UINT                        nBitmapHeight;
UINT                        nBitmapWidth;


// Internal functions to be used by the functions in this file
HRESULT initPoom();
HRESULT PoomSaveWM65StartIcon(DataItem * data, TCHAR * szName);
HRESULT PoomDeleteWM65StartIcon(TCHAR * szName);

// http://msdn.microsoft.com/en-us/library/bb446087.aspx
HRESULT PoomCategoriesPopulate(DataItem * parent, void (*adder)(DataItem*),
                     CSettings * pSettings) {

    HRESULT           hr = E_FAIL;
    IFolder    * pFolder = NULL;
    IItem * pFolderIItem = NULL;
    CEPROPVAL    * pVals = NULL;
    ULONG       cbBuffer = 0;
    int            index = 0;
    DataItem        data = {0};

    data.ID = 1;
    data.iGroup = 0;
    data.isFavorite = false;
    data.isMissed = false;
    data.oId = 0;
    data.type = diListItem;

    data.ID = 0;
    StringCchCopy(data.szPrimaryText, 64, pSettings->allcontacts_string);
    adder(&data);

    data.ID = 1;

    CEPROPID rgPropID = PIMPR_FOLDER_CATEGORIES;

    initPoom();
    HANDLE hHeap = GetProcessHeap();

    // Get the IFolder object (Contacts, Contacts, Tasks).
    hr = polApp->GetDefaultFolder(olFolderContacts, &pFolder);
    CHR(hr);

    // Get the IItem object representing a IFolder object.
    hr = pFolder->QueryInterface(__uuidof(IItem), (LPVOID*)&pFolderIItem);
    CHR(hr);

    // Get the list of categories.
    hr = pFolderIItem->GetProps(&rgPropID, CEDB_ALLOWREALLOC, 1, &pVals, &cbBuffer, hHeap);
    CHR(hr);

    // Copy the list of categories for use outside of this function.
    TCHAR * start = pVals->val.lpwstr;
    TCHAR * end = start + _tcslen(pVals->val.lpwstr);
    TCHAR * comma;
    while (start < end) {
        comma = _tcschr(start, ',');
        if (comma == NULL) comma = end;
        hr = StringCchCopyN(data.szPrimaryText, 64, start, comma - start);
        data.isFavorite = 0 == _tcscmp(data.szPrimaryText, pSettings->favorite_category);
        CHR(hr);
        adder(&data);
        start = comma + 2;
    }

Error:
    // Free resources.
    HeapFree(hHeap, 0, pVals);
    RELEASE_OBJ(pFolderIItem);
    RELEASE_OBJ(pFolder);

    return index;
}

HRESULT PoomCategoriesGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                               CSettings * pSettings) {

    StringCchCopy(buffer, cchDest, pSettings->categories_string);
    return S_OK;
}

HRESULT PoomCategoriesClick(DataItem * data, float x, 
                            int * newScreen, CSettings * pSettings) {
    
    if (_tcsstr(data->szPrimaryText, pSettings->allcontacts_string))
        *newScreen = 2;
    else if (_tcsstr(data->szPrimaryText, pSettings->favorite_category))
        *newScreen = 0;
    else
        *newScreen = 6;
    return S_OK;
}

// *************************************************
// These functions are for the main list of contacts
// *************************************************

HRESULT PoomPopulate(DataItem * parent, void (*adder)(DataItem*),
                     CSettings * pSettings) {

    HRESULT hr = S_OK;
    BSTR bstrPrimary = NULL;
    BSTR bstrCategories = NULL;
    TCHAR buffer[PRIMARY_TEXT_LENGTH];

    // This will hold our "next" list item, which we'll later
    // pass to this->_addListItem
    DataItem data = {0};
    data.iGroup = 0;
    data.isFavorite = false;
    data.isMissed = false;
    data.type = diListItem;

    hr = initPoom();
    CHR(hr);

    // Get the contacts folder.
    hr = polApp->GetDefaultFolder(olFolderContacts, &pCurrFldr);
    CHR(hr);

    // Get the contacts Items collection.
    hr = pCurrFldr->get_Items(&pItemCol);
    CHR(hr);

    // parent's ID == 0 means: use all contacts
    // parent's ID == 1 means: restrict dataset to parent's primary text
    if (1 == parent->ID) {
        StringCchPrintf(buffer, PRIMARY_TEXT_LENGTH, 
            TEXT("[Categories] = \"%s\""), parent->szPrimaryText);
        hr = pItemCol->Restrict((BSTR)buffer, &pItemCol);
        CHR(hr);
    }

	IContact * pContact = NULL;
	
    int cItems;
	pItemCol->get_Count(&cItems);

	for (int i = 0; i < cItems; i++) {

        // the itemCollection is indexed 1-based, weird...
        hr = pItemCol->Item(i+1, (IDispatch**)&pContact);
        if (FAILED(hr))
            continue;

	    // grab properties
	    hr = pContact->get_Oid(&data.oId);
        CHR(hr);

        hr = pContact->get_FileAs(&bstrPrimary);
        CHR(hr);
        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, bstrPrimary);
        SysFreeString(bstrPrimary);

        // Determine if this is a favorite 
        hr = pContact->get_Categories(&bstrCategories);
        CHR(hr);
        data.isFavorite = NULL != 
            _tcsstr(bstrCategories, pSettings->favorite_category);
        SysFreeString(bstrCategories);

	    // Don't display groups if filtered by category
        if (NULL == tszCategory) {
		    data.iGroup = (int)_totupper(data.szPrimaryText[0]);
        }

        adder(&data); 

		// clean up
        RELEASE_OBJ(pContact);
    }

    hr = S_OK;

Error:
    SysFreeString(bstrPrimary);
    SysFreeString(bstrCategories);
	RELEASE_OBJ(pContact);
    return hr;
}

HRESULT PoomClick(DataItem * data, float x,  
                  int * newScreen, CSettings * pSettings) {
    *newScreen = 4;
    return S_OK;
}

HRESULT PoomGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                               CSettings * pSettings) {

    StringCchCopy(buffer, cchDest, parent->szPrimaryText);
    return S_OK;
}

HRESULT PoomGetGroup(DataItem * data, TCHAR * buffer, int cchDest,
                     CSettings * pSettings) {

    buffer[0] = (TCHAR)data->iGroup;
    buffer[1] = 0;
    return S_OK;
}

HRESULT PoomAddItem() {
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    HRESULT hr = S_OK;
    HWND hWnd;

    hr = initPoom();
    CHR(hr);

    hr = polApp->CreateItem(olContactItem, (IDispatch**)&pDisp);
    CHR(hr);

    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

    hWnd = FindWindow (SZ_APP_NAME, NULL);
	hr = pItem->Edit(hWnd);

Error:
	RELEASE_OBJ(pItem);
    RELEASE_OBJ(pDisp);
    return hr;
}

// *************************************************
// These functions are for the favorites list of contacts
// *************************************************


// *************************************************
// These functions are for a contact details screen
// *************************************************

HRESULT PoomDetailsPopulate(DataItem * parent, void (*adder)(DataItem*),
                            CSettings * pSettings) {
    HRESULT hr = S_OK;
    IContact * pContact = NULL;
    BSTR buffer = NULL;
    BSTR bstrCategories = NULL;
    int id = parent->ID;
    TCHAR * szName = parent->szPrimaryText;

    // Initialize the data item that we're going to write
    DataItem data = {0};
    data.ID = 0;
    data.iGroup = 0;
    data.isFavorite = false;
    data.isMissed = false;
    data.oId = parent->oId;

    hr = initPoom();
    CHR(hr);

    // GET CURRENT CONTACT
    hr = polApp->GetItemFromOid((CEOID)data.oId, (IDispatch**)&pContact);
    CHR(hr);

    //***** GET CONTACT DETAILS *****
    
    // NAME
    pContact->get_FileAs(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    data.type = diName;
    adder(&data); 

    // COMPANY NAME
    pContact->get_CompanyName(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    data.type = (SysStringLen(buffer) > 0) ? diCompany : diNothing;
    adder(&data); 

    
    
    // Phone numbers
    data.type = diPhone;

    // MOBILE TELEPHONE NUMBER
    pContact->get_MobileTelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->mobile_string);
        adder(&data); 
    }

    // HOME TELEPHONE NUMBER 1
    pContact->get_HomeTelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->home_string);
        adder(&data); 
    }

    // HOME TELEPHONE NUMBER 2
    pContact->get_Home2TelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->home_string);
        adder(&data); 
    }

    // Home Fax Number
    pContact->get_HomeFaxNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->fax_string);
        adder(&data); 
    }
	
    // BUSINESS TELEPHONE NUMBER 1
    pContact->get_BusinessTelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->work_string);
        adder(&data); 
    }
	
    // BUSINESS TELEPHONE NUMBER 2
    pContact->get_Business2TelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->work_string);
        adder(&data); 
    }

    // Business Fax Number
    pContact->get_BusinessFaxNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->fax_string);
        adder(&data); 
    }

    // Company phone number... not a standard property!
    IItem* pItem = NULL;
    hr = ((IDispatch*)pContact)->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

    CEPROPID rgPropId = PIMPR_COMPANY_TELEPHONE_NUMBER;
    CEPROPVAL *prgPropvalPoom = NULL;
    ULONG cbBuffer = 0;
    HANDLE hHeap = ::GetProcessHeap(); 

    // let Outlook Mobile allocate memory, then get item properties.
    hr = pItem->GetProps(&rgPropId, CEDB_ALLOWREALLOC, 1, 
        &prgPropvalPoom, &cbBuffer, hHeap);

    if (SUCCEEDED(hr) && NULL != prgPropvalPoom && cbBuffer > 0 
        && LOWORD(prgPropvalPoom->propid) == CEVT_LPWSTR 
        && ! (prgPropvalPoom->wFlags & CEDB_PROPNOTFOUND)
        && NULL != prgPropvalPoom->val.lpwstr) {

        StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, prgPropvalPoom->val.lpwstr);
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->company_string);
        adder(&data); 
    }

    // Free memory.
    HeapFree(hHeap, 0, prgPropvalPoom);
    RELEASE_OBJ(pItem);


    // Car Telephone Number
    pContact->get_CarTelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->car_string);
        adder(&data); 
    }

    // Pager Number
    pContact->get_PagerNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->pager_string);
        adder(&data); 
    }

    // Radio Telephone
    pContact->get_RadioTelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->radio_string);
        adder(&data); 
    }

    // Assistant Telephone Number
    pContact->get_AssistantTelephoneNumber(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        StringCchCopy(data.szSecondaryText, SECONDARY_TEXT_LENGTH, pSettings->assistant_string);
        adder(&data); 
    }

    // "EMAIL" Category
    data.type = diEmail;
    data.szSecondaryText[0] = 0;

    // EMAIL ADDRESS 1
    pContact->get_Email1Address(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        adder(&data); 
    }
	
    // EMAIL ADDRESS 2
    pContact->get_Email2Address(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        adder(&data); 
    }
	
    // EMAIL ADDRESS 3
    pContact->get_Email3Address(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        adder(&data); 
    }

    // URL
    data.type = diUrl;
    pContact->get_WebPage(&buffer);
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, buffer);
    SysFreeString(buffer);
    if (_tcslen(data.szPrimaryText)) {
        adder(&data); 
    }

    // Button to show details
    data.type = diDetailsButton;
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->managecontact_string);
    SysFreeString(buffer);
    adder(&data); 

    // Button to edit contact
    data.type = diEditButton;
    StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->editcontact_string);
    SysFreeString(buffer);
    adder(&data); 

	// Add either "Create Shortcut" or "Remove Shortcut"
	// depending on whether the shortcut exists.
	pContact->get_FileAs(&buffer);
	TCHAR szLinkPath[MAX_PATH] = {0};
	BOOL shortcutExists = GetShortcutFilename(szLinkPath, buffer);
    SysFreeString(buffer);
	if (shortcutExists) {
		data.type = diRemoveShortcutButton;
		StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->removeshortcut_string);
	}
	else {
		data.type = diCreateShortcutButton;
		StringCchCopy(data.szPrimaryText, PRIMARY_TEXT_LENGTH, pSettings->createshortcut_string);
	}
    adder(&data); 

    hr = S_OK;

Error:
    // cleanup
    if (NULL != bstrCategories)
        SysFreeString(bstrCategories);
    if (NULL != buffer)
        SysFreeString(buffer);
    RELEASE_OBJ(pContact);

    return hr;
}

HRESULT PoomDetailsGetTitle(DataItem * parent, TCHAR * buffer, int cchDest,
                               CSettings * pSettings) {

    StringCchCopy(buffer, cchDest, pSettings->details_string);
    return S_OK;
}

HRESULT PoomDetailsClick(DataItem * data, float x, 
                         int * newScreen, CSettings * pSettings) {
    HRESULT hr = S_OK;
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    IContact * pContact = NULL;
    BSTR buffer = NULL;
    HWND hWnd = 0;

    TCHAR szObjectPath[MAX_PATH] = {0};
    TCHAR szArguments[64] = {0};
    TCHAR szIconPath[MAX_PATH] = {0};

    int oid = data->oId;
    if (oid == -1)
        return E_INVALIDARG;

    hr = initPoom();
    CHR(hr);

    hr = polApp->GetItemFromOid(oid, &pDisp);
    CHR(hr);

    switch (data->type) {
        case diEditButton:
            hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
            CHR(hr);
            hWnd = FindWindow (SZ_APP_NAME, NULL);
        	hr = pItem->Edit(hWnd);
            break;

        case diDetailsButton:
            hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
            CHR(hr);
            hWnd = FindWindow (SZ_APP_NAME, NULL);
        	hr = pItem->Display(hWnd);
            CHR(hr);
            break;

        case diCreateShortcutButton:
            ::GetModuleFileName(NULL, szObjectPath, MAX_PATH);
            StringCchPrintf(szArguments, 64, TEXT("-details %ld"), oid);
            
            hr = pDisp->QueryInterface(IID_IContact, (LPVOID*)&pContact);
            CHR(hr);
            pContact->get_FileAs(&buffer);

            CreateShortcutFile(buffer, szObjectPath, szArguments, NULL);
            PoomSaveWM65StartIcon(data, buffer);

            *newScreen = NEWSCREEN_BACK;
            break;

		case diRemoveShortcutButton:
            hr = pDisp->QueryInterface(IID_IContact, (LPVOID*)&pContact);
            CHR(hr);
            pContact->get_FileAs(&buffer);
            RemoveShortcutFile(buffer);
            PoomDeleteWM65StartIcon(buffer);

            *newScreen = NEWSCREEN_BACK;
			break;

        case diPhone:
            hr = pDisp->QueryInterface(IID_IContact, (LPVOID*)&pContact);
            CHR(hr);
            pContact->get_FileAs(&buffer);
            
            if (x > 0.8)
                SendSMS(data->szPrimaryText, buffer);
            else
                Call(data->szPrimaryText, buffer);

            SysFreeString(buffer);
            *newScreen = NEWSCREEN_BACK_ON_DEACTIVATE;
            break;

        case diEmail:
            SendEMail(pSettings->email_account, data->szPrimaryText);
            *newScreen = NEWSCREEN_BACK_ON_DEACTIVATE;
            break;

        case diUrl:
            OpenURL(data->szPrimaryText);
            *newScreen = NEWSCREEN_BACK_ON_DEACTIVATE;
            break;

    }

    hr = S_OK;

Error:
	RELEASE_OBJ(pItem);
	RELEASE_OBJ(pContact);
    RELEASE_OBJ(pDisp);
    return hr;
}

HRESULT PoomDetailsToggleFavorite(DataItem * data, CSettings * pSettings) {
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    HRESULT hr = S_OK;

    int oid = data->oId;
    if (oid == -1)
        return E_INVALIDARG;

    hr = initPoom();
    CHR(hr);

    hr = polApp->GetItemFromOid(oid, &pDisp);
    CHR(hr);

    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

    // update the categories in the POOM
    hr = (data->isFavorite)
        ? pItem->RemoveCategory(pSettings->favorite_category)
        : pItem->AddCategory(pSettings->favorite_category);
    CHR(hr);

    // category update succeeded
    hr = pItem->Save();

    // update the list data
    data->isFavorite = !data->isFavorite;

Error:
    RELEASE_OBJ(pItem);
    RELEASE_OBJ(pDisp);

    return hr;
}


// *************************************************
// Internal functions to be used only by the functions in this file
// *************************************************

HRESULT initPoom() {
	HWND      hWnd     = 0;
    HRESULT   hr       = 0;
    IFolder   *pFolder = NULL;
    IItem     *pItem   = NULL;
    CEPROPVAL propval  = {0};
	
    if (polApp == NULL) {
        hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER,
                              __uuidof(IPOutlookApp2), (LPVOID*) &polApp);
        CHR(hr);
    
		hWnd = FindWindow (SZ_APP_NAME, NULL);
        hr = polApp->Logon((long)hWnd);
        CHR(hr);
		
		// register for updates
		// http://msdn.microsoft.com/en-us/library/ms862902.aspx
    
	    // Get the folder for contacts.
	    hr = polApp->GetDefaultFolder(olFolderContacts, &pFolder);
        CHR(hr);

	    // Get the IItem interface for the IFolder.
	    hr = pFolder->QueryInterface(IID_IItem, (LPVOID*)&pItem);
        CHR(hr);

	    // Set the folder's properties.
	    propval.propid    = PIMPR_FOLDERNOTIFICATIONS;
	    propval.val.ulVal = PIMFOLDERNOTIFICATION_ALL;
	    hr                = pItem->SetProps(0, 1, &propval);

        CHR(hr);
    }

    hr = S_OK;

Error:
	RELEASE_OBJ(pItem);
	RELEASE_OBJ(pFolder);
    if (FAILED(hr)) {
        // If we failed to log on, don't keep the object around
        RELEASE_OBJ(polApp);
    }
    return hr;
}

// **************************************************************************
// Function Name: PoomDetailsLoadBitmap
// 
// Purpose: given a DataItem pointer, finds an associated bitmap 
// and its dimensions
//
// Arguments:
//    IN  int      index    - index of the contact
//    IN  HBITMAP* phBitmap - bitmap of the contact's picture
//    OUT UINT*    puWidth  - width of the bitmap
//    OUT UINT*    puHeight - height of the bitmap
//
// Return Values:
//    HRESULT - S_OK if success, failure code if not
//

HRESULT PoomDetailsLoadBitmap(DataItem * data, HBITMAP * phBitmap, 
                              UINT * puWidth, UINT * puHeight) {

    HRESULT   hr;
    IItem*    pItem = NULL;
    IStream*  pStream = NULL;
    ULONG     ulSize;

    // Make sure we can access POOM
    hr = initPoom();
    CHR(hr);

    hr = polApp->GetItemFromOidEx(data->oId, 0, &pItem);
    CHR(hr);
    
    // Extract the picture from the contact
    hr = pItem->OpenProperty(PIMPR_PICTURE, GENERIC_READ, &pStream);
    CHR(hr);
    
    hr = GetStreamSize(pStream, &ulSize);
    CHR(hr);

    // In some cases, the property may exist even if there is no picture.
    // Make sure we can access the stream and don't have a 0 byte stream
    CBR(ulSize > 0);

    hr = GetBitmapFromStream(pStream, phBitmap, puWidth, puHeight);
    CHR(hr);

Error:
    RELEASE_OBJ(pItem);
    RELEASE_OBJ(pStream);

    return hr;
}

HRESULT PoomSaveWM65StartIcon(DataItem * data, TCHAR * szName) {
	HRESULT hr = E_FAIL;
	UINT uWidth = 90;
	UINT uHeight = 90;


	// Load the user's avatar onto a bitmap in memory
	HDC hdcForeground = GetDC(GetForegroundWindow());
	HDC hdc = CreateCompatibleDC(hdcForeground);
	HBITMAP hbitmap = NULL;
	// May fail if user doesn't have an avatar
	hr = PoomDetailsLoadBitmap(data, &hbitmap, &uWidth, &uHeight);
	CHR(hr);
	HGDIOBJ hold = SelectObject(hdc, hbitmap);
#ifdef DEBUG
	BitBlt(hdcForeground, 0, 0, uWidth, uHeight, hdc, 0, 0, SRCCOPY);
#endif

	// copy the image to a DIBSection
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = uWidth;
	bmi.bmiHeader.biHeight = -1 * uHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB; 
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	BYTE * pBuffer = NULL;

	HDC hdc2 = CreateCompatibleDC(hdcForeground);
	HBITMAP hbitmap2 = CreateDIBSection(hdc2, &bmi, DIB_RGB_COLORS,
		(void **)&pBuffer, NULL, 0);

	HGDIOBJ hold2 = SelectObject(hdc2, hbitmap2);

	BitBlt(hdc2, 0, 0, uWidth, uHeight, hdc, 0, 0, SRCCOPY);

	// create an imaging factory
    IImagingFactory* pFactory = NULL;
    hr = CoCreateInstance(CLSID_ImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        IID_IImagingFactory, (void**) &pFactory);
    CHR(hr);

	// Save the image
	TCHAR szFullPath[MAX_PATH];
	GetCurDirFilename(szFullPath, szName, TEXT("png"));
	hr = SavePNG(hdc2, hbitmap2, szFullPath, pFactory);
	CHR(hr);
	
	// Update registry
	// http://windowsteamblog.com/blogs/windowsphone/archive/2009/08/11/using-custom-icons-in-windows-mobile-6-5.aspx
	// [HKEY_LOCAL_MACHINE\Security\Shell\StartInfo\Start\Phone.lnk]
	// "Icon"="\Application Data\My App\newphoneicon.png"
	HKEY  hkey = 0;
    DWORD dwDisposition = 0;
    DWORD dwType = REG_SZ;
	TCHAR szSubKey[MAX_PATH];
	StringCchPrintf(szSubKey, MAX_PATH, TEXT("\\Security\\Shell\\StartInfo\\Start\\%s.lnk"), szName);

	// create (or open) the specified registry key
    LONG result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 
        0, NULL, 0, 0, NULL, &hkey, &dwDisposition);
    CBR(result == ERROR_SUCCESS);

    DWORD dwSize = (_tcslen(szFullPath) + 1) * sizeof(TCHAR);
    result = RegSetValueEx(hkey, TEXT("Icon"), NULL, dwType, (PBYTE)szFullPath, dwSize);

	hr = S_OK;

Error:
    if (hkey != NULL)
        RegCloseKey(hkey);
	RELEASE_OBJ(pFactory);
	SelectObject(hdc, hold);
	SelectObject(hdc2, hold2);
	return hr;
}

HRESULT PoomDeleteWM65StartIcon(TCHAR * szName) {
	// Delete registry key
	TCHAR szSubKey[MAX_PATH];
	StringCchPrintf(szSubKey, MAX_PATH, TEXT("\\Security\\Shell\\StartInfo\\Start\\%s.lnk"), szName);
	LONG result = RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);

	// Delete image file
	TCHAR szFullPath[MAX_PATH];
	GetCurDirFilename(szFullPath, szName, TEXT("png"));
	DeleteFile(szFullPath);

	return S_OK;
}