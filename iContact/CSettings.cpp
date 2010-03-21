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

#include <stdafx.h>
#include <string.h>

#include "CSettings.h"
#include "Macros.h"

CSettings::CSettings(void) {
    this->ini = CSimpleIniW(false, false, false);
    this->iniLanguage = CSimpleIniW(false, false, false);
    this->iniSkin = CSimpleIniW(false, false, false);

    TCHAR szLanguagePath[MAX_PATH];
    TCHAR szSkinPath[MAX_PATH];

    // Get program file path
	GetModuleFileName(NULL, this->szIniPath, MAX_PATH);

    TCHAR * pstr = _tcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';

    StringCchCopy(szLanguagePath, MAX_PATH, this->szIniPath);
    StringCchCopy(szSkinPath, MAX_PATH, this->szIniPath);

    StringCchCat(this->szIniPath, MAX_PATH, TEXT("settings.ini"));

    //////////////////////////////////////////////////
    // Settings
	this->ini.LoadFile(szIniPath);

    this->email_account = ini.GetValue(
        MAIN_SECTION, INI_EMAIL_ACCOUNT_KEY, INI_EMAIL_ACCOUNT_DEFAULT);

    this->favorite_category = ini.GetValue(
        MAIN_SECTION, INI_FAVORITE_CAT_KEY, INI_FAVORITE_CAT_DEFAULT);

	this->doExitOnMinimize = 0 == wmemcmp(ini.GetValue(
		MAIN_SECTION, INI_EXIT_ON_MIN_KEY, INI_EXIT_ON_MIN_DEFAULT), 
        TEXT("1"), 1);

	this->doExitOnAction = 0 == wmemcmp(ini.GetValue(
		MAIN_SECTION, INI_EXIT_ON_ACTION_KEY, INI_EXIT_ON_ACTION_DEFAULT), 
        TEXT("1"), 1);

    this->doFastGraphics = 0 == wmemcmp(ini.GetValue(
        MAIN_SECTION, INI_FAST_GFX_KEY, INI_FAST_GFX_DEFAULT), 
        TEXT("1"), 1);

    this->skin_name = ini.GetValue(
        MAIN_SECTION, INI_SKIN_KEY, INI_SKIN_DEFAULT);
    StringCchCat(szSkinPath, MAX_PATH, this->skin_name);
    StringCchCat(szSkinPath, MAX_PATH, TEXT(".skn"));
    
    StringCchCat(szLanguagePath, MAX_PATH, ini.GetValue(
        MAIN_SECTION, INI_LANGUAGE_KEY, INI_LANGUAGE_DEFAULT));
    StringCchCat(szLanguagePath, MAX_PATH, TEXT(".lng"));

    //////////////////////////////////////////////////
    // Language
	this->iniLanguage.LoadFile(szLanguagePath);

    const struct LanguageSetting languageSettings[] = {
        &this->alphabet, TEXT("alphabet"), TEXT(""),
        &this->mobile_string, TEXT("mobile"), TEXT("Mobile"),
        &this->home_string, TEXT("home"), TEXT("Home"),
        &this->work_string, TEXT("work"), TEXT("Work"),
        &this->company_string, TEXT("company"), TEXT("Company"),
        &this->car_string, TEXT("car"), TEXT("Car"),
        &this->assistant_string, TEXT("assistant"), TEXT("Assistant"),
        &this->fax_string, TEXT("fax"), TEXT("Fax"),
        &this->pager_string, TEXT("pager"), TEXT("Pager"),
        &this->radio_string, TEXT("radio"), TEXT("Radio"),
        &this->email_string, TEXT("email"), TEXT("E-mail"),
        &this->sms_string, TEXT("sms"), TEXT("SMS"),
        &this->today_string, TEXT("today"), TEXT("Today"),
        &this->yesterday_string, TEXT("yesterday"), TEXT("Yesterday"),
        &this->sunday_string, TEXT("sunday"), TEXT("Sunday"),
        &this->monday_string, TEXT("monday"), TEXT("Monday"),
        &this->tuesday_string, TEXT("tuesday"), TEXT("Tuesday"),
        &this->wednesday_string, TEXT("wednesday"), TEXT("Wednesday"),
        &this->thursday_string, TEXT("thursday"), TEXT("Thursday"),
        &this->friday_string, TEXT("friday"), TEXT("Friday"),
        &this->saturday_string, TEXT("saturday"), TEXT("Saturday"),
        &this->older_string, TEXT("older"), TEXT("Older"),
        &this->date_string, TEXT("date"), TEXT("Date"),
        &this->duration_string, TEXT("duration"), TEXT("Duration"),
        &this->seconds_string, TEXT("seconds"), TEXT("Seconds"),
        &this->outgoing_string, TEXT("outgoing"), TEXT("Outgoing Call"),
        &this->missed_string, TEXT("missed"), TEXT("Missed Call"),
        &this->incoming_string, TEXT("incoming"), TEXT("Incoming Call"),
        &this->unknown_string, TEXT("unknown"), TEXT("Unknown"),
        &this->returncall_string, TEXT("returncall"), TEXT("Return Call"),
        &this->savecontact_string, TEXT("savecontact"), TEXT("Save Contact"),
        &this->managecontact_string, TEXT("managecontact"), TEXT("Manage Contact"),
        &this->editcontact_string, TEXT("editcontact"), TEXT("Edit Contact"),
        &this->recents_string, TEXT("recents"), TEXT("Recents"),
        &this->allcontacts_string, TEXT("allcontacts"), TEXT("All Contacts"),
        &this->details_string, TEXT("details"), TEXT("Details"),
        &this->categories_string, TEXT("categories"), TEXT("Categories"),
    };

    for (int i = 0; i < ARRAYSIZE(languageSettings); i++) {
        *(languageSettings[i].ppSetting) = this->iniLanguage.GetValue(
            LANGUAGE_SECTION, languageSettings[i].pszName, 
            languageSettings[i].pszDefault);
    }

    //////////////////////////////////////////////////
    // Skin
	this->iniSkin.LoadFile(szSkinPath);

    // Colors, Brushes & Fonts
    this->rgbHeader = this->initColor(
        SKIN_SECTION, TEXT("HeaderText"), TEXT("ffffff"));

    this->rgbHeaderLoading = this->initColor(
        SKIN_SECTION, TEXT("HeaderLoadingText"), TEXT("7f7f7f"));

	this->hbrListGroupBackground = CreateSolidBrush(this->initColor(
		SKIN_SECTION, TEXT("ListGroupBackground"), TEXT("000000")));

	this->rgbListGroupText = this->initColor(
		SKIN_SECTION, TEXT("ListGroupText"), TEXT("ffffff"));

    this->hbrListItemBackground = CreateSolidBrush(this->initColor(
		SKIN_SECTION, TEXT("ListItemBackground"), TEXT("1e1e1e")));

	this->rgbListItemText = this->initColor(
		SKIN_SECTION, TEXT("ListItemText"), TEXT("dcdcdc"));
    this->hbrListItemForeground = CreateSolidBrush(this->rgbListItemText);
    this->hpenListItemForeground = CreatePen(PS_SOLID, 1, this->rgbListItemText);

    this->rgbListItemMissedText = this->initColor(
		SKIN_SECTION, TEXT("ListItemMissedText"), TEXT("ff0000"));

    this->rgbListItemFavoriteText = this->initColor(
		SKIN_SECTION, TEXT("ListItemFavoriteText"), TEXT("ffff00"));

    this->rgbListItemSelectedBackground1 = this->initColor(
        SKIN_SECTION, TEXT("ListItemSelectedBackground1"), TEXT("4b5ab5"));

    this->rgbListItemSelectedBackground2 = this->initColor(
        SKIN_SECTION, TEXT("ListItemSelectedBackground2"), TEXT("162793"));

    this->hbrListItemSelectedBackground = CreateSolidBrush(this->initColor(
        SKIN_SECTION, TEXT("ListItemSelectedBackground1"), TEXT("4b5ab5")));

	this->rgbListItemSelectedText = this->initColor(
		SKIN_SECTION, TEXT("ListItemSelectedText"), TEXT("e6e6e6"));

    // If the selected shadow doesn't exist in the file,
    // make it the same as the selected text 
    // (and then it won't get displayed later)
    const TCHAR * tszListItemSelectedShadow = iniSkin.GetValue(
        SKIN_SECTION, TEXT("ListItemSelectedShadow"), NULL);
    this->rgbListItemSelectedShadow = 
        tszListItemSelectedShadow == NULL
        ? this->rgbListItemSelectedText
        : parseColor(tszListItemSelectedShadow, TEXT("505050"));

	this->hbrListItemSeparator = CreateSolidBrush(this->initColor(
		SKIN_SECTION, TEXT("ListItemSeparator"), TEXT("323232")));
    
    this->rgbDetailMainText = this->initColor(
		SKIN_SECTION, TEXT("DetailMainText"), TEXT("e6e6e6"));
    
    this->rgbDetailMainShadow = this->initColor(
		SKIN_SECTION, TEXT("DetailMainShadow"), TEXT("505050"));

    this->hbrDetailItemSeparator = CreateSolidBrush(this->initColor(
        SKIN_SECTION, TEXT("DetailItemSeparator"), TEXT("999999")));

    this->rgbKeyboardText = this->initColor(
		SKIN_SECTION, TEXT("KeyboardText"), TEXT("dcdcdc"));

    this->hbrKeyboardBackground = CreateSolidBrush(this->initColor(
        SKIN_SECTION, TEXT("KeyboardBackground"), TEXT("464646")));

    this->hpenKeyboardGrid = CreatePen(PS_SOLID, 1, this->initColor(
		SKIN_SECTION, TEXT("KeyboardGrid"), TEXT("646464")));
}

CSettings::~CSettings(void) {
    // Deallocate all memory used by the ini object
    this->ini.Reset();
    this->iniSkin.Reset();
    this->iniLanguage.Reset();
}

void CSettings::Save() {
    this->ini.SaveFile(this->szIniPath);
}

COLORREF CSettings::initColor(TCHAR * wp_section, TCHAR * wp_key,
                        TCHAR * wp_default_hex) {

    const TCHAR * wpColor = this->iniSkin.GetValue(
        wp_section, wp_key, wp_default_hex);

    return parseColor(wpColor, wp_default_hex);
}

COLORREF parseColor(const TCHAR * color_hex, const TCHAR * default_hex) {
    const TCHAR * source = _tcslen(color_hex) == 6 
        ? color_hex : default_hex;

    TCHAR red[3];
    TCHAR green[3];
    TCHAR blue[3];

	StringCchCopy(red, 3, source);
	StringCchCopy(green, 3, source + 2);
	StringCchCopy(blue, 3, source + 4);

	int r = _tcstol(red, NULL, 16);
	int g = _tcstol(green, NULL, 16);
	int b = _tcstol(blue, NULL, 16);

	return RGB(r, g, b);
}