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
#include "RegistryUtils.h"
#include "FileUtils.h"

CSettings::CSettings(void) {
	TCHAR buffer[MAX_PATH] = {0};
	TCHAR szLanguagePath[MAX_PATH] = {0};

    //////////////////////////////////////////////////
    // Settings
    LoadSetting(this->email_account, REGISTRY_MAXLENGTH, SZ_ICONTACT_REG_KEY,
        INI_EMAIL_ACCOUNT_KEY, INI_EMAIL_ACCOUNT_DEFAULT);

	LoadSetting(buffer, REGISTRY_MAXLENGTH, SZ_ICONTACT_REG_KEY,
        INI_EXIT_ON_MIN_KEY, INI_EXIT_ON_MIN_DEFAULT);
    this->doExitOnMinimize = '1' == buffer[0];

	LoadSetting(buffer, REGISTRY_MAXLENGTH, SZ_ICONTACT_REG_KEY,
        INI_EXIT_ON_ACTION_KEY, INI_EXIT_ON_ACTION_DEFAULT);
    this->doExitOnAction = '1' == buffer[0];

    LoadSetting(buffer, REGISTRY_MAXLENGTH, SZ_ICONTACT_REG_KEY,
        INI_FAST_GFX_KEY, INI_FAST_GFX_DEFAULT);
    this->doFastGraphics = '1' == buffer[0];

	LoadSetting(buffer, REGISTRY_MAXLENGTH, SZ_ICONTACT_REG_KEY,
        INI_FULLSCREEN_KEY, INI_FULLSCREEN_DEFAULT);
    this->doShowFullScreen = '1' == buffer[0];
    
    LoadSetting(buffer, MAX_PATH, SZ_ICONTACT_REG_KEY,
        INI_SKIN_KEY, INI_SKIN_DEFAULT);
	if (buffer[0] == '\\') 
		StringCchCopy(this->skin_path, MAX_PATH, buffer);
	else
	    GetCurDirFilename(this->skin_path, buffer, TEXT("png"));
	// TODO: check if skin exists


    LoadSetting(buffer, MAX_PATH, SZ_ICONTACT_REG_KEY,
        INI_LANGUAGE_KEY, INI_LANGUAGE_DEFAULT);
	if (buffer[0] == '\\') 
		StringCchCopy(szLanguagePath, MAX_PATH, buffer);
	else
	    GetCurDirFilename(szLanguagePath, buffer, TEXT("lng"));

    //////////////////////////////////////////////////
    // Language
    const struct LanguageSetting languageSettings[] = {
		&this->favorites_default, TEXT("favorites"), TEXT("Favorites"),
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

	// Read the file into this->language_data TCHAR array
    HANDLE hCache = CreateFile(szLanguagePath, GENERIC_READ, 0, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hCache, 0, NULL, FILE_BEGIN);

	char cbuffer[2048];
    DWORD dwFileSize = min(2048, GetFileSize(hCache, NULL));
    DWORD dwBytesRead;

    ReadFile(hCache, cbuffer, dwFileSize, &dwBytesRead, NULL);
    ASSERT(dwBytesRead == dwFileSize);

	::mbstowcs(this->language_data, cbuffer, 2048);

	// Then set all the TCHAR * to the appropriate locations in that array
	TCHAR * pStr;
    for (int i = 0; i < ARRAYSIZE(languageSettings); i++) {

		pStr = _tcsstr(this->language_data, languageSettings[i].pszName);

		if (pStr == NULL) {
			*(languageSettings[i].ppSetting) = languageSettings[i].pszDefault;
			continue;
		}

		// we want what's after the equal, not what's before it
        *(languageSettings[i].ppSetting) = _tcsstr(pStr, TEXT("=")) + 1;
    }

	// Then add nulls in the appropriate locations 
	// (replace '=', '\r', ,\n, with 0
	// (the other data in the file is then just junk)
	for (pStr = this->language_data; pStr < this->language_data + 2048; pStr++) {
		if (*pStr == '\r' || *pStr == '\n')
			*pStr = 0;
	}

    //////////////////////////////////////////////////
	// Special handling for "Favorites" category:
	
	// 1. If they've chosen a category specifically, it's in the registry
    LoadSetting(this->favorite_category, REGISTRY_MAXLENGTH, SZ_ICONTACT_REG_KEY,
        INI_FAVORITE_CAT_KEY, NULL);

	// 2. Otherwise, use the default category from the language file
	// (or "Favorites" if it doesn't exist in the language file)
	if (0 == _tcslen(this->favorite_category))
		StringCchCopy(this->favorite_category, REGISTRY_MAXLENGTH,
			this->favorites_default);
}
