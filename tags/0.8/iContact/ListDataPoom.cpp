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

ListDataPoom::ListDataPoom(CSettings * pSettings) {
    this->_construct(pSettings, false);
    this->_canFavorite = true;
    this->_canAdd = true;
}

ListDataPoom::ListDataPoom(CSettings * pSettings, bool bOnlyFavorites) {
    this->_construct(pSettings, bOnlyFavorites);
    this->_canFavorite = true;
    this->_canAdd = false;
}

void ListDataPoom::_construct(CSettings * pSettings, bool bOnlyFavorites) {
    HRESULT hr;

    this->polApp = NULL;
    this->pCurrFldr = NULL;
    this->pItemCol = NULL;

    this->_settings = pSettings;
	this->_bOnlyFavorites = bOnlyFavorites;

    TCHAR buffer[64];

    hr = this->_initPoom();
    CHR(hr);

    // Get the contacts folder.
    hr = this->polApp->GetDefaultFolder(olFolderContacts, &this->pCurrFldr);
    CHR(hr);

    // Get the contacts Items collection.
    hr = this->pCurrFldr->get_Items(&this->pItemCol);
    CHR(hr);

    if (this->_bOnlyFavorites) {
        // Restrict the collection to Contacts in our favorite category
        StringCchPrintf(buffer, 64, TEXT("[Categories] = \"%s\""), 
            pSettings->favorite_category);
        hr = this->pItemCol->Restrict((BSTR)buffer, &this->pItemCol);
        CHR(hr);
    }

    hr = this->Populate();

Error:
    if (FAILED(hr)) {
        RELEASE_OBJ(this->pCurrFldr);
        RELEASE_OBJ(this->pItemCol);
    }
}

void ListDataPoom::Release(void) {
    RELEASE_OBJ(this->pCurrFldr);
    RELEASE_OBJ(this->pItemCol);
    RELEASE_OBJ(this->polApp);
    if (this->_hBitmap)
        DeleteObject((HGDIOBJ)this->_hBitmap);
    this->_hBitmap = NULL;

}

HRESULT ListDataPoom::Populate() {
    HRESULT hr = S_OK;
	BSTR bstrFileAs = NULL;
    BSTR bstrCategories = NULL;
	LONG lOid;
    TCHAR wcGroup = 0;

	// grpBuf is the "Group" or first letter of the name 
	// that this contact belongs to
    TCHAR grpBuf[1] = {0};

	IContact * pContact = NULL;
	int cItems = 0;
	bool bOnlyFavorites = this->_bOnlyFavorites;

    COLORREF rgbPrimary = this->_settings->rgbListItemText;

	this->pItemCol->get_Count(&cItems);

    this->_arrayLength = cItems;

    this->_items = new Data[this->_arrayLength];

	for (int i = 0; i < cItems; i++) {
        // the itemCollection is indexed 1-based, weird...
        hr = this->pItemCol->Item(i+1, (IDispatch**)&pContact);
        if (FAILED(hr))
            continue;

	    // grab properties
	    hr = pContact->get_Oid(&lOid);
        CHR(hr);

        hr = pContact->get_FileAs(&bstrFileAs);
        CHR(hr);

	    // Don't display groups if showing favorites
        if (bOnlyFavorites) {
            this->_items[i].isFavorite = true;
        }
        else {
		    wcGroup = _totupper(bstrFileAs[0]);

            // Determine if this is a favorite 
            hr = pContact->get_Categories(&bstrCategories);
            CHR(hr);
            
            this->_items[i].isFavorite = NULL != 
                _tcsstr(bstrCategories, this->_settings->favorite_category);

            rgbPrimary = this->_items[i].isFavorite
                ? this->_settings->rgbListItemFavoriteText
                : this->_settings->rgbListItemText;

            SysFreeString(bstrCategories);
        }

		this->_addListItem(NULL, bstrFileAs, TEXT(""), 
            wcGroup, lOid, rgbPrimary);

		// clean up
        SysFreeString(bstrFileAs);
        RELEASE_OBJ(pContact);
    }

    hr = S_OK;

Error:
    SysFreeString(bstrFileAs);
    SysFreeString(bstrCategories);
	RELEASE_OBJ(pContact);
    return hr;
}

HRESULT ListDataPoom::PopulateDetailsFor(int index) {
    HRESULT hr = S_OK;
    IContact * pContact = NULL;
    BSTR buffer = NULL;
    BSTR bstrCategories = NULL;
    int id = this->_items[index].ID;
    CEOID oid = this->_items[index].oId;
    TCHAR * szName = this->_items[index].szPrimaryText;
    
    if (NULL != this->_hBitmap)
        DeleteObject((HGDIOBJ)this->_hBitmap);

    this->_hBitmap = NULL;
	this->_itemDetailCount = 0;
    this->_currentDetailTitle = this->_settings->details_string;
    this->_currentItemIndex = index;

    // GET CURRENT CONTACT
    hr = this->polApp->GetItemFromOid(oid, (IDispatch**)&pContact);
    CHR(hr);


    //***** GET CONTACT DETAILS *****//
    
    // NAME
    this->_addDetail(diName, szName);

    // COMPANY NAME
    pContact->get_CompanyName(&buffer);
    if (SysStringLen(buffer) > 0)
        this->_addDetail(diCompany, buffer);
    else //if (NULL != this->_hBitmap)
        this->_addDetail(diNothing, NULL);
    SysFreeString(buffer);

    
    
    // Phone numbers

    // MOBILE TELEPHONE NUMBER
    pContact->get_MobileTelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->mobile_string);
    SysFreeString(buffer);

    // HOME TELEPHONE NUMBER 1
    pContact->get_HomeTelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->home_string);
    SysFreeString(buffer);

    // HOME TELEPHONE NUMBER 2
    pContact->get_Home2TelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->home_string);
    SysFreeString(buffer);

    // Home Fax Number
    pContact->get_HomeFaxNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->fax_string);
    SysFreeString(buffer);
	
    // BUSINESS TELEPHONE NUMBER 1
    pContact->get_BusinessTelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->work_string);
    SysFreeString(buffer);
	
    // BUSINESS TELEPHONE NUMBER 2
    pContact->get_Business2TelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->work_string);
    SysFreeString(buffer);

    // Business Fax Number
    pContact->get_BusinessFaxNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->fax_string);
    SysFreeString(buffer);


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
        this->_addDetail(diPhone, prgPropvalPoom->val.lpwstr, 
            this->_settings->company_string);
    }

    // Free memory.
    HeapFree(hHeap, 0, prgPropvalPoom);
    RELEASE_OBJ(pItem);


    // Car Telephone Number
    pContact->get_CarTelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->car_string);
    SysFreeString(buffer);

    // Pager Number
    pContact->get_PagerNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->pager_string);
    SysFreeString(buffer);

    // Radio Telephone
    pContact->get_RadioTelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->radio_string);
    SysFreeString(buffer);

    // Assistant Telephone Number
    pContact->get_AssistantTelephoneNumber(&buffer);
    this->_addDetail(diPhone, buffer, this->_settings->assistant_string);
    SysFreeString(buffer);

    // "EMAIL" Category

    // EMAIL ADDRESS 1
    pContact->get_Email1Address(&buffer);
    this->_addDetail(diEmail, buffer);
    SysFreeString(buffer);
	
    // EMAIL ADDRESS 2
    pContact->get_Email2Address(&buffer);
    this->_addDetail(diEmail, buffer);
    SysFreeString(buffer);
	
    // EMAIL ADDRESS 3
    pContact->get_Email3Address(&buffer);
    this->_addDetail(diEmail, buffer);
    SysFreeString(buffer);

    // URL
    pContact->get_WebPage(&buffer);
    this->_addDetail(diUrl, buffer);
    SysFreeString(buffer);

    // Button to show details
    this->_addDetail(diDetailsButton, this->_settings->managecontact_string);

    // Button to edit contact
    this->_addDetail(diEditButton, this->_settings->editcontact_string);

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

void ListDataPoom::ToggleFavorite() {
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    HRESULT hr = S_OK;
    int index = this->_currentItemIndex;

    int oid = this->_items[index].oId;
    if (oid == -1)
        return;

    hr = this->_initPoom();
    CHR(hr);

    hr = this->polApp->GetItemFromOid(oid, &pDisp);
    CHR(hr);

    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

    // update the categories in the POOM
    hr = (this->_items[index].isFavorite)
        ? pItem->RemoveCategory(this->_settings->favorite_category)
        : pItem->AddCategory(this->_settings->favorite_category);
    CHR(hr);

    // category update succeeded
    hr = pItem->Save();

    // update the list data
    this->_items[index].isFavorite = !this->_items[index].isFavorite;
    this->_items[index].rgbPrimaryText 
        = !this->_bOnlyFavorites && this->_items[index].isFavorite
        ? this->_settings->rgbListItemFavoriteText
        : this->_settings->rgbListItemText;


Error:
    RELEASE_OBJ(pItem);
    RELEASE_OBJ(pDisp);
}

void ListDataPoom::AddItem() {
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    HRESULT hr = S_OK;

    hr = this->_initPoom();
    CHR(hr);

    hr = this->polApp->CreateItem(olContactItem, (IDispatch**)&pDisp);
    CHR(hr);

    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

	hr = pItem->Edit(NULL);

Error:
	RELEASE_OBJ(pItem);
    RELEASE_OBJ(pDisp);
    return;
}

HRESULT ListDataPoom::DisplayItem() {
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    HRESULT hr = S_OK;
    int index = this->_currentItemIndex;

    int oid = this->_items[index].oId;
    if (oid == -1)
        return E_INVALIDARG;

    hr = this->_initPoom();
    CHR(hr);

    hr = this->polApp->GetItemFromOid(oid, &pDisp);
    CHR(hr);

    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

	hr = pItem->Display(NULL);
    CHR(hr);

    hr = this->PopulateDetailsFor(index);

Error:
	RELEASE_OBJ(pItem);
    RELEASE_OBJ(pDisp);
    return hr;
}

void ListDataPoom::EditItem() {
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    HRESULT hr = S_OK;

    int oid = this->_items[this->_currentItemIndex].oId;
    if (oid == -1)
        return;

    hr = this->_initPoom();
    CHR(hr);

    hr = this->polApp->GetItemFromOid(oid, &pDisp);
    CHR(hr);

    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
    CHR(hr);

	hr = pItem->Edit(NULL);

Error:
	RELEASE_OBJ(pItem);
    RELEASE_OBJ(pDisp);
}

HRESULT ListDataPoom::_initPoom() {
    HRESULT hr;

    if (this->polApp == NULL) {
        hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER,
                              __uuidof(IPOutlookApp2), (LPVOID*) &this->polApp);
        CHR(hr);

        hr = this->polApp->Logon(NULL);
        CHR(hr);
    }

    hr = S_OK;

Error:
    if (FAILED(hr)) {
        // If we failed to log on, don't keep the object around
        RELEASE_OBJ(this->polApp);
    }
    return hr;
}

// **************************************************************************
// Function Name: _loadBitmap
// 
// Purpose: given a list index, finds an associated bitmap 
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

HRESULT ListDataPoom::_loadBitmap(int size) {
    UINT*     puWidth = &this->_nBitmapWidth;
    UINT*     puHeight = &this->_nBitmapHeight;
    HBITMAP*  phBitmap = &this->_hBitmap;
    int       index = this->_currentItemIndex;
    CEOID     oid = this->_items[index].oId;
    HRESULT   hr;
    IItem*    pItem = NULL;
    IStream*  pStream = NULL;
    ULONG     ulSize;

    // Make sure we can access POOM
    hr = this->_initPoom();
    CHR(hr);

    this->_nBitmapHeight = this->_nBitmapWidth = size;

    hr = this->polApp->GetItemFromOidEx(oid, 0, &pItem);
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