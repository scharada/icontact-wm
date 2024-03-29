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

#pragma once

#define SZ_ICONTACT_REG_KEY         TEXT("Software\\Supware.net\\iContact")
#define REGISTRY_MAXLENGTH          32

#define INI_EMAIL_ACCOUNT_KEY       TEXT("EmailAccount")
#define INI_EMAIL_ACCOUNT_DEFAULT   TEXT("")
#define INI_FAVORITE_CAT_KEY        TEXT("FavoriteCategory")
#define INI_FAVORITE_CAT_DEFAULT    TEXT("")
#define INI_EXIT_ON_MIN_KEY         TEXT("ExitOnMinimize")
#define INI_EXIT_ON_MIN_DEFAULT     TEXT("0")
#define INI_EXIT_ON_ACTION_KEY      TEXT("ExitOnAction")
#define INI_EXIT_ON_ACTION_DEFAULT  TEXT("0")
#define INI_FAST_GFX_KEY            TEXT("FastGraphics")
#define INI_FAST_GFX_DEFAULT        TEXT("0")
#define INI_ENABLE_SENSOR_KEY       TEXT("EnableSensor")
#define INI_ENABLE_SENSOR_DEFAULT   TEXT("1")
#define INI_SKIN_KEY                TEXT("Skin")
#define INI_SKIN_DEFAULT            TEXT("default")
#define INI_LANGUAGE_KEY            TEXT("Language")
#define INI_LANGUAGE_DEFAULT        TEXT("english")
#define INI_FULLSCREEN_KEY			TEXT("FullScreen")
#define INI_FULLSCREEN_DEFAULT		TEXT("1")
#define INI_GRAVITY_KEY				TEXT("Gravity")
#define INI_GRAVITY_DEFAULT			TEXT("20")

struct LanguageSetting {
    const TCHAR ** ppSetting;
    TCHAR * pszName;
    TCHAR * pszDefault;
};

class CSettings {
public:
    CSettings(void);

    // POOM settings
    TCHAR favorite_category[REGISTRY_MAXLENGTH];
    TCHAR email_account[REGISTRY_MAXLENGTH];

	// toggles
	bool doExitOnAction;
	bool doExitOnMinimize;
    bool doFastGraphics;
	bool doShowFullScreen;
    bool doEnableSensor;

    // Strings
    TCHAR skin_path[MAX_PATH];
	TCHAR language_data[2048];

	// Physics
	float gravity;

	// These strings are just pointers to within language_data
	const TCHAR * favorites_default;
    const TCHAR * alphabet;
    const TCHAR * mobile_string;
    const TCHAR * home_string;
    const TCHAR * work_string;
    const TCHAR * company_string;
    const TCHAR * car_string;
    const TCHAR * assistant_string;
    const TCHAR * fax_string;
    const TCHAR * pager_string;
    const TCHAR * radio_string;
    const TCHAR * email_string;
    const TCHAR * sms_string;
    const TCHAR * today_string;
    const TCHAR * yesterday_string;
    const TCHAR * sunday_string;
    const TCHAR * monday_string;
    const TCHAR * tuesday_string;
    const TCHAR * wednesday_string;
    const TCHAR * thursday_string;
    const TCHAR * friday_string;
    const TCHAR * saturday_string;
    const TCHAR * older_string;
    const TCHAR * date_string;
    const TCHAR * duration_string;
    const TCHAR * seconds_string;
    const TCHAR * outgoing_string;
    const TCHAR * missed_string;
    const TCHAR * incoming_string;
    const TCHAR * unknown_string;
    const TCHAR * returncall_string;
    const TCHAR * savecontact_string;
    const TCHAR * managecontact_string;
    const TCHAR * editcontact_string;
    const TCHAR * recents_string;
    const TCHAR * allcontacts_string;
    const TCHAR * details_string;
    const TCHAR * categories_string;
	const TCHAR * createshortcut_string;
	const TCHAR * removeshortcut_string;
};
