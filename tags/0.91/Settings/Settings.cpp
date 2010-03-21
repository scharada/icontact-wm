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

// iContact.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Settings.h"
#include "../iContact/CSettings.h"
#include "../iContact/Titlebar.h"
#include "../iContact/Macros.h"
#include "../iContact/PhoneUtils.h"
#include "../iContact/FileUtils.h"
#include "../iContact/Version.h"
// for favorites category
#include <pimstore.h>

// for email account
#include <cemapi.h>
#include <mapiutil.h>
#include <mapidefs.h>

//-----------------------------------------------------------------------------
// Global data
//
HINSTANCE   hInst;                     // Program instance handle

CSettings * pSettings = NULL;
int		    ListHeight = 0;
POINT       ptMouseDown = { -1, -1 };
int         nTouchRadius = 4;

// Graphic
RECT		rScreen = {0};
RECT        rTitlebar = {0};
RECT        rContent = {0};
int         rListHeight = 0;
int         minScroll = 0;
int         maxScroll = 0;
RECT        rTouchZone = {0};


// These deal with an expanded item (to choose from a list, for instance)
int         nExpandedIndex = -1;
int         rExpandedHeight = 0;
int         nQueuedExpandedIndex = -1;

TCHAR       SettingOptions[MAX_OPTIONS][MAX_LOADSTRING];
int         nOptions;

// Pocket Outlook (POOM)
IPOutlookApp2 * polApp = NULL;

// UI Element Sizes. These can't be static because
// of different DPI devices
int         TitlebarHeight = TITLEBAR_HEIGHT;
int         DefaultItemHeight = DEFAULT_ITEM_HEIGHT;
int         DefaultGroupHeight = DEFAULT_GROUP_HEIGHT;
int         HeaderHeight = HEADER_HEIGHT;
int         HeaderClickHeight = HEADER_CLICK_HEIGHT;
int         MenuBarHeight = MENU_BAR_HEIGHT;
int         MenuBarIconWidth = MENU_BAR_ICON_WIDTH;

int         TitlebarFontSize = TITLEBAR_FONT_SIZE;
int	        ItemFontSize = ITEM_FONT_SIZE;
int	        ItemSecondaryFontSize = ITEM_SECONDARY_FONT_SIZE;
int	        KeyboardFontSize = KEYBOARD_FONT_SIZE;
int	        ItemDetailsFontSize = ITEM_DETAILS_FONT_SIZE;
int         ItemDetailsPictureSize = ITEM_DETAILS_PICTURE_SIZE;
int         ItemDetailsPadding = ITEM_DETAILS_PADDING;
int	        GroupItemFontSize = GROUP_ITEM_FONT_SIZE;
int	        ListIndicatorFontSize = LIST_INDICATOR_FONT_SIZE;

int	        ListItemIndent = LIST_ITEM_INDENT;
int	        ListGroupItemIndent = LIST_GROUP_ITEM_INDENT;
int	        ListSeparatorHeight = LIST_SEPARATOR_HEIGHT;

// Fonts
HFONT       TitlebarFont;
HFONT		PrimaryListFont;
HFONT		SecondaryListFont;
HFONT		GroupFont;
HFONT		ItemDetailsFont;
HFONT		ListIndicatorFont;
HFONT		KeyboardFont;

// Screen buffers
HDC         hdcMem = NULL;
HBITMAP		hbmMem = NULL;
HDC         hdcTmp = NULL;
HBITMAP		hbmTmp = NULL;
HDC			hdcSkin = NULL;
HBITMAP		hbmSkin = NULL;
HDC         hdcPage1 = NULL;
HBITMAP     hbmPage1 = NULL;
HDC         hdcPage2 = NULL;
HBITMAP     hbmPage2 = NULL;
HBITMAP     hbmCanvas = NULL;
HBRUSH      hbrCanvas = NULL;

// Scrolling
bool		bDragging = false;
int			Scrolled = 0;
int         ListScrolled = 0;
int			LastX;
int			LastY;
int			tStartTime;
int			tEndTime;
double		Velocity = 0;
int         nKeyRepeatCount = 0;

// Scroll To
DWORD       Scroll_StartTime = 0;
double		Scroll_TimeCounter = 0.0;
int 		Scroll_StartPosition = 0;
int 		Scroll_Change = 0;
double		Scroll_Duration = 0.0;

// Screen Transition
DWORD       dwTransitionStart = 0;
double      dTransitionPct = 0.0;
int         nTransitionDuration = 0;
bool        bTransitioning = false;
TransitionType trTransitionType = ttExpand;

// Popup window
bool        bDisplayingPopup = false;
PopupType   ptPopupType = ptYesNo;

// Keyboard Rows/Columns
TCHAR       alphabet[ALPHABET_MAX_SIZE];
int         nKeyboardLetters = 26;
int         nKeyboardRows = 0;
int         nKeyboardCols = 0;
int		    GroupWidth = 0;     // Keyboard group width
int		    GroupHeight = 0;    // Keyboard group height



// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_PAINT, DoPaintMain,
    WM_DESTROY, DoDestroyMain,
    WM_ACTIVATE, DoActivate,
    WM_SIZE, DoSize,
    WM_LBUTTONDOWN, DoMouseDown,
    WM_MOUSEMOVE, DoMouseMove,
    WM_LBUTTONUP, DoMouseUp,
    WM_TIMER, DoTimer,
    WM_KEYDOWN, DoKeyDown,
    WM_TITLEBAR, DoTitlebarCallback,
};

// Configuration of the settings
struct SingleSetting Settings[] = {
    TEXT("Email Account"), 
    INI_EMAIL_ACCOUNT_KEY,
    INI_EMAIL_ACCOUNT_DEFAULT,
    stList, 
    emailAccountFiller,

    TEXT("Favorite Category"), 
    INI_FAVORITE_CAT_KEY,
    INI_FAVORITE_CAT_DEFAULT,
    stList, 
    favoriteCategoryFiller,

    TEXT("Exit on Minimize"), 
    INI_EXIT_ON_MIN_KEY,
    INI_EXIT_ON_MIN_DEFAULT,
    stOnOff, 
    NULL,

    TEXT("Exit on Action"), 
    INI_EXIT_ON_ACTION_KEY,
    INI_EXIT_ON_ACTION_DEFAULT,
    stOnOff, 
    NULL,

    TEXT("Fast Graphics"), 
    INI_FAST_GFX_KEY,
    INI_FAST_GFX_DEFAULT,
    stOnOff,
    NULL,

    TEXT("Skin"), 
    INI_SKIN_KEY,
    INI_SKIN_DEFAULT,
    stList,
    skinFiller,

    TEXT("Language"), 
    INI_LANGUAGE_KEY,
    INI_LANGUAGE_DEFAULT,
    stList,
    languageFiller,

    TEXT("Full Screen"), 
    INI_FULLSCREEN_KEY,
    INI_FULLSCREEN_DEFAULT,
    stOnOff,
    NULL,
};

TCHAR SettingValues[ARRAYSIZE(Settings)][MAX_LOADSTRING] = {0};

//=============================================================================
//
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    HWND hwndMain;

    // Initialize this instance.
    hwndMain = InitInstance(hInstance, lpCmdLine, nCmdShow);
    if (hwndMain == 0)
        return 0x10;

    // Application message loop
    while (GetMessage (&msg, NULL, 0, 0)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
    // Instance cleanup
    return TermInstance (hInstance, msg.wParam);
}
//-----------------------------------------------------------------------------
// InitInstance - Instance initialization
//
HWND InitInstance (HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow){
    WNDCLASS wc;
    HWND hWnd;

    // Save program instance handle in global variable.
    hInst = hInstance;

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // If Windows Mobile, allow only one instance of the application.
    hWnd = FindWindow (SZ_APP_NAME, NULL);
    if (hWnd) {
        SetForegroundWindow ((HWND)(((DWORD)hWnd) | 0x01));   
        return 0;
    }
#endif

    // Register application main window class.
    wc.style = 0;                             // Window style
    wc.lpfnWndProc = MainWndProc;             // Callback function
    wc.cbClsExtra = 0;                        // Extra class data
    wc.cbWndExtra = 0;                        // Extra window data
    wc.hInstance = hInstance;                 // Owner handle
    wc.hIcon = NULL;                          // Application icon
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);// Default cursor
    wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wc.lpszMenuName =  NULL;                  // Menu name
    wc.lpszClassName = SZ_APP_NAME;         // Window class name

    if (RegisterClass (&wc) == 0) return 0;
    // Create main window.
    hWnd = CreateWindowEx(WS_EX_NODRAG,       // Ex Style
                         SZ_APP_NAME,         // Window class
                         SZ_APP_NAME,         // Window title
                         WS_SYSMENU,          // Style flags
                                              // don't use WS_VISIBLE.
                                              // Why not? It causes an "error report"
                                              // Why? I don't know.
                         CW_USEDEFAULT,       // x position
                         CW_USEDEFAULT,       // y position
                         CW_USEDEFAULT,       // Initial width
                         CW_USEDEFAULT,       // Initial height
                         NULL,                // Parent
                         NULL,                // Menu, must be null
                         hInstance,           // Application instance
                         NULL);               // Pointer to create
                                              // parameters
    // Return fail code if window not created.
    if (!IsWindow (hWnd)) return 0;

    // Initialize COM libraries (for POOM)
    if (FAILED(CoInitializeEx(NULL, 0)))
        return 0;

    // Additional initialization
    _initPoom();

    // Perform DPI adjustments
    HDC hdc = GetDC(hWnd);
    int dpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
    if (dpi > SKIN_DPI) {
        TitlebarHeight = MulDiv(TitlebarHeight, dpi, SKIN_DPI);
        DefaultItemHeight = MulDiv(DefaultItemHeight, dpi, SKIN_DPI);
        DefaultGroupHeight = MulDiv(DefaultGroupHeight, dpi, SKIN_DPI);
        HeaderHeight = MulDiv(HeaderHeight, dpi, SKIN_DPI);
        HeaderClickHeight = MulDiv(HeaderClickHeight, dpi, SKIN_DPI);
        MenuBarHeight = MulDiv(MenuBarHeight, dpi, SKIN_DPI);
        MenuBarIconWidth = MulDiv(MenuBarIconWidth, dpi, SKIN_DPI);

        TitlebarFontSize = MulDiv(TitlebarFontSize, dpi, SKIN_DPI);
        ItemFontSize = MulDiv(ItemFontSize, dpi, SKIN_DPI);
        ItemSecondaryFontSize = MulDiv(ItemSecondaryFontSize, dpi, SKIN_DPI);
        KeyboardFontSize = MulDiv(KeyboardFontSize, dpi, SKIN_DPI);
        ItemDetailsFontSize = MulDiv(ItemDetailsFontSize, dpi, SKIN_DPI);
        ItemDetailsPictureSize = MulDiv(ItemDetailsPictureSize, dpi, SKIN_DPI);
        ItemDetailsPadding = MulDiv(ItemDetailsPadding, dpi, SKIN_DPI);
        GroupItemFontSize = MulDiv(GroupItemFontSize, dpi, SKIN_DPI);
        ListIndicatorFontSize = MulDiv(ListIndicatorFontSize, dpi, SKIN_DPI);

        ListItemIndent = MulDiv(ListItemIndent, dpi, SKIN_DPI);
        ListGroupItemIndent = MulDiv(ListGroupItemIndent, dpi, SKIN_DPI);
        ListSeparatorHeight = MulDiv(ListSeparatorHeight, dpi, SKIN_DPI);
    }

    // Create fonts
    TitlebarFont = BuildFont(TitlebarFontSize, FALSE, FALSE);
	PrimaryListFont = BuildFont(ItemFontSize, FALSE, FALSE);
	SecondaryListFont = BuildFont(ItemSecondaryFontSize, TRUE, FALSE);
	ItemDetailsFont = BuildFont(ItemDetailsFontSize, FALSE, FALSE);
	GroupFont = BuildFont(GroupItemFontSize, TRUE, FALSE);
    ListIndicatorFont = BuildFont(ListIndicatorFontSize, TRUE, FALSE);
	KeyboardFont = BuildFont(KeyboardFontSize, TRUE, FALSE);

    // Create data lists
    pSettings = new CSettings();

    // Initialize titlebar callbacks
	if (pSettings->doShowFullScreen) 
	    InitTitlebar(hWnd);

	LoadSettings();

    InitSurface(hWnd);

    // Standard show and update calls
    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);

    // Find running iContact executable, and kill it!
    HWND hWndiContact = FindWindow (TEXT("iContact"), NULL);
    if (hWndiContact) {
        SendMessage(hWndiContact, WM_CLOSE, NULL, NULL);
    }

    return hWnd;
}

//-----------------------------------------------------------------------------
// TermInstance - Program cleanup
//
int TermInstance (HINSTANCE hInstance, int nDefRC) {

    return nDefRC;
}
//=============================================================================
// Message handling procedures for MainWindow
//

//-----------------------------------------------------------------------------
// MainWndProc - Callback function for application window
//
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT wMsg, WPARAM wParam, 
                              LPARAM lParam) {
    INT i;
    //
    // Search message list to see if we need to handle this
    // message. If in list, call procedure.
    //
    for (i = 0; i < ARRAYSIZE(MainMessages); i++) {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoPaintMain - Process WM_PAINT message for window.
//
LRESULT DoPaintMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;
    
    int rScreenWidth = rScreen.right - rScreen.left;
    int rScreenHeight = rScreen.bottom - rScreen.top;   

    hdc = BeginPaint (hWnd, &ps); 

    // rect is the region that needs to be painted
    rect = ps.rcPaint;

    // draw the screen...
    DrawScreenOn(hdcMem, hdcTmp, rect);

    // Transfer everything to the actual screen
    BitBlt(hdc, rect.left, rect.top, rect.right - rect.left,
        rect.bottom - rect.top, hdcMem, rect.left, rect.top, SRCCOPY);

    EndPaint (hWnd, &ps);

    return 0;
}

//-----------------------------------------------------------------------------
// DoActivate - Process WM_ACTIVATE message for window
//
LRESULT DoActivate (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    DWORD dwState;
    RECT rc;

    if (wParam == WA_CLICKACTIVE || wParam == WA_ACTIVE) {
        // To switch to full screen mode, first hide all of the shell parts.

		if (pSettings->doShowFullScreen) {
			dwState = SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON;

			// resize the main window to the size of the screen.
			SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN), 
				GetSystemMetrics(SM_CYSCREEN));
		}
		else {
			dwState = SHFS_SHOWTASKBAR | SHFS_HIDESIPBUTTON;

			// resize the main window to the size of the work area.
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE);
		}

        SHFullScreen(hWnd, dwState);

        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, 
            rc.bottom-rc.top, TRUE);

        ShowWindow(hWnd, SW_SHOWNORMAL);
    }

    // The window is being deactivated... restore it to non-fullscreen
    else {
        // To switch to normal mode, first show all of the shell parts.
        dwState = (SHFS_SHOWTASKBAR 
            | SHFS_SHOWSTARTICON 
            | SHFS_SHOWSIPBUTTON);

        SHFullScreen(hWnd, dwState);

        // Next resize the main window to the size of the work area.
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE);

        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left,
            rc.bottom-rc.top, TRUE);

		// No need to keep this program around
        DestroyWindow(hWnd);
    }

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoTitlebarCallback - Process WM_TITLEBAR message for window
//
LRESULT DoTitlebarCallback (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {
    RefreshTitlebar(lParam);
    InvalidateRect(hWnd, &rTitlebar, false);
    return 0;
}

//-----------------------------------------------------------------------------
// DoSize - Process WM_SIZE message for window
//
LRESULT DoSize (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

	InitSurface(hWnd);

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoMouseDown - Process WM_LBUTTONDOWN message for window
//
LRESULT DoMouseDown (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

	LastX = ptMouseDown.x = LOWORD(lParam);
	LastY = ptMouseDown.y = HIWORD(lParam);
    tStartTime = ::GetTickCount();
    nTouchRadius = 4;

    if (bDisplayingPopup || bTransitioning) {
        return 0;
    }

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoMouseMove - Process WM_MOUSEMOVE message for window
//
LRESULT DoMouseMove (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    int x = LOWORD(lParam);
	int y = HIWORD(lParam);
    int t = ::GetTickCount();

    if (bDisplayingPopup || bTransitioning) {
        return 0;
    }

    if (ptMouseDown.y < rTitlebar.bottom) {
        return 0;
    }

    if (!bDragging && PtInRect(&rTouchZone, ptMouseDown)) {
        nTouchRadius = max(nTouchRadius, (int)
            sqrt(pow(x - ptMouseDown.x, 2) + pow(y - ptMouseDown.y, 2))
            );
        InvalidateRect(hWnd, &rContent, false);
        UpdateWindow(hWnd);
        return 0;
    }

	if (abs(y - ptMouseDown.y) > SCROLL_THRESHOLD) {
        bDragging = true;
	}

    // SCROLL
    Scrolled = Scrolled - y + LastY;
	LastY = y;
	LastX = x;

    Velocity = (double)(LastY - ptMouseDown.y) / (t - tStartTime);
    tEndTime = t;

    InvalidateRect(hWnd, &rContent, false);

    UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// DoMouseUp - Process WM_LBUTTONUP message for window
//
LRESULT DoMouseUp (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    POINT pt;

    if (bTransitioning)
        return 0;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);

    // They clicked on the popup, no matter what screen
    if (bDisplayingPopup) {
        StartTransition(hWnd, ttContract, CONTRACT_PERIOD);
    }


    // They clicked in the titlebar
    // no matter what the screen type is
	else if (PtInRect(&rTitlebar, ptMouseDown) && PtInRect(&rTitlebar, pt)) {
        DestroyWindow(hWnd);
	}


    // They scrolled the screen up too far
    else if (bDragging && Scrolled < minScroll) {
        bDragging = false;
        ScrollTo(hWnd, minScroll);
    }


    // They scrolled the screen down too far
    else if (bDragging && Scrolled > maxScroll) {
        bDragging = false;
        ScrollTo(hWnd, maxScroll);
    }


    // now we're scrolling
    else if (bDragging) {
        SetTimer(hWnd, IDT_TIMER_SCROLL, REFRESH_RATE, (TIMERPROC) NULL);
        bDragging = false;
        return 0;
    } 

    // This is the normal *click* event
    else {
        int y = pt.y + Scrolled - rContent.top;
        int index = -1;

        // They clicked above the expanded part
        if (y / DefaultItemHeight <= nExpandedIndex) {
            index = y / DefaultItemHeight;
        }

        // They clicked below the expanded part
        else if ((y - rExpandedHeight) / DefaultItemHeight > nExpandedIndex) {
            index = (y - rExpandedHeight) / DefaultItemHeight;
        }

        // They clicked on the expanded part
        if (index == -1) {
            switch (Settings[nExpandedIndex].type) {
                case stList:
                    index = (y - (DefaultItemHeight * nExpandedIndex)) 
                        / DefaultItemHeight - 1;
                    
                    // They clicked "download more skins"
                    if (0 == _tcscmp(SettingOptions[index], 
                        SZ_DOWNLOAD_SKINS)) {
                        OpenURL(URL_DOWNLOAD_SKINS);
                    }
                    
                    // They clicked "download more languages"
                    else if (0 == _tcscmp(SettingOptions[index], 
                        SZ_DOWNLOAD_LANGUAGES)) {
                        OpenURL(URL_DOWNLOAD_LANGUAGES);
                    }
                    
                    // They clicked something starting with "AUTO",
                    // so the proper setting value for this is ""
                    else if (SettingOptions[index] 
                        == _tcsstr(SettingOptions[index], SZ_AUTO)) {
                        SettingValues[nExpandedIndex][0] = '\0';
                    }

                    else {
                        StringCchCopy(SettingValues[nExpandedIndex], 
                            MAX_LOADSTRING, SettingOptions[index]);
                    }

                    StartTransition(hWnd, ttContract, 150);
                    break;
                case stOnOff:
                    // we should never get here
                    break;
            }
        }

        else if (index < ARRAYSIZE(Settings)) {
            if (nExpandedIndex > -1) {
                StartTransition(hWnd, ttContract, 150);
            }

            switch (Settings[index].type) {
                case stList:
                    if (index != nExpandedIndex && nExpandedIndex > -1)
                        nQueuedExpandedIndex = index;
                    if (!bTransitioning)
                        ExpandIt(hWnd, index);
                    break;
                case stOnOff:
                    SettingValues[index][0] 
                        = SettingValues[index][0] == '0' ? '1' : '0';
                    break;
            }
        }

        SaveSettings();

        InvalidateRect(hWnd, &rContent, FALSE);
    }

    UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// DoTimer - Process WM_TIMER message for window
//
LRESULT DoTimer (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
    DWORD t = ::GetTickCount();
    DWORD dt = 0;
    double s = 0.0;

	switch (wParam)	{

        ///// TIMER for scrolling
	    case IDT_TIMER_SCROLL:

            // Time
            dt = t - tEndTime;

            // Velocity
            if (Scrolled < minScroll)
                Velocity = (double)(Scrolled - minScroll) / 2 / dt;
            else if (Scrolled > maxScroll)
                Velocity = (double)(Scrolled - maxScroll) / 2 / dt;
            else {
                double dv = Velocity * FRICTION_COEFF * dt;
                if (fabs(dv) > fabs(Velocity)) 
                    Velocity = 0;
                else 
			        Velocity = Velocity - dv;
            }

            // Displacement
            s = Velocity * dt;
            if (s < 0 && s > -1 && Scrolled < minScroll)
                s = -1;
            else if (s > 0 && s < 1 && Scrolled > maxScroll)
                s = 1;
            
            // We're done scrolling
            if ((int)s == 0) {
                KillTimer(hWnd, IDT_TIMER_SCROLL);
		        Velocity = 0;
            }

            Scrolled = Scrolled - (int)s;
            tEndTime = t;
            InvalidateRect(hWnd, &rContent, false);
		    break;

        ///// TIMER for scroll to
        case IDT_TIMER_SCROLL_TO:
            KillTimer(hWnd, IDT_TIMER_SCROLL);
            Scroll_TimeCounter = (double)(t - Scroll_StartTime);
            if (Scroll_TimeCounter < Scroll_Duration) {
                // Cubic
                double amount = Scroll_Change
                    * (pow(Scroll_TimeCounter/Scroll_Duration - 1, 3) + 1);
                Velocity = (amount - Scroll_StartPosition) / Scroll_TimeCounter;
                Scrolled = Scroll_StartPosition + (int)amount;
            }
            else {
                Velocity = 0;
                KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
                Scrolled = Scroll_Change + Scroll_StartPosition;
            }
            InvalidateRect(hWnd, &rContent, false);
            break;

        case IDT_TIMER_TRANSITION:
            dTransitionPct = (double)(t - dwTransitionStart) 
                / nTransitionDuration;

            if (dTransitionPct >= 1.0) {
                dTransitionPct = 1.0;
                bTransitioning = false;

                KillTimer(hWnd, IDT_TIMER_TRANSITION);

                if (nQueuedExpandedIndex > -1) {
                    int tmp = nQueuedExpandedIndex;
                    nQueuedExpandedIndex = -1;
                    ExpandIt(hWnd, tmp);
                }
                else if (trTransitionType == ttContract) {
                    nExpandedIndex = -1;
                    rExpandedHeight = 0;
                }

                CalculateHeights();
            }
            InvalidateRect(hWnd, &rScreen, false);
            break;
	} 

	UpdateWindow(hWnd);
	return 0;
}

//-----------------------------------------------------------------------------
// DoKeyDown - Process WM_KEYDOWN message for window
//
LRESULT DoKeyDown (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
    int top = 0;
    int bot = 0;
    bool bRepeating = (lParam & (1 << 30)) != 0;

    if (bTransitioning) {
        return 0;
    }

    // TODO: this part

	UpdateWindow(hWnd);

    return 0;
}


//-----------------------------------------------------------------------------
// DoDestroyMain - Process WM_DESTROY message for window
//
LRESULT DoDestroyMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
   
    // Uninitialize the COM classes
    CoUninitialize();

    // Save Settings
    SaveSettings();

	// Clean up titlebar
	if (pSettings->doShowFullScreen)
		DestroyTitlebar();

    // Quit
    PostQuitMessage (0);
    return 0;
}

//-----------------------------------------------------------------------------
// Screen Drawing Functions
//
void DrawScreenOn(HDC hdc, HDC hdcTmp, RECT rClip) {

    RECT rItem;
    rItem.bottom = rContent.top - Scrolled;

    rTouchZone.right = rTouchZone.left = 0;
    rTouchZone.bottom = rTouchZone.top = 0;

    SetBkMode(hdc, TRANSPARENT);
    SetBkMode(hdcTmp, TRANSPARENT);

	// ******* DRAW LIST BACKGROUND
    FillRect(hdc, &rContent, hbrCanvas);

    // "About" at bottom of screen
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
    SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
    ExtTextOut(hdc, rContent.left + 2, rContent.bottom - 2, 
        NULL, NULL, SZ_ABOUT, ABOUT_LENGTH, 0);

    HFONT hfOld = (HFONT)SelectObject(hdc, PrimaryListFont);
    SetTextColor(hdc, pSettings->rgbListItemText);

    // MAIN CONTENT
    for (int i = 0; i < ARRAYSIZE(Settings); i++) {
        SingleSetting ss = Settings[i];
        TCHAR * value = SettingValues[i];

        // calculate rItem
        rItem.top = rItem.bottom;
        rItem.bottom = rItem.top + DefaultItemHeight;
        rItem.left = rContent.left;
        rItem.right = rContent.right;
        FillRect(hdc, &rItem, pSettings->hbrListItemBackground);
        int baseline = rItem.bottom - ((DefaultItemHeight - ItemFontSize) / 2);

        // draw separator
        RECT rSep = rItem;
        rSep.top = rItem.bottom - ListSeparatorHeight;
        FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
      
        // draw caption
        SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
        SetTextColor(hdc, pSettings->rgbListItemText);
        ExtTextOut(hdc, rItem.left + ListItemIndent, baseline, 
            ETO_CLIPPED, &rItem, ss.caption, _tcslen(ss.caption), NULL);

        // draw setting
        switch (ss.type) {
            case stList:
                DrawValue(hdc, rItem, value);
                break;
            case stOnOff:
                DrawOnOff(hdc, rItem, value[0] == '0' ? false : true);
                break;
        }

        // if this item is selected, draw the details
        if (nExpandedIndex == i) {
            rItem.top = rItem.bottom;
            rItem.bottom += trTransitionType == ttExpand
                ? (int)(dTransitionPct * rExpandedHeight)
                : (int)((1.0 - dTransitionPct) * rExpandedHeight);
            rItem.left = ListItemIndent;
            rItem.right -= ListItemIndent;

            RECT rTmp;
            rTmp.left = 0;
            rTmp.top = 0;
            rTmp.right = rItem.right - rItem.left;
            rTmp.bottom = rExpandedHeight;

            if (ss.type == stList) {
                DrawListDetailsOn(hdcTmp, rTmp, value);
            }

            BitBlt(hdc, rItem.left, rItem.top, 
                rItem.right - rItem.left, rItem.bottom - rItem.top, 
                hdcTmp, 0, rExpandedHeight - (rItem.bottom - rItem.top),
                SRCCOPY);
        }
    }

    SelectObject(hdc, hfOld);

    // TITLE BAR
    if (rClip.top < rTitlebar.bottom && pSettings->doShowFullScreen)
	    DrawTitlebarOn(hdc, rTitlebar, hdcSkin, TitlebarFont);

}

void DrawListDetailsOn(HDC hdc, RECT rect, const TCHAR * tszValue) {
    FillRect(hdc, &rect, pSettings->hbrListItemSeparator);

    HFONT hfOld = (HFONT)SelectObject(hdc, PrimaryListFont);
    SetTextColor(hdc, pSettings->rgbListItemText);
    SetTextAlign(hdc, TA_LEFT | TA_TOP);

    for (int j = 0; j < nOptions; j++) {
        ExtTextOut(hdc, rect.left + ListItemIndent * 2, 
            rect.top + ListItemIndent + j * DefaultItemHeight,
            NULL, NULL, SettingOptions[j], 
            _tcslen(SettingOptions[j]), NULL);

        if (0 == _tcscmp(tszValue, SettingOptions[j])
            || _tcslen(tszValue) == 0
            && _tcsstr(SettingOptions[j], SZ_AUTO) == SettingOptions[j]) {
            int radius = ListItemIndent / 2;
            Ellipse(hdc, 
                rect.left + ListItemIndent - radius,
                rect.top + DefaultItemHeight / 2 
                - radius + j * DefaultItemHeight,
                rect.left + ListItemIndent + radius,
                rect.top + DefaultItemHeight / 2 
                + radius + j * DefaultItemHeight);

        }
    }

    SelectObject(hdc, hfOld);
}

//-----------------------------------------------------------------------------
// Utility functions
//

void InitSurface(HWND hWnd) {
    HRESULT hr = S_OK;
	HDC hdc;
	hdc = GetDC(hWnd);

    // Update the RECTs for the individual sections
	GetClientRect(hWnd, &rScreen);
    int nScreenHeight = rScreen.bottom - rScreen.top;
    int nScreenWidth = rScreen.right - rScreen.left;

    // Title bar, with date, carrier, battery, signal strength, etc.
	rTitlebar = rScreen;
	if (pSettings->doShowFullScreen) {
		rTitlebar.bottom = rTitlebar.top + TitlebarHeight;
	}
	else {
		// collapse new titlebar so it is not active
		rTitlebar.bottom = rTitlebar.top;
	}

    // From the header to the bottom of the screen
    rContent = rScreen;
    rContent.top = rTitlebar.bottom;
    rListHeight = rContent.bottom - rContent.top;

    // Initialize the DCs and Bitmaps
    if (hdcMem)
        CBR(DeleteDC(hdcMem));
    hdcMem = CreateCompatibleDC(hdc);
    if (hbmMem)
        CBR(DeleteObject(hbmMem));
	hbmMem = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcMem, hbmMem);

    // Temporary canvas
    if (hdcTmp)
        CBR(DeleteDC(hdcTmp));
    hdcTmp = CreateCompatibleDC(hdc);
    if (hbmTmp)
        CBR(DeleteObject(hbmTmp));
	hbmTmp = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcTmp, hbmTmp);


    // Calculate skin filename
    if (!hbmSkin) {
	    TCHAR szSkinFileName[MAX_PATH];
        GetCurDirFilename(szSkinFileName, pSettings->skin_name, TEXT("png"));

        hbmSkin = SHLoadImageFile(szSkinFileName);

	    // Load skin
	    hdcSkin = CreateCompatibleDC(hdc);
        SelectObject(hdcSkin, hbmSkin);

        // find the size of the skin
        BITMAP bmSkin;
        GetObject(hbmSkin, sizeof(bmSkin), &bmSkin);
        int nSkinWidth = bmSkin.bmWidth;
        int nSkinHeight = bmSkin.bmHeight;
        int dpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
        int nCanvasWidth = MulDiv(nSkinWidth, dpi, SKIN_DPI);
        int nCanvasHeight = MulDiv(SKIN_CANVAS_HEIGHT, dpi, SKIN_DPI);

        // Create canvas brush by copying the appropriate region
        // from the skin into a bitmap
        HDC hdcCanvas = CreateCompatibleDC(hdc);
        hbmCanvas = CreateCompatibleBitmap(hdc, nCanvasWidth, nCanvasHeight);
        SelectObject(hdcCanvas, hbmCanvas);

        // for compatibility with older skins...
        if (nSkinHeight != SKIN_CANVAS_Y_OFFSET + SKIN_CANVAS_HEIGHT) {
            RECT rCanvas = {0, 0, nCanvasWidth, nCanvasHeight};
            FillRect(hdcCanvas, &rCanvas, (HBRUSH)GetStockObject(BLACK_BRUSH));
        }

        else {
            StretchBlt(hdcCanvas, 0, 0, nCanvasWidth, nCanvasHeight, 
                hdcSkin, 0, SKIN_CANVAS_Y_OFFSET, nSkinWidth, SKIN_CANVAS_HEIGHT,
                SRCCOPY);
        }

        hbrCanvas = CreatePatternBrush(hbmCanvas);
        CBR(DeleteDC(hdcCanvas));
    }

    CalculateHeights();

    CBR(ReleaseDC(hWnd, hdc));

Error:
    ASSERT(SUCCEEDED(hr));
}

void CalculateHeights() {
    ListHeight = ARRAYSIZE(Settings) * DefaultItemHeight;
    if (nExpandedIndex >= 0) {
        int tmp = rExpandedHeight;
        if (bTransitioning) {
            tmp = (int)(tmp * dTransitionPct);
        }
        ListHeight += tmp;
    }
    
    minScroll = 0;
    maxScroll = ListHeight > rListHeight ? ListHeight - rListHeight : 0;
}

void ScrollTo(HWND hWnd, int position, int duration) {
    if (position < minScroll)
        position = minScroll;
    if (position > maxScroll)
        position = maxScroll;

    Scroll_StartPosition = Scrolled;
	Scroll_Change = position - Scroll_StartPosition;
	Scroll_Duration = duration;
	Scroll_StartTime = ::GetTickCount();
	Scroll_TimeCounter = 0;
	SetTimer(hWnd, IDT_TIMER_SCROLL_TO, REFRESH_RATE, NULL);
}

void StartTransition(HWND hWnd, TransitionType tr, int duration) {
    bTransitioning = true;
    nTransitionDuration = duration;
    dTransitionPct = 0.0;
    trTransitionType = tr;
    dwTransitionStart = ::GetTickCount();

    InvalidateRect(hWnd, &rScreen, FALSE);

	SetTimer(hWnd, IDT_TIMER_TRANSITION, REFRESH_RATE, NULL);
}

void ExpandIt(HWND hWnd, int index) {
    nOptions = Settings[index].filler(SettingOptions, SettingValues[index]);
    nExpandedIndex = index;
    rExpandedHeight = DefaultItemHeight * nOptions;
    StartTransition(hWnd, ttExpand);
}

HFONT BuildFont(int iFontSize, BOOL bBold, BOOL bItalic) {
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	lf.lfHeight = iFontSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = bBold ? 600 : 500;
	lf.lfItalic = bItalic;
	lf.lfUnderline = false;
	lf.lfStrikeOut = false;
	lf.lfCharSet = EASTEUROPE_CHARSET;
	lf.lfOutPrecision = OUT_RASTER_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcsncpy (lf.lfFaceName, TEXT("Tahoma"), LF_FACESIZE);
	//lf.lfFaceName[LF_FACESIZE-1] = L'\0';  // Ensure null termination
	return CreateFontIndirect(&lf);
}

//-----------------------------------------------------------------------------
// Load/Save/Default functions
//

void LoadSettings() {
    for (int i = 0; i < ARRAYSIZE(Settings); i++) {
        StringCchCopy(SettingValues[i], MAX_LOADSTRING, 
            pSettings->ini.GetValue(MAIN_SECTION, 
            Settings[i].key, Settings[i].def));
    }
}

void SaveSettings() {
    for (int i = 0; i < ARRAYSIZE(Settings); i++) {
        pSettings->ini.SetValue(MAIN_SECTION, 
            Settings[i].key, SettingValues[i]);
    }
    pSettings->Save();
}

void LoadDefaults() {
    for (int i = 0; i < ARRAYSIZE(Settings); i++) {
        StringCchCopy(SettingValues[i], MAX_LOADSTRING, 
            Settings[i].def);
    }
}

//-----------------------------------------------------------------------------
// Settings handlers
//

// generic setting handlers
void DrawOnOff(HDC hdc, RECT rect, bool value) {
    SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
    SetTextColor(hdc, pSettings->rgbListItemText);

    int baseline = rect.bottom - (DefaultItemHeight - ItemSecondaryFontSize) / 2;

    const TCHAR * szOnOff = value ? SZ_ON : SZ_OFF;

    HFONT hfOld = (HFONT)SelectObject(hdc, SecondaryListFont);

    ExtTextOut(hdc, rect.right - ListItemIndent, baseline, ETO_CLIPPED, 
        &rect, szOnOff, _tcslen(szOnOff), NULL);

    SelectObject(hdc, hfOld);
}

void DrawValue(HDC hdc, RECT rect, const TCHAR * value) {
    HFONT hfOld = (HFONT)SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
    SetTextColor(hdc, pSettings->rgbListItemText);
    int baseline = rect.bottom - (DefaultItemHeight - ItemSecondaryFontSize) / 2;
    
    if (0 == _tcslen(value)) {
        ExtTextOut(hdc, rect.right - ListItemIndent, baseline, ETO_CLIPPED, 
            &rect, SZ_AUTO, 4, NULL);
    }
    else {
        ExtTextOut(hdc, rect.right - ListItemIndent, baseline, ETO_CLIPPED, 
            &rect, value, _tcslen(value), NULL);
    }
    SelectObject(hdc, hfOld);
}

// http://blogs.msdn.com/windowsmobile/archive/2007/03/21/getting-started-with-mapi.aspx
int emailAccountFiller(TCHAR options[MAX_OPTIONS][MAX_LOADSTRING], 
                       TCHAR * value) {

    HRESULT hr;
    IMAPITable * ptbl;
    IMAPISession * pSession;
    SRowSet *prowset = NULL;
    SPropValue * pval = NULL;
    SizedSPropTagArray (1, spta) = { 1, PR_DISPLAY_NAME };
    int index = 0;
   
    StringCchCopy(options[index], MAX_LOADSTRING, SZ_AUTO);
    StringCchCat(options[index], MAX_LOADSTRING, TEXT(" ["));
    GetDefaultEmailAccount(options[index] + 6);
    StringCchCat(options[index++], MAX_LOADSTRING, TEXT("]"));

    // Log onto MAPI
    hr = MAPILogonEx(0, NULL, NULL, 0, static_cast<LPMAPISESSION *>(&pSession));
    CHR(hr); // CHR will goto Error if FAILED(hr)
   
    // Get the table of accounts
    hr = pSession->GetMsgStoresTable(0, &ptbl);
    CHR(hr);
   
    // set the columns of the table we will query
    hr = ptbl->SetColumns ((SPropTagArray *) &spta, 0);
    CHR(hr);
   
    while (TRUE) {
        // Free the previous row
        FreeProws(prowset);
        prowset = NULL;
 
        hr = ptbl->QueryRows (1, 0, &prowset);
        if ((hr != S_OK) || (prowset == NULL) || (prowset->cRows == 0)) {
            break;
        }
 
        ASSERT(prowset->aRow[0].cValues == spta.cValues);
        pval = prowset->aRow[0].lpProps;
 
        ASSERT(pval[0].ulPropTag == PR_DISPLAY_NAME);
 
        if (0 != _tcscmp(pval[0].Value.lpszW, TEXT("SMS"))) {
            StringCchCopy(options[index++], MAX_LOADSTRING, pval[0].Value.lpszW);
        }
        //MessageBox(NULL, pval[0].Value.lpszW, TEXT("Message Store"), MB_OK);
    }
 
    pSession->Logoff(0, 0, 0);
 
Error:
    RELEASE_OBJ(ptbl);
    RELEASE_OBJ(pSession);
    FreeProws(prowset);
    return index;
}

// http://msdn.microsoft.com/en-us/library/bb446087.aspx
int favoriteCategoryFiller(TCHAR options[MAX_OPTIONS][MAX_LOADSTRING],
                           TCHAR * value) {

    HRESULT           hr = E_FAIL;
    IFolder    * pFolder = NULL;
    IItem * pFolderIItem = NULL;
    CEPROPVAL    * pVals = NULL;
    ULONG       cbBuffer = 0;
    int            index = 0;

    CEPROPID rgPropID = PIMPR_FOLDER_CATEGORIES;

    _initPoom();
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
        hr = StringCchCopyN(options[index++], MAX_LOADSTRING, start, comma - start);
        CHR(hr);
        start = comma + 2;
    }

Error:
    // Free resources.
    HeapFree(hHeap, 0, pVals);
    RELEASE_OBJ(pFolderIItem);
    RELEASE_OBJ(pFolder);

    return index;
}

HRESULT _initPoom() {
    HRESULT hr;

    if (polApp == NULL) {
        hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER,
                              __uuidof(IPOutlookApp2), (LPVOID*) &polApp);
        CHR(hr);

        hr = polApp->Logon(NULL);
        CHR(hr);
    }

    hr = S_OK;

Error:
    if (FAILED(hr)) {
        // If we failed to log on, don't keep the object around
        RELEASE_OBJ(polApp);
    }
    return hr;
}

int skinFiller(TCHAR options[MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value) {
    TCHAR szIniPath[MAX_PATH];
    WIN32_FIND_DATA findFileData;
    HANDLE handle;
    int index = 0;
    BOOL found = false;

    // Get program file path
    ::GetModuleFileName(NULL, szIniPath, MAX_PATH);

    TCHAR * pstr = _tcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';
    StringCchCat(szIniPath, MAX_PATH, TEXT("*.skn"));

    handle = FindFirstFile(szIniPath, &findFileData);
    if (handle != INVALID_HANDLE_VALUE) {
        do {
            StringCchCopy(options[index], MAX_LOADSTRING, findFileData.cFileName);

            // remove ".lng"
            options[index][_tcslen(options[index])-4] = '\0';

            index++;
            found = FindNextFile(handle, &findFileData);
        } while (found);

        FindClose(handle);
    }

    // add a cool download button
    StringCchCopy(options[index++], MAX_LOADSTRING, SZ_DOWNLOAD_SKINS);

    return index;
}

int languageFiller(TCHAR options[MAX_OPTIONS][MAX_LOADSTRING], TCHAR * value) {
    TCHAR szIniPath[MAX_PATH];
    WIN32_FIND_DATA findFileData;
    HANDLE handle;
    int index = 0;
    BOOL found = false;

    // Get program file path
    ::GetModuleFileName(NULL, szIniPath, MAX_PATH);

    TCHAR * pstr = _tcsrchr(szIniPath, '\\');
	if (pstr) *(++pstr) = '\0';
    StringCchCat(szIniPath, MAX_PATH, TEXT("*.lng"));

    handle = FindFirstFile(szIniPath, &findFileData);
    if (handle != INVALID_HANDLE_VALUE) {
        do {
            StringCchCopy(options[index], MAX_LOADSTRING, findFileData.cFileName);

            // remove ".lng"
            options[index][_tcslen(options[index])-4] = '\0';

            index++;
            found = FindNextFile(handle, &findFileData);
        } while (found);

        FindClose(handle);
    }

    // add a cool download button
    StringCchCopy(options[index++], MAX_LOADSTRING, SZ_DOWNLOAD_LANGUAGES);

    return index;
}
