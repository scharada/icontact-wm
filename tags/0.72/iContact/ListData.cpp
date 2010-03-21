#include <stdafx.h>
#include "iContact.h"
#include "ListData.h"

ListData::ListData() {
    // moved this to Populate
    //this->_items = new Data[MAX_LIST_ITEMS];

    this->_actionsList = new DataDetail[MAX_SUBLIST_ITEMS];
    this->_arrayLength = 0;
    this->_subListCurrentAction = -1;
    this->_currentGroup = -1;
    this->_actionsNumber = 0;
    this->_sms = false;
    this->_canFavorite = false;
    this->Init();
}

ListData::ListData(Settings * pSettings) {
    this->_settings = pSettings;
}

ListData::~ListData(void) {
    this->Clear();
    delete [] this->_items;
    delete [] this->_actionsList;
}

Data ListData::GetItem(int index) {
    return this->_items[index];
}

void ListData::GetItemGroup(int index, wchar_t * pszGroup) {
    pszGroup[0] = this->_items[index].wcGroup;
    pszGroup[1] = 0;
}

bool ListData::IsItemNewGroup(int index) {
	if (index == 0)
		return true;

    return this->_items[index - 1].wcGroup != this->_items[index].wcGroup;
}

bool ListData::CanSms() {
    return this->_sms;
}

void ListData::SetSms(bool is) {
    this->_sms = is;
}

int ListData::GetSubListCurrentActionIndex() {
    return this->_subListCurrentAction;
}

void ListData::SetSubListCurrentActionIndex(int index) {
    this->_subListCurrentAction = index;
}

void ListData::SelectPreviousSubListAction() {
    int max = this->_actionsNumber;
        
    // there may be no actions; in this case just don't do anything
    if (!max)
        return;

    int index = (this->_subListCurrentAction - 1) % max;
    int count = 1;

    while (this->_actionsList[index].action == SLA_TEXT && count++ < max)
        index = (index - 1) % max;

    if (count <= max)
        this->_subListCurrentAction = index;
}

void ListData::SelectNextSubListAction() {
    int max = this->_actionsNumber;
    
    // there may be no actions; in this case just don't do anything
    if (!max)
        return;

    int index = (this->_subListCurrentAction + 1) % max;
    int count = 1;

    while (this->_actionsList[index].action == SLA_TEXT && count++ < max)
        index = (index + 1) % max;

    if (count <= max)
        this->_subListCurrentAction = index;
}

DataDetail ListData::GetSubListCurrentAction() {
    return this->_actionsList[this->_subListCurrentAction];
}

DataDetail ListData::GetItemDetail(int index) {
    return this->_actionsList[index];
}

int ListData::GetItemCount() {
    return this->_listCounter;
}

int ListData::GetItemDetailCount() {
    //TODO: optimize this
    for (int i = 0; i < MAX_SUBLIST_ITEMS; i++) {
        if (this->_actionsList[i].action == -1)
            return i;
    }
    return MAX_SUBLIST_ITEMS;
}

void ListData::Init() {
	this->_listCounter = 0;

    for(int i = 0; i < this->_arrayLength; i++) {
	    this->_items[i].szPrimaryText[0] = 0;
        this->_items[i].wcGroup = 0;
	    this->_items[i].ID = -1;
        this->_items[i].nPrimaryTextLength = 0;
		this->_items[i].nSecondaryTextLength = 0;
    }
}

bool ListData::InitDetailData() {
    for(int j = 0; j < MAX_SUBLIST_ITEMS; j++) {
	    this->_actionsList[j].id = -1;
	    this->_actionsList[j].action = -1;
	    this->_actionsList[j].canSms = false;
        this->_actionsList[j].label[0] = 0;
        this->_actionsList[j].text[0] = 0;
    }
    return true;
}

bool ListData::AddItem(int id, wchar_t * tszPrimary, wchar_t * tszSecondary,
	wchar_t wcGroup, wchar_t * tszExtra, LONG oId) {

    int i = this->_listCounter;

    if (i >= this->_arrayLength)
        return false;

    this->_items[i].ID = id;
    this->_items[i].oId = oId;
    wcsncpy(this->_items[i].szPrimaryText, tszPrimary, 24);
    this->_items[i].szPrimaryText[24] = 0;
	wcsncpy(this->_items[i].szSecondaryText, tszSecondary, 9);
    this->_items[i].szSecondaryText[9] = 0;
    this->_items[i].wcGroup = wcGroup;
    wcsncpy(this->_items[i].szExtra, tszExtra, 25);
    this->_items[i].szExtra[24] = 0;
    this->_items[i].nPrimaryTextLength = wcslen(this->_items[i].szPrimaryText);
	this->_items[i].nSecondaryTextLength = wcslen(this->_items[i].szSecondaryText);
	
    this->_listCounter++;
    return true;
}

void ListData::Clear(void) {}
bool ListData::CanFavorite(void) { return this->_canFavorite; }
void ListData::ToggleFavorite(int index) {}
void ListData::DisplayItem(int index) {}
void ListData::Populate(void) {}
int ListData::PopulateDetailsFor(int id) { return 0; }