#include "stdafx.h"
#include <initguid.h>

#include "ListDataPoom.h"


ListDataPoom::ListDataPoom(Settings * pSettings) {
    this->_settings = pSettings;
    this->_bOnlyFavorites = false;
    this->_canFavorite = true;

    this->InitPoom();

    this->polApp->GetDefaultFolder(olFolderContacts, &this->pCurrFldr);
    this->pCurrFldr->get_Items(&this->pItemCol);
    this->Populate();
}

ListDataPoom::ListDataPoom(Settings * pSettings, bool bOnlyFavorites) {
    this->_settings = pSettings;
	this->_bOnlyFavorites = bOnlyFavorites;
    this->_canFavorite = false;

    this->InitPoom();

    // Get the contacts folder.
    this->polApp->GetDefaultFolder(olFolderContacts, &this->pCurrFldr);

    // Get the contacts Items collection.
    this->pCurrFldr->get_Items(&this->pItemCol);

    if (this->_bOnlyFavorites) {
        // Restrict the collection to Contacts in our favorite category
        //TODO: change "Favorites" to setting
        this->pItemCol->Restrict(TEXT("[Categories] = \"Favorites\""), &this->pItemCol);
    }

    this->Populate();
}

void ListDataPoom::Clear(void) {
    if (this->pCurrFldr) {
        this->pCurrFldr->Release();
    }
    if (this->pItemCol) {
        this->pItemCol->Release();
    }
    if (this->polApp) {
	    this->polApp->Logoff();
	    this->polApp->Release();
    }
}

void ListDataPoom::Populate() {
	BSTR bstrFileAs;
	LONG lOid;
	wchar_t wcContactInfo[25];
    wchar_t wcGroup;

	// grpBuf is the "Group" or first letter of the name 
	// that this contact belongs to
	wchar_t grpBuf[1] = L"";

	IContact * pContact = NULL;
	int cItems = 0;
	bool bOnlyFavorites = this->_bOnlyFavorites;

	this->pItemCol->get_Count(&cItems);

    this->_arrayLength = cItems;
    
    if (!bOnlyFavorites) 
        this->_arrayLength++; // +1 for "Add new contact"

    this->_items = new Data[this->_arrayLength];

	// first item in the list is "Add new contact"
	if (!bOnlyFavorites) {
		wcsncpy(wcContactInfo, this->_settings->add_new_contact_string, 25);
		this->AddItem(ADD_NEW_CONTACT_ID, wcContactInfo, L"", 0, L"", -1);
	}

	for (int i = 1; i <= cItems; i++) {
        if (FAILED(this->pItemCol->Item(i, 
            reinterpret_cast<IDispatch**>(&pContact))))
            continue;

	    // grab properties
	    pContact->get_FileAs(&bstrFileAs);
	    pContact->get_Oid(&lOid);

	    wcsncpy(wcContactInfo, bstrFileAs, 25);

	    // Don't display groups if showing favorites
        if (bOnlyFavorites) {
		    wcGroup = 0;
        }
        else {
            wcGroup = wcContactInfo[0];
            if (wcGroup >= 'a' && wcGroup <= 'z')
                wcGroup += 'A' - 'a';
        }

		this->AddItem(i, wcContactInfo, L"", wcGroup, L"", lOid);

		// clean up
	    SysFreeString(bstrFileAs);
		pContact->Release();
    }

}

int ListDataPoom::PopulateDetailsFor(int index) {
    int c = 0;
	this->InitDetailData();
    IContact * pContact;
    BSTR buffer;
    BSTR bstrCategories;
    int id = this->_items[index].ID;

    if (id == ADD_NEW_CONTACT_ID) {
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

    // Determine for reals if this is a favorite (it doesn't get initialized until here)
    pContact->get_Categories(&bstrCategories);
    this->_items[index].isFavorite = NULL != wcsstr(bstrCategories, 
        this->_settings->favorite_category);
    SysFreeString(bstrCategories);

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

    pContact->get_Home2TelephoneNumber(&buffer);
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
	
    pContact->get_Business2TelephoneNumber(&buffer);
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
	
    pContact->get_Email2Address(&buffer);
    if(SysStringLen(buffer) > 0) {
	    this->_actionsList[c].id = c + 1;
	    wcsncpy(this->_actionsList[c].text, buffer, 25);
	    this->_actionsList[c].action = SLA_EMAIL;
	    wcscpy(this->_actionsList[c].label, this->_settings->email_string);
	    c++;
    }
    SysFreeString(buffer);
	
    pContact->get_Email3Address(&buffer);
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

void ListDataPoom::ToggleFavorite(int index) {
    if (!this->polApp)
	    return;

    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;
    HRESULT hr = NULL;

    LPCWSTR lpCategory = this->_settings->favorite_category;
    int id = this->_items[index].ID;

    hr = this->pItemCol->Item(id, &pDisp);
    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);

    // update the categories in the POOM
    if (this->_items[index].isFavorite)
        hr = pItem->RemoveCategory(lpCategory);
    else
        hr = pItem->AddCategory(lpCategory);

    if (SUCCEEDED(hr))
        this->_items[index].isFavorite = !this->_items[index].isFavorite;

    // save and clean up
    pItem->Save();
    pItem->Release();
}

void ListDataPoom::DisplayItem(int index) {
    IDispatch * pDisp = NULL;
    IItem * pItem = NULL;

    int id = this->_items[index].ID;

    HRESULT hr = this->pItemCol->Item(id, &pDisp);
    hr = pDisp->QueryInterface(IID_IItem, (LPVOID*)&pItem);
	hr = pItem->Edit(NULL);

	pItem->Release();
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
