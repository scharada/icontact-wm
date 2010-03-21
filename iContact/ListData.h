#pragma once

#include "Settings.h"

#define MAX_LIST_ITEMS 2000
#define MAX_SUBLIST_ITEMS 10
#define MAX_GROUP_ITEMS 30

#define SLA_TEXT		00
#define SLA_CALL		10
#define SLA_SMS			20
#define SLA_EMAIL		30

/// List DATA
struct Data {
	int		    ID;
	LONG    	oId;
	wchar_t     szPrimaryText[25];
	wchar_t		szSecondaryText[10];
    wchar_t     wcGroup;
    wchar_t     szExtra[25];
	int		    nPrimaryTextLength;
	int			nSecondaryTextLength;
    bool        isFavorite;
};

struct DataDetail {
	int		    id;
	wchar_t     text[25];
	wchar_t	    label[25];
	int		    action;
	bool	    canSms;
};

class ListData {
protected:
    Settings * _settings;

    Data * _items;
    DataDetail * _actionsList; //TODO: change the name of this

    int _arrayLength;
    int _listCounter;
    int _subListCurrentAction;
    int _actionsNumber;
    int _currentGroup;
    bool _sms;
    bool _canFavorite;

public:

    ListData();
    ListData(Settings * pSettings);
    ~ListData(void);
    
    virtual void Init();
    virtual void Clear();
    virtual void Populate();
    
    Data GetItem(int index);
    virtual void GetItemGroup(int index, wchar_t * pszGroup);

    bool CanSms(void);
    void SetSms(bool is);
    bool CanFavorite(void);

    void Select(int index);

    virtual void ToggleFavorite(int index);

    int GetItemCount(void);

    bool AddItem(int id, wchar_t * tszPrimary, wchar_t * tszSecondary,
		wchar_t tszGroup, wchar_t * tszExtra, LONG oId);

    virtual void DisplayItem(int index);

    virtual int PopulateDetailsFor(int id);
    bool InitDetailData(void);
    int GetItemDetailCount(void);
    int GetSubListCurrentActionIndex(void);//TODO: change the name of this
    void SetSubListCurrentActionIndex(int index);//TODO: change the name of this
    void SelectPreviousSubListAction(void);//TODO: change the name of this
    void SelectNextSubListAction(void);//TODO: change the name of this
    DataDetail GetSubListCurrentAction(void); //TODO: change the name of this
    DataDetail GetItemDetail(int index);
    bool IsItemNewGroup(int index);
};
