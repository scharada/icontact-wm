#include "stdafx.h"
#include <initguid.h>

#include "ListDataPoom.h"


ListDataPoom::ListDataPoom(Settings * pSettings) {
    this->_settings = pSettings;
    this->_bOnlyFavorites = false;
    this->_canFavorite = true;

    this->InitPoom();
    GetPoomFolder(olFolderContacts, &this->pCurrFldr);
    this->pCurrFldr->get_Items(&this->pItemCol);
    this->Populate();
}

ListDataPoom::ListDataPoom(Settings * pSettings, bool bOnlyFavorites) {
    this->_settings = pSettings;
	this->_bOnlyFavorites = bOnlyFavorites;
    this->_canFavorite = false;

    this->InitPoom();
    GetPoomFolder(olFolderContacts, &this->pCurrFldr);
    this->pCurrFldr->get_Items(&this->pItemCol);
    this->Populate();
}

void ListDataPoom::Clear(void) {
    if (this->pCurrFldr) {
        this->pCurrFldr->Release();
        // no need to delete COM objects delete pCurrFldr;
    }
    if (this->pItemCol) {
        this->pItemCol->Release();
        // no need to delete COM objects delete pItemCol;
    }
    if (this->polApp) {
	    this->polApp->Logoff();
	    this->polApp->Release();
        // no need to delete COM objects delete this->polApp;
    }
}

void ListDataPoom::Populate() {
	BSTR bstrFileAs;
	BSTR bstrCategories;
	LONG lOid;
	wchar_t wcContactInfo[25];
	wchar_t wcCategories[25];
    wchar_t wcGroup[2];

	// grpBuf is the "Group" or first letter of the name 
	// that this contact belongs to
	wchar_t grpBuf[1] = L"";

	IContact * pContact = NULL;
	int cItems = 0;
	bool bOnlyFavorites = this->_bOnlyFavorites;

	// first item in the list is "Add new contact"
	if (!bOnlyFavorites) {
		wcsncpy(wcContactInfo, this->_settings->add_new_contact_string, 25);
		this->AddItem(10000, wcContactInfo, L"", L"", L"", L"", -1, false);
	}

	this->pItemCol->get_Count(&cItems);
	for (int i = 1; i <= cItems; i++) {
        if (FAILED(this->pItemCol->Item(i, 
            reinterpret_cast<IDispatch**>(&pContact))))
            continue;
		
		// grab properties
		pContact->get_FileAs(&bstrFileAs);
		pContact->get_Categories(&bstrCategories);
		pContact->get_Oid(&lOid);

		wcsncpy(wcContactInfo, bstrFileAs, 25);
		wcsncpy(wcCategories, bstrCategories, 25);

		// Don't display groups if showing favorites
        if (bOnlyFavorites) {
			wcGroup[0] = 0;
        }
        else {
            wcGroup[0] = wcContactInfo[0];
            if (wcGroup[0] >= 'a' && wcGroup[0] <= 'z')
                wcGroup[0] += 'A' - 'a';
            wcGroup[1] = 0;
        }

		// actually add this contact to the item list
        bool isFavorite = NULL 
            != wcsstr(wcCategories, this->_settings->favorite_category);

		if (!bOnlyFavorites || isFavorite) {
			this->AddItem(i-1, wcContactInfo, L"", wcCategories, wcGroup, L"", 
                lOid, isFavorite);
		}

		// clean up
	    SysFreeString(bstrFileAs);
	    SysFreeString(bstrCategories);
		pContact->Release();
    }

}

int ListDataPoom::PopulateDetailsFor(int index) {
    int c = 0;
	this->InitDetailData();
    IContact * pContact;
    BSTR buffer;

    if (this->_items[index].ID == 10000) {
	    HRESULT hr = 0;
	    this->polApp->CreateItem(olContactItem, (IDispatch**)&pContact);
	    hr = pContact->Display();
	    pContact->Release();
	    return 0;
    }
	
    LONG oid = this->_items[index].oId;

    // GET CURRENT CONTACT
    if (FAILED(this->polApp->GetItemFromOid(oid, 
        reinterpret_cast<IDispatch**>(&pContact))))
        return 0;

    // GET CONTACT DETAILS
    pContact->get_MobileTelephoneNumber(&buffer);
    if(SysStringLen(buffer) > 0) {
	    this->_actionsList[c].id = c + 1;
	    wcsncpy(this->_actionsList[c].text, buffer, 25);
	    this->_actionsList[c].action = SLA_CALL;
	    this->_actionsList[c].canSms = true;
	    wcsncpy(this->_actionsList[c].label, this->_settings->mobile_string, 25);
	    c++;
    }
    SysFreeString(buffer);

    pContact->get_HomeTelephoneNumber(&buffer);
    if(SysStringLen(buffer) > 0) {
	    this->_actionsList[c].id = c + 1;
	    wcsncpy(this->_actionsList[c].text, buffer, 25);
	    this->_actionsList[c].action = SLA_CALL;
	    wcsncpy(this->_actionsList[c].label, this->_settings->home_string, 25);
	    c++;
    }
    SysFreeString(buffer);
	
    pContact->get_BusinessTelephoneNumber(&buffer);
    if(SysStringLen(buffer) > 0) {
	    this->_actionsList[c].id = c + 1;
	    wcsncpy(this->_actionsList[c].text, buffer, 25);
	    this->_actionsList[c].action = SLA_CALL;
	    this->_actionsList[c].canSms = true;
	    wcscpy(this->_actionsList[c].label, this->_settings->work_string);
	    c++;
    }
    SysFreeString(buffer);
	
    pContact->get_CompanyName(&buffer);
    if(SysStringLen(buffer) > 0) {
	    this->_actionsList[c].id = c + 1;
	    wcsncpy(this->_actionsList[c].text, buffer, 25);
	    this->_actionsList[c].action = SLA_TEXT;
	    wcscpy(this->_actionsList[c].label, this->_settings->company_string);
	    c++;
    }
    SysFreeString(buffer);
	
    pContact->get_Email1Address(&buffer);
    if(SysStringLen(buffer) > 0) {
	    this->_actionsList[c].id = c + 1;
	    wcsncpy(this->_actionsList[c].text, buffer, 25);
	    this->_actionsList[c].action = SLA_EMAIL;
	    wcscpy(this->_actionsList[c].label, this->_settings->email_string);
	    c++;
    }
    SysFreeString(buffer);

    this->_actionsNumber = c;

    // cleanup
    pContact->Release();

    return 1;
}

void ListDataPoom::AddToFavorites(int index) {
    LONG oId = this->_items[index].oId;

    // update the categories in the POOM
    BSTR bstrCategories((BSTR)this->_settings->favorite_category); 
    if (SetPoomCategories(oId, bstrCategories)) {
        // update the "favorite" status in our list data
	    this->_items[index].isFavorite = true;
    }

    // TODO: I don't think I need this: SysFreeString(bstrCategories);
}

void ListDataPoom::RemoveFromFavorites(int index) {
    LONG oId = this->_items[index].oId;

    // update the categories in the POOM
    BSTR bstrCategories(L""); 
    
    if (SetPoomCategories(oId, bstrCategories)) {
        // update the "favorite" status in our list data
	    this->_items[index].isFavorite = false;
    }

    // TODO: I don't think I need this: SysFreeString(bstrCategories);
}

void ListDataPoom::DisplayItem(int index) {
    IContact * pContact;
    this->pItemCol->Item(this->_items[index].ID + 1,
        reinterpret_cast<IDispatch**>(&pContact));
	pContact->Display();
	pContact->Release();
    //should not delete COM objects delete pContact;
}

bool ListDataPoom::InitPoom() {
    // Now, let's get the main outlook application
    if (FAILED(CoCreateInstance(CLSID_Application, NULL, CLSCTX_INPROC_SERVER, 
	    IID_IPOutlookApp, reinterpret_cast<void **>(&this->polApp))))
        return false;
	
    // login to the Pocket Outlook object model
    if(FAILED(this->polApp->Logon(NULL)))
	    // can't login to the app
        return false;

    return true;
}

// **************************************************************************
// Function Name: GetPoomFolder
// 
// Purpose: Gets the default folder for the specified POOM item type
//
// Arguments:
//    IN int nFolder - POOM folder type to return, olFolderTasks,
//		olFolderContacts, or olFolderCalendar in this app
//	  OUT IFolder ** ppFolder - pointer to IFolder interface returned. 
//			Must be released by caller of GetPoomFolder
//
// Return Values:
//    BOOL
//    returns TRUE if the folder interface was retrieved, FALSE otherwise
// 
// Description:  
//  This function simply encapsulates a call on the global Outlook app interface. The 
//	returned pointer is simply passed through this function.
bool ListDataPoom::GetPoomFolder(int nFolder,   IFolder ** ppFolder) {
    return
        SUCCEEDED(this->polApp->GetDefaultFolder(nFolder, ppFolder));
}

// **************************************************************************
// Function Name: GetPoomApp
// 
// Purpose: Gets pointer to POOM application interface
//
// Arguments:
//	  OUT IFolder ** ppFolder - pointer to IPOutlookApp interface returned. 
//			Must be released by caller of GetPoomApp
//
// Return Values:
//    BOOL
//    returns TRUE if the folder interface was retrieved, FALSE otherwise
// 
// Description:  
//  This function simply returns a pointer to the global Outlook app interface 
//	The returned pointer is simply passed through this function after being AddRef'd.
/*bool ListDataPoom::GetPoomApp(IPOutlookApp **ppOutApp) {
    if (this->polApp) {
	    this->polApp->AddRef();
	    *ppOutApp = this->polApp;
	    return true;
    }
    else {
	    return false;
    }
}*/

bool ListDataPoom::SetPoomCategories(LONG oId, BSTR bstrCategories) {
    // Find the desired contact
    IContact * poomContact = NULL;
    if (!this->polApp)
	    return false;

    if (FAILED(this->polApp->GetItemFromOid(oId, 
        reinterpret_cast<IDispatch**>(&poomContact))))
        return false;

    // update the categories in the POOM
    poomContact->put_Categories(bstrCategories);

    // save and clean up
    poomContact->Save();
    poomContact->Release();
    
    return true;
}
