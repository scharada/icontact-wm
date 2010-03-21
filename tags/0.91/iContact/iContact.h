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
#include "resourceppc.h"

//----------------------------------------------------------------------
// Generic defines and data types
//

#define SCROLL_THRESHOLD            12
#define MAX_LOADSTRING              100

//----------------------------------------------------------------------
// Registry defines and data types
//

#define SZ_IDIALER_REG_KEY          TEXT("Software\\Supware.net\\iDialer")
#define SERVICE_NUM                 TEXT("service")
#define SERVICE_TITLE_FORMAT        TEXT("service%dtitle")
#define SERVICE_TYPE_FORMAT         TEXT("service%dtype")

//----------------------------------------------------------------------
// Window Message defines and data types
//

#define WM_SETTINGS_TAINTED         WM_APP + 2
#define WM_RECENTS_TAINTED          WM_APP + 3

#define SETTINGS_TAINTED_IDIALER    0x01

#define CMD_GOTO_FAVORITES          0x01
#define CMD_GOTO_RECENTS            0x02
#define CMD_GOTO_CONTACTS           0x03
#define CMD_GOTO_DIALER             0x04
#define CMD_GOTO_SEARCH             0x05
#define CMD_SWITCH_TAB              0x10
#define CMD_RELOAD                  0x11
#define CMD_REFRESH                 0x12
#define CMD_CLICK_ITEM              0x13
#define CMD_ALT_CLICK_ITEM          0x14
#define CMD_BACK                    0x15
#define CMD_FORWARD                 0x16
#define CMD_ADD                     0x17
#define CMD_FAVORITE                0x18
#define CMD_CHANGE_SERVICE          0x30
#define CMD_GREEN_BUTTON            0x100
#define CMD_RED_BUTTON              0x101

//----------------------------------------------------------------------
// Timer defines and data types
//

#define IDT_TIMER_SCROLL			10010
#define IDT_TIMER_SCROLL_TO			10012
#define IDT_TIMER_TRANSITION        10015

#define SCROLL_TO_PERIOD            300
#define EXPAND_DETAILS_PERIOD       300
#define TRANSITION_PERIOD           300
#define EXPAND_KEYBOARD_PERIOD      300
#define SHRINK_KEYBOARD_PERIOD      200
#define ALPHABET_MAX_SIZE           128

#define LIST_DISPLAY_LIST			10
#define LIST_DISPLAY_DETAILS		20
#define LIST_DISPLAY_KEYBOARD		30
#define LIST_DISPLAY_ADDFAVORITES	40

#define	REFRESH_RATE                11
#define FRICTION_COEFF              0.001

//----------------------------------------------------------------------
// Skin defines and data types
//

#define SKIN_HEADER_HEIGHT          24
#define SKIN_HEADER_Y_OFFSET        16
#define SKIN_MENU_BAR_HEIGHT        40
#define SKIN_MENU_BAR_Y_OFFSET      40
#define SKIN_MENU_BAR_SEL_Y_OFFSET  80
#define SKIN_MENU_BAR_ICON_WIDTH    48
#define SKIN_CANVAS_HEIGHT          4
#define SKIN_CANVAS_Y_OFFSET        120
#define SKIN_WIDTH                  240

// These sizes are for screens of 96 DPI only
// For 192 DPI, these will be adjusted
#define SKIN_DPI                 96

//----------------------------------------------------------------------
// Graphics defines and data types
//

#define TITLEBAR_HEIGHT             16

#define DEFAULT_ITEM_HEIGHT			36
#define DEFAULT_GROUP_HEIGHT        17
#define HEADER_HEIGHT               24
#define HEADER_CLICK_HEIGHT         40
#define MENU_BAR_HEIGHT             40
#define MENU_BAR_ICON_WIDTH         48

#define TITLEBAR_FONT_SIZE          13
#define	ITEM_FONT_SIZE              20
#define	ITEM_SECONDARY_FONT_SIZE    12
#define	KEYBOARD_FONT_SIZE          40
#define	ITEM_DETAILS_FONT_SIZE      12
#define ITEM_DETAILS_PICTURE_SIZE   64
#define ITEM_DETAILS_PADDING        10
#define	GROUP_ITEM_FONT_SIZE        13
#define	LIST_INDICATOR_FONT_SIZE    50

#define	LIST_ITEM_INDENT            8
#define	LIST_GROUP_ITEM_INDENT      14
#define	LIST_SEPARATOR_HEIGHT       1


//----------------------------------------------------------------------
// Function prototypes
//
HWND InitInstance(HINSTANCE, LPWSTR, int);
int TermInstance(HINSTANCE, int);

// Window procedures
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

// Message handlers
LRESULT DoPaintMain(HWND, UINT, WPARAM, LPARAM);
LRESULT DoActivate(HWND, UINT, WPARAM, LPARAM);
LRESULT DoTitlebarCallback (HWND, UINT, WPARAM, LPARAM);
LRESULT DoSettingsTaintedCallback (HWND, UINT, WPARAM, LPARAM);
LRESULT DoRecentsTaintedCallback (HWND, UINT, WPARAM, LPARAM);
LRESULT DoPoomTaintedCallback (HWND, UINT, WPARAM, LPARAM);
LRESULT DoSize(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseMove(HWND, UINT, WPARAM, LPARAM);
LRESULT DoMouseUp(HWND, UINT, WPARAM, LPARAM);
LRESULT DoTimer(HWND, UINT, WPARAM, LPARAM);
LRESULT DoKeyDown(HWND, UINT, WPARAM, LPARAM);
LRESULT DoCommand(HWND, UINT, WPARAM, LPARAM);
LRESULT DoDestroyMain(HWND, UINT, WPARAM, LPARAM);

void InitSurface(HWND);

void DrawScreenOn(HDC, RECT, HDC, int);
void DrawContentOn(HDC, RECT, HDC, int);
void DrawMenubarOn(HDC);
void DrawGroupHeaderOn(HDC, DataItem, RECT);
void DrawItemBackgroundOn(HDC hdc, DataItemType diType, RECT rect);
void DrawItemDetailsOn(HDC, DataItem, int);
void DrawItemHoverOn(HDC, DataItem, RECT);
void DrawItemOn(HDC, DataItem, RECT);
void DrawKeyboardOn(HDC, RECT);
void DrawHeaderOn(HDC, RECT, HDC);

void CalculateHeights();
int GetPixelToItem(int);
void ScrollBar(int);
void ScrollTo (HWND, int, int = SCROLL_TO_PERIOD);
void StartTransition(HWND, TransitionType, int = TRANSITION_PERIOD);
void ParseCommandLine(HWND, LPTSTR);
void CalculateClickRegion(POINT p);

void GetIDialerServiceName();
void NextIDialerService();
bool HasMultipleIDialerServices();