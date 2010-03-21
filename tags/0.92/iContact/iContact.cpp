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
#include "iContact.h"
#include "Skin.h"
#include "ListData.h"
#include "ListDataPoom.h"
#include "ListDataCallLog.h"
#include "GraphicFunctions.h"
#include "PhoneUtils.h"
#include "FileUtils.h"
#include "RegistryUtils.h"
#include "Sensor.h"
#include "Titlebar.h"
#include "Version.h"

#include "regext.h"
#include "snapi.h"

//#define DEBUG_GRAPHICS_SPEED

//-----------------------------------------------------------------------------
// Global data
//
HINSTANCE   hInst;                     // Program instance handle

CSettings * pSettings = NULL;

int         nCurrentTab = 2;
int         nHighlightedTab = 2;
bool        isRecentsTainted = false;
bool        isPoomTainted = false;
bool        isFavoritesTainted = false;

TCHAR       szWindowTitle[64] = {0};
bool        hasiDialerServices = false;

#define HR_NOTIFY_COUNT 2
HREGNOTIFY  hrNotify[HR_NOTIFY_COUNT] = {0};

// Information about which screen you're on
HistoryItem History[3];
int         depth = 0;

int		    ListHeight = 0;
int         AverageItemHeight = DEFAULT_ITEM_HEIGHT;
int         StartPosition[2000];
int         GroupPosition[ALPHABET_MAX_SIZE];
POINT       ptMouseDown = { -1, -1 };

// Graphic
RECT		rScreen = {0};
int         nScreenHeight = {0};
RECT        rTitlebar = {0};
RECT        rServiceTitle = {0};
RECT        rHeader = {0};
RECT		rMenubar = {0};
RECT		rList = {0};
RECT        rContent = {0};
RECT        rClickRegion = {0};
int         rListHeight = 0;
int         minScrolled = 0;
int         maxScrolled = 0;

// UI Element multiplier. 
int         vga = 1; // 1 = qvga, 2 = vga

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
HDC         hdcCanvas = NULL;
HBITMAP     hbmCanvas = NULL;

// Scrolling
bool        bMouseDown = false;
bool		bDragging = false;
bool		bScrolling = false;
int			LastX;
int			LastY;
int			tStartTime;
int			tEndTime;
double		Velocity = 0;
int         nKeyRepeatCount = 0;

// G Sensor Scrolling
bool bGScrolling = false;
UINT iInitialAngle = 0;


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
TransitionType trTransitionType = ttSlideLeft;
bool        bBackOnDeactivate = false;

// Popup window
bool        bDisplayingPopup = false;
PopupType   ptPopupType = ptKeyboard;

// Keyboard Rows/Columns
TCHAR       alphabet[ALPHABET_MAX_SIZE];
int         nKeyboardLetters = 26;
int         nKeyboardRows = 0;
int         nKeyboardCols = 0;
int		    GroupWidth = 0;     // Keyboard group width
int		    GroupHeight = 0;    // Keyboard group height



// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_PAINT,        DoPaintMain,
    WM_DESTROY,      DoDestroyMain,
    WM_ACTIVATE,     DoActivate,
    WM_SIZE,         DoSize,
    WM_LBUTTONDOWN,  DoMouseDown,
    WM_MOUSEMOVE,    DoMouseMove,
    WM_LBUTTONUP,    DoMouseUp,
    WM_TIMER,        DoTimer,
    WM_KEYDOWN,      DoKeyDown,
    WM_COMMAND,      DoCommand,
    WM_TITLEBAR,     DoTitlebarCallback,
    WM_SETTINGS_TAINTED, DoSettingsTaintedCallback,
    WM_RECENTS_TAINTED, DoRecentsTaintedCallback,
	WM_HTC_GESTURE,	DoGestureCallback,
	WM_HTC_TOUCH, DoTouchCallback,
    PIM_ITEM_CREATED_LOCAL, DoPoomTaintedCallback,
    PIM_ITEM_DELETED_LOCAL, DoPoomTaintedCallback,
    PIM_ITEM_CHANGED_LOCAL, DoPoomTaintedCallback,
    PIM_ITEM_CREATED_REMOTE, DoPoomTaintedCallback,
    PIM_ITEM_DELETED_REMOTE, DoPoomTaintedCallback,
    PIM_ITEM_CHANGED_REMOTE, DoPoomTaintedCallback,
};

// TCHAR * filename: filename of cache data file (NULL means don't cache)
// int parent: index of "parent" or "go back"
// bool hasMenus: whether the menus should be displayed
// HRESULT fnPopulate(HANDLE fh): function reference for (re)populate
// HRESULT fnGetGroup(DataItem * data, TCHAR * buffer, int cchDest)
// HRESULT fnClick(DataItem * data, POINT pt, int * newScreen): function reference for click
// HRESULT fnAdd(void): function reference for add
// HRESULT fnToggleFavorite(DataItem * data): function reference for toggle favorite
// HRESULT fnGetHBitmap(DataItem * data, HBITMAP * phBitmap, UINT * puWidth, UINT * puHeight): function reference for get bitmap

const struct ScreenDefinition Screens[] = {
    // cached, favorites
    TEXT("favorites.dat"),
    3,
    true,
    PoomPopulate,
    PoomGetTitle,
    NULL,
    PoomClick,
    NULL,
    NULL,
    NULL,

    // cached, recents
    TEXT("recents.dat"),
    -1,
    true,
    RecentsPopulate,
    RecentsGetTitle,
    RecentsGetGroup,
    RecentsClick,
    NULL,
    NULL,
    NULL,

    // cached, "All Contacts"
    TEXT("allcontacts.dat"),
    3,
    true,
    PoomPopulate,
    PoomGetTitle,
    PoomGetGroup,
    PoomClick,
    PoomAddItem,
    NULL,
    NULL,

    // uncached, categories
    NULL,
    -1,
    true,
    PoomCategoriesPopulate,
    PoomCategoriesGetTitle,
    NULL,
    PoomCategoriesClick,
    NULL,
    NULL, 
    NULL,

    // uncached, poom item details
    NULL,
    2,
    false,
    PoomDetailsPopulate,
    PoomDetailsGetTitle,
    NULL,
    PoomDetailsClick,
    NULL,
    PoomDetailsToggleFavorite,
    PoomDetailsLoadBitmap,

    // uncached, call log item details
    NULL,
    1,
    false,
    RecentDetailsPopulate,
    RecentDetailsGetTitle,
    NULL,
    RecentDetailsClick,
    NULL,
    NULL,
    NULL,

    // uncached, list of contacts in a category
    NULL,
    3,
    true,
    PoomPopulate,
    PoomGetTitle,
    PoomGetGroup,
    PoomClick,
    PoomAddItem,
    NULL,
    NULL,
};

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
        ParseCommandLine(hWnd, lpCmdLine);
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
    if (!IsWindow (hWnd))
		return 0;

    // Initialize COM libraries (for POOM)
    if (FAILED(CoInitializeEx(NULL, 0)))
        return 0;

    // Perform DPI adjustments
    HDC hdc = GetDC(hWnd);
	int res = min(::GetDeviceCaps(hdc, HORZRES), ::GetDeviceCaps(hdc, VERTRES));
    vga = MulDiv(vga, res, DEFAULT_SCREEN_WIDTH);

    // Get the window title (possibly, from iDialer)
    GetIDialerServiceName();
    hasiDialerServices = HasMultipleIDialerServices();

    // Initialize Callbacks for detecting tainted data
    HRESULT hr = RegistryNotifyWindow(
        HKEY_CURRENT_USER, SZ_IDIALER_REG_KEY, SERVICE_NUM, 
        hWnd, WM_SETTINGS_TAINTED, SETTINGS_TAINTED_IDIALER,
        NULL, &hrNotify[0]);

    hr = RegistryNotifyWindow(
        SN_PHONEACTIVECALLCOUNT_ROOT,
        SN_PHONEACTIVECALLCOUNT_PATH,
        SN_PHONEACTIVECALLCOUNT_VALUE, 
        hWnd, WM_RECENTS_TAINTED, NULL,
        NULL, &hrNotify[1]);

    // Create fonts
    TitlebarFont = BuildFont(TITLEBAR_FONT_SIZE * vga, FALSE, FALSE);
	PrimaryListFont = BuildFont(ITEM_FONT_SIZE * vga, FALSE, FALSE);
	SecondaryListFont = BuildFont(ITEM_SECONDARY_FONT_SIZE * vga, TRUE, FALSE);
	ItemDetailsFont = BuildFont(ITEM_DETAILS_FONT_SIZE * vga, FALSE, FALSE);
	GroupFont = BuildFont(GROUP_ITEM_FONT_SIZE * vga, TRUE, FALSE);
    ListIndicatorFont = BuildFont(LIST_INDICATOR_FONT_SIZE * vga, TRUE, FALSE);
	KeyboardFont = BuildFont(KEYBOARD_FONT_SIZE * vga, TRUE, FALSE);

    // Load User Settings
    pSettings = new CSettings();

    // Initialize titlebar callbacks
	if (pSettings->doShowFullScreen)
		InitTitlebar(hWnd);
	
	// Initialize sensors
	SensorGestureInit(hWnd);
	SensorPollingInit();

    // Initialize the screen and bitmaps
    InitSurface(hWnd);

    // Start out on "all contacts"
    PostMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, 2);

    // Standard show and update calls
    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);

    ParseCommandLine(hWnd, lpCmdLine);

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
            return MainMessages[i].Fxn(hWnd, wMsg, wParam, lParam);
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

    if (bTransitioning) {
        RECT rContent = rScreen;

        if (trTransitionType == ttSlideRight 
            || trTransitionType == ttSlideLeft) {

            rContent.top = rTitlebar.bottom;
            int rHeight = rContent.bottom - rContent.top;
            int width1 = (int)(rScreenWidth * dTransitionPct);
            int width2 = rScreenWidth - width1;

            if (trTransitionType == ttSlideLeft) {
                BitBlt(hdcMem, rContent.left, rContent.top, width2, rHeight, 
                    hdcPage1, width1, rContent.top, SRCCOPY);

                BitBlt(hdcMem, width2, rContent.top, width1, rHeight,
                    hdcPage2, rContent.left, rContent.top, SRCCOPY);
            }
            else {
                BitBlt(hdcMem, rContent.left, rContent.top, width1, rHeight, 
                    hdcPage2, width2, rContent.top, SRCCOPY);

                BitBlt(hdcMem, width1, rContent.top, width2, rHeight,
                    hdcPage1, rContent.left, rContent.top, SRCCOPY);
            }

            BitBlt(hdcMem, rTitlebar.left, rTitlebar.top, 
                rScreenWidth, rTitlebar.bottom - rTitlebar.top,
                hdcPage1, rTitlebar.left, rTitlebar.top, SRCCOPY);
        }

        else if (trTransitionType == ttKeyboardExpand 
            || trTransitionType == ttKeyboardShrink) {

            double cubic = pow(dTransitionPct - 1, 3) + 1;
            if (trTransitionType == ttKeyboardShrink)
                cubic = 1 - cubic;

            int nKbWidth = (int)(rScreenWidth * cubic);
            int nKbHeight = (int)(rScreenHeight * cubic);

            BitBlt(hdcMem, rScreen.left, rScreen.top, 
                rScreenWidth, rScreenHeight, 
                hdcPage1, rScreen.left, rScreen.top, SRCCOPY);

            // for faster rendering, don't stretch or alpha-ize the keyboard
            if (pSettings->doFastGraphics) {
                BitBlt(hdcMem, rScreen.right - nKbWidth, 
                    rScreen.bottom - nKbHeight, nKbWidth, nKbHeight,
                    hdcPage2, 0, 0, SRCCOPY);
            }
            else {
                BltAlpha(hdcMem, rScreen.right - nKbWidth, 
                    rScreen.bottom - nKbHeight, nKbWidth, nKbHeight,
                    hdcPage2, 0, 0, rScreenWidth, rScreenHeight, 220);
            }
        }

        if (dTransitionPct == 1.0) {
            bTransitioning = false;

            if (trTransitionType == ttSlideRight) {
                UnselectItem();
            }
            bDisplayingPopup = trTransitionType == ttKeyboardExpand;

            KillTimer(hWnd, IDT_TIMER_TRANSITION);
            
            InvalidateRect(hWnd, &rScreen, FALSE);
        }

    }

    // not transitioning
    else {
        DrawScreenOn(hdcMem, rect, hdcTmp, History[depth].scrolled);

        if (bDisplayingPopup) {
            switch (ptPopupType) {
                case ptKeyboard:
	                // draw the keyboard
	                DrawKeyboardOn(hdcTmp, rScreen);
                    BltAlpha(hdcMem, rScreen.left, rScreen.top,
                        rScreenWidth, rScreenHeight, hdcTmp, 0, 0, 
                        rScreenWidth, rScreenHeight, 220);
                    break;
            }
        }
    }

    // Highlight the region being clicked right now
    if (GetCurrentItemIndex() > -1 || (bMouseDown && !bDragging 
        && (
            PtInRect(&rList, ptMouseDown)
            || PtInRect(&rHeader, ptMouseDown)
            || PtInRect(&rServiceTitle, ptMouseDown) && hasiDialerServices
            || PtInRect(&rMenubar, ptMouseDown) 
                && !Screens[History[depth].screen].hasMenus
            )
        )) {

        DrawRect(hdcTmp, &rClickRegion, GetSkinRGB(SKIN_COLOR_HIGHLIGHT));

        BltAlpha(hdcMem, rClickRegion.left, rClickRegion.top, 
            rClickRegion.right - rClickRegion.left, 
            rClickRegion.bottom - rClickRegion.top,
            hdcTmp, rClickRegion.left, rClickRegion.top,
            rClickRegion.right - rClickRegion.left,
            rClickRegion.bottom - rClickRegion.top,
            128);
    }

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
        // do not enable SHFS_HIDESTARTICON; it breaks the volume control on Diamond
		
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

        nHighlightedTab = nCurrentTab;
        bMouseDown = false;

        ShowWindow(hWnd, SW_SHOWNORMAL);
    }

    // The window is being deactivated and another program 
    // is being run... so minimize
    // lParam == NULL when the window being activated is not this app.
    else if (lParam == NULL) {
        if (pSettings->doExitOnMinimize) {
            DestroyWindow(hWnd);
        }
        else if (bBackOnDeactivate) {
            PostMessage(hWnd, WM_COMMAND, CMD_BACK, 0);
            bBackOnDeactivate = false;
        }
    }

    // The window is being deactivated 
    // (maybe for "manage contact", or "edit contact")... 
    // restore it to non-fullscreen
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
    UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// DoSettingsTaintedCallback - Process WM_SETTINGS_TAINTED message for window
//
LRESULT DoSettingsTaintedCallback (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {

    if (lParam & SETTINGS_TAINTED_IDIALER) {
        GetIDialerServiceName();
        InvalidateRect(hWnd, &rTitlebar, false);
    }

    UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// DoRecentsTaintedCallback - Process WM_RECENTS_TAINTED message for window
//
LRESULT DoRecentsTaintedCallback (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {

    isRecentsTainted = true;

    int screen = History[depth].screen;

    if (hWnd == GetForegroundWindow() && (
        screen == 1 || screen == 5)) {
        PostMessage(hWnd, WM_COMMAND, CMD_RELOAD, NULL);
    }

    UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// DoPoomTaintedCallback - Process PIM_ITEM_*_* message for window
//
LRESULT DoPoomTaintedCallback (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {

    // wParam = Item OID, lParam = Database OID.

    isPoomTainted = true;
    isFavoritesTainted = true;

    int screen = History[depth].screen;

    if (hWnd == GetForegroundWindow() && (
           screen == 0 || screen == 2 || screen == 3 
        || screen == 4 || screen == 6
        )) {
        PostMessage(hWnd, WM_COMMAND, CMD_RELOAD, NULL);
    }

    UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// DoGestureCallback - Process WM_HTC_GESTURE message for window.
//
LRESULT DoGestureCallback (HWND hWnd, UINT wMsg, WPARAM wParam,
						   LPARAM lParam) {

	switch (wParam & ROTATION_MASK)	{
		case ROTATION_CLOCKWISE:
			PostMessage(hWnd, WM_KEYDOWN, VK_DOWN, NULL);
			break;
		case ROTATION_COUNTER:
			PostMessage(hWnd, WM_KEYDOWN, VK_UP, NULL);
			break;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// DoTouchCallback - Process WM_HTC_TOUCH message for window.
//
LRESULT DoTouchCallback (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
    HTCTOUCH_WPARAM touchW;
	memcpy(&touchW, &wParam, sizeof HTCTOUCH_WPARAM);

    //HTCTOUCH_LPARAM touchL;	// No need for this to check where or if the panel is being touched
	//memcpy(&touchL, &lParam, sizeof HTCTOUCH_LPARAM);
    
	if(touchW.Up == 0) {

		// If it's touched in the central region
		if(touchW.Where == 81||touchW.Where == 17) {
			SENSORDATA sd;
			SensorGesturePoll(&sd);

			// Set the initial angle
			iInitialAngle = sd.AngleY;					

			// Start default timer speed
			SetTimer(hWnd, IDT_TIMER_SCROLL, REFRESH_RATE, (TIMERPROC)NULL);

			// We're G-Scrolling now!
			bGScrolling = true;							
		}
	}
	else {
		// We're no longer scrolling :(
		bGScrolling = false;							
	}
		

	
	return 0;
}


//-----------------------------------------------------------------------------
// DoSize - Process WM_SIZE message for window
//
LRESULT DoSize (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
    
    switch (wParam) {
        case SIZE_RESTORED:
        case SIZE_MAXIMIZED:
        case SIZE_MAXSHOW:
	        InitSurface(hWnd);
            break;
        case SIZE_MINIMIZED:
            if (pSettings->doExitOnMinimize)
                DestroyWindow(hWnd);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DoMouseDown - Process WM_LBUTTONDOWN message for window
//

LRESULT DoMouseDown (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

	LastX = ptMouseDown.x = LOWORD(lParam);
	LastY = ptMouseDown.y = HIWORD(lParam);
    tStartTime = ::GetTickCount();
    bMouseDown = true;

    // Touching the list stops the scrolling
    if (bScrolling
        && (
            PtInRect(&rList, ptMouseDown)
            || PtInRect(&rMenubar, ptMouseDown) 
            && !Screens[History[depth].screen].hasMenus
        )
    ) {
        KillTimer(hWnd, IDT_TIMER_SCROLL);
        KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
        rClickRegion.top = -1;
        rClickRegion.bottom = -1;

        double v = Velocity;
        Velocity = 0;

        // Try to detect if they're just touching the list to stop it.
        // Maybe it's scrolling slowly enough that they want to click.
        if (v < -0.1 || v > 0.1)
            return 0;

        bScrolling = false;
    }

    if (bDisplayingPopup || bTransitioning) {
        return 0;
    }

    // invalidate the old click region too, in case it needs to be un-highlighted
    InvalidateRect(hWnd, &rClickRegion, FALSE);

    CalculateClickRegion(ptMouseDown);
    InvalidateRect(hWnd, &rClickRegion, FALSE);

    // virtual scroll bar
    if (LastX > rList.right * 8 / 10 
        && LastY >= rHeader.bottom 
        && LastY < rMenubar.top
        && depth == 1) {

        bScrolling = true;
        ScrollBar(LastY);
        InvalidateRect(hWnd, &rList, FALSE);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DoMouseMove - Process WM_MOUSEMOVE message for window
//
LRESULT DoMouseMove (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
    int t = ::GetTickCount();

    if (bDisplayingPopup || bTransitioning) {
    }

    else if (ptMouseDown.y >= rMenubar.top 
        && Screens[History[depth].screen].hasMenus) {
        if (y > rMenubar.top) {
            ptMouseDown.x = x;
            ptMouseDown.y = y;
            InvalidateRect(hWnd, &rMenubar, false);
        }
    }

    else if (ptMouseDown.y < rHeader.bottom) {
    }

    // "back" button in header bar
    else if (ptMouseDown.y < rHeader.top + HEADER_CLICK_HEIGHT * vga 
        && ptMouseDown.x <= HEADER_CLICK_HEIGHT * vga
        && depth > 0
        ) {
    }

    // "+" button in header bar, or * in detail view
    else if (ptMouseDown.y < rHeader.top + HEADER_CLICK_HEIGHT * vga 
        && ptMouseDown.x >= rList.right - HEADER_CLICK_HEIGHT * vga 
        && (
            Screens[History[depth].screen].fnAdd != NULL
            || Screens[History[depth].screen].fnToggleFavorite != NULL
        ) ) {

    }

	else if (bScrolling && Velocity == 20.0) {
        InvalidateRect(hWnd, &rClickRegion, false);
        rClickRegion.top = -1;
        rClickRegion.bottom = -1;
        rClickRegion.left = -1;
        rClickRegion.right = -1;
        ScrollBar(y);
        InvalidateRect(hWnd, &rList, false);
        UpdateWindow(hWnd);
	}

	else if (bDragging 
        || abs(y - ptMouseDown.y) > SCROLL_THRESHOLD * vga) {

        if (!bDragging) {
            UnselectItem();
            bDragging = true;
        }

        if (bScrolling) 
            bScrolling = false;

        // SCROLL
        History[depth].scrolled += LastY - y;
	    LastY = y;
	    LastX = x;

        Velocity = (double)(LastY - ptMouseDown.y) / (t - tStartTime);
        tEndTime = t;

        InvalidateRect(hWnd, 
            Screens[History[depth].screen].hasMenus ? &rList : &rContent, 
            FALSE);

        UpdateWindow(hWnd);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DoMouseUp - Process WM_LBUTTONUP message for window
//
LRESULT DoMouseUp (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    POINT pt = {0};

    // This will trigger if the list was scrolling quickly, but
    // the user clicked to stop it.
    if (bScrolling
        && (
            PtInRect(&rList, ptMouseDown)
            || PtInRect(&rMenubar, ptMouseDown) 
            && !Screens[History[depth].screen].hasMenus
        )
    ) {
        bScrolling = false;
        InvalidateRect(hWnd, &rContent, FALSE);
        return 0;
    }

    if (bTransitioning)
        return 0;

    if (!bMouseDown)
        return 0;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
    bMouseDown = false;
    InvalidateRect(hWnd, &rClickRegion, FALSE);

    // They clicked on the popup, no matter what screen
    if (bDisplayingPopup) {
        StartTransition(hWnd, ttKeyboardShrink, SHRINK_KEYBOARD_PERIOD);

        int keyboardIndex = (pt.y / GroupHeight) 
            * (rScreen.right - rScreen.left) / GroupWidth 
            + (pt.x - rScreen.left) / GroupWidth;
        if (keyboardIndex < nKeyboardLetters) {
            UnselectItem();
            ScrollTo(hWnd, GroupPosition[keyboardIndex]);
        }
    }

    // They clicked in the bottom menus
    else if (Screens[History[depth].screen].hasMenus 
        && PtInRect(&rMenubar, ptMouseDown) && PtInRect(&rMenubar, pt)) {
        int tab = pt.x * 5 / (rMenubar.right - rMenubar.left);

        nHighlightedTab = tab;

        if (tab == nCurrentTab && History[depth].screen == tab) {
            PostMessage(hWnd, WM_COMMAND, CMD_RELOAD, NULL);
        }
        else {
            PostMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, tab);
        }
    }


    // They scrolled the screen up too far
    else if (bDragging && History[depth].scrolled < minScrolled) {
        bDragging = false;
        ScrollTo(hWnd, minScrolled);
    }


    // They scrolled the screen down too far
    else if (bDragging && History[depth].scrolled > maxScrolled) {
        bDragging = false;
        ScrollTo(hWnd, maxScrolled);
    }


    // now we're scrolling
    else if (bDragging) {
        SetTimer(hWnd, IDT_TIMER_SCROLL, 
            REFRESH_RATE, (TIMERPROC) NULL);
        bScrolling = true;
        bDragging = false;
        return 0;
    } 

    // "back" button in header bar
    else if (depth > 0 && pt.y >= rHeader.top 
        && pt.y < rHeader.top + HEADER_CLICK_HEIGHT * vga 
        && pt.x <= HEADER_CLICK_HEIGHT * vga
        && Screens[History[depth].screen].parent >= 0) {

        PostMessage(hWnd, WM_COMMAND, CMD_BACK, EXPAND_DETAILS_PERIOD);
    }

    // "+" button in header bar
    else if (pt.y >= rHeader.top 
        && pt.y < rHeader.top + HEADER_CLICK_HEIGHT * vga 
        && pt.x >= rList.right - HEADER_CLICK_HEIGHT * vga 
        && Screens[History[depth].screen].fnAdd != NULL) {

        PostMessage(hWnd, WM_COMMAND, CMD_ADD, NULL);
    }

    // "*" button in header bar
    else if (pt.y >= rHeader.top 
        && pt.y < rHeader.top + HEADER_CLICK_HEIGHT * vga 
        && pt.x >= rList.right - HEADER_CLICK_HEIGHT * vga 
        && Screens[History[depth].screen].fnToggleFavorite != NULL) {

        PostMessage(hWnd, WM_COMMAND, CMD_FAVORITE, NULL);
    }

    // They clicked the service name (to change iDialer service)
    else if (hasiDialerServices &&
        PtInRect(&rServiceTitle, ptMouseDown) 
        && PtInRect(&rServiceTitle, pt)) {
        // change the service
        PostMessage(hWnd, WM_COMMAND, CMD_CHANGE_SERVICE, NULL);
    }

    // They clicked in the titlebar
    // no matter what the screen type is
	else if (PtInRect(&rTitlebar, ptMouseDown) && PtInRect(&rTitlebar, pt)) {
        ShowWindow(hWnd, SW_MINIMIZE);
	}

    // Clicked a list item
    else if (pt.y >= rList.top && GetItemCount() > 0) {
        pt.y += History[depth].scrolled - rList.top;

        int nItem = GetPixelToItem(pt.y);

        if (pt.y >= StartPosition[nItem] && pt.y <= StartPosition[nItem + 1])
            PostMessage(hWnd, WM_COMMAND, CMD_CLICK_ITEM, nItem);
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
    RECT * pr;

	switch (wParam)	{

        ///// TIMER for scrolling
	    case IDT_TIMER_SCROLL:

            // Time
            dt = t - tEndTime;

            // Velocity
            if (History[depth].scrolled < minScrolled)
                Velocity = (double)(History[depth].scrolled - minScrolled) / 2 / dt;
            else if (History[depth].scrolled > maxScrolled)
                Velocity = (double)(History[depth].scrolled - maxScrolled) / 2 / dt;
            else {
                double dv = Velocity * FRICTION_COEFF * dt;
                if (fabs(dv) > fabs(Velocity)) 
                    Velocity = 0;
                else 
			        Velocity = Velocity - dv;

				// Gravity
				if (bGScrolling) {
					SENSORDATA sd;
					SensorGesturePoll(&sd);
					int sign = sd.AngleY < iInitialAngle ? -1 : 1;
					int dangle = abs(sd.AngleY - iInitialAngle);
					double radians = dangle * 3.141592654 / 180.0;
					Velocity += sin(radians) * 0.2 * sign;
				}
            }

            // Displacement
            s = Velocity * dt;
            if (s < 0 && s > -1 && History[depth].scrolled < minScrolled)
                s = -1;
            else if (s > 0 && s < 1 && History[depth].scrolled > maxScrolled)
                s = 1;
            
            // We're done scrolling
            if ((int)s == 0 && !bGScrolling) {
                KillTimer(hWnd, IDT_TIMER_SCROLL);
		        bScrolling = false;
		        Velocity = 0;
            }

            History[depth].scrolled -= (int)s;
            tEndTime = t;
            pr = Screens[History[depth].screen].hasMenus ? &rList : &rContent;
            InvalidateRect(hWnd, pr, false);
		    break;

        ///// TIMER for scroll to
        case IDT_TIMER_SCROLL_TO:
            KillTimer(hWnd, IDT_TIMER_SCROLL);
            Scroll_TimeCounter = (double)(t - Scroll_StartTime);
            if (Scroll_TimeCounter < Scroll_Duration) {
                bScrolling = true;

                // Cubic
                double amount = Scroll_Change
                    * (pow(Scroll_TimeCounter/Scroll_Duration - 1, 3) + 1);
                Velocity = (amount - Scroll_StartPosition) / Scroll_TimeCounter;
                History[depth].scrolled = Scroll_StartPosition + (int)amount;
            }
            else {
                bScrolling = false;
                Velocity = 0;
                KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
                History[depth].scrolled = Scroll_Change + Scroll_StartPosition;
            }
            InvalidateRect(hWnd, Screens[History[depth].screen].hasMenus 
                ? &rList : &rContent, false);
            break;

        case IDT_TIMER_TRANSITION:
            dTransitionPct = min(1.0, (double)(t - dwTransitionStart) 
                / nTransitionDuration);

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
    int index;

    if (bTransitioning || bDisplayingPopup)
        return 0;

    if (bScrolling) {
        KillTimer(hWnd, IDT_TIMER_SCROLL);
	    bScrolling = false;
	    Velocity = 0;
    }

    if (bRepeating) {
        nKeyRepeatCount++;
        if (nKeyRepeatCount < 10)
            bRepeating = false;
        else if (nKeyRepeatCount % 5 != 0)
            return 0;
    }
    else {
        nKeyRepeatCount = 0;
    }

	switch (wParam) {
		case VK_UP:
            index = SelectPreviousItem(
                GetPixelToItem(History[depth].scrolled + rListHeight),
                bRepeating);

            if (index == -1)
                break;

			// make sure the selected item is visible
            top = StartPosition[index];
			bot = StartPosition[index + 1];

            if (bScrolling) {
                History[depth].scrolled = max(0, 
                    min(top, ListHeight - rListHeight));
            }
			else if (
                   top < History[depth].scrolled 
                || bot > History[depth].scrolled + rListHeight
            ) {
                History[depth].scrolled = max(0, bot - rListHeight);
            }

			break;

	    case VK_DOWN:
            index = SelectNextItem(
                GetPixelToItem(History[depth].scrolled), bRepeating);

            if (index == -1)
                break;

			// make sure the selected item is visible
			top = StartPosition[index];
			bot = StartPosition[index + 1];

            if (bScrolling) {
                History[depth].scrolled = max(0, 
                    min(top, ListHeight - rListHeight));
            }
            else if (
                   top < History[depth].scrolled 
                || bot > History[depth].scrolled + rListHeight
            ) {
                History[depth].scrolled = min(top, ListHeight - rListHeight);
            }

			break;

        case VK_LEFT:
			if (depth > 1) {
                PostMessage(hWnd, WM_COMMAND, CMD_BACK, EXPAND_DETAILS_PERIOD);
			}
			else if (nCurrentTab > 0 && Screens[History[depth].screen].hasMenus) {
                PostMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, nCurrentTab - 1);
			}
            break;

        case VK_RIGHT:
			if (GetCurrentItemIndex() >= 0) {
				// simulate clicking on the right edge
				ptMouseDown.x = rList.right;
                PostMessage(hWnd, WM_COMMAND, CMD_CLICK_ITEM, GetCurrentItemIndex());
			}
			else if (nCurrentTab < 2) {
				PostMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, nCurrentTab + 1);
			}
			else if (nCurrentTab == 2) {
				PostMessage(hWnd, WM_COMMAND, CMD_GOTO_DIALER, NULL);
			}
            break;

	    case VK_TACTION:
			if (GetCurrentItemIndex() >= 0)
                PostMessage(hWnd, WM_COMMAND, CMD_CLICK_ITEM,
                    GetCurrentItemIndex());
			break;

        case VK_TTALK:
            PostMessage(hWnd, WM_COMMAND, CMD_GREEN_BUTTON, NULL);
            return 0;
            break;
	}

    InvalidateRect(hWnd,
        Screens[History[depth].screen].hasMenus ? &rList : &rContent,
        FALSE);
	UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// ReloadThread - Function to run as a thread, to reload the current list
//
void ReloadThread(LPVOID lpvThreadParam) {
    DataItem * parent = depth > 0 ? &History[depth-1].data : NULL;
    ListLoad(parent, Screens[History[depth].screen], pSettings);
    HWND hWnd = (HWND)lpvThreadParam;
    PostMessage(hWnd, WM_COMMAND, CMD_REFRESH, 0);
    return;
}

//-----------------------------------------------------------------------------
// DoCommand - Process WM_COMMAND message for window
//
LRESULT DoCommand (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
    DataItem * parent = NULL;
    DataItem dItem;
    int newScreen;
    //POINT pt;
    float x = 0;
    float y = 0;
    HRESULT hr;

    switch (wParam) {
        case CMD_GOTO_FAVORITES:
            PostMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, 0);
            break;

        case CMD_GOTO_RECENTS:
            PostMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, 1);
            break;

        case CMD_GOTO_CONTACTS:
            PostMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, 2);
            break;

        case CMD_GOTO_DIALER:
            RunDialer();
            if (pSettings->doExitOnAction)
                DestroyWindow(hWnd);
            break;

        case CMD_GOTO_SEARCH:
            if (nCurrentTab == 2)
                nHighlightedTab = 2;
            else
                SendMessage(hWnd, WM_COMMAND, CMD_SWITCH_TAB, 2);

            StartTransition(hWnd, ttKeyboardExpand, EXPAND_KEYBOARD_PERIOD);
            break;

        case CMD_SWITCH_TAB:
            // there are only 5 tabs...
            if (lParam < 0 || lParam > 4)
                break;

            if (3 == lParam) {
                PostMessage(hWnd, WM_COMMAND, CMD_GOTO_DIALER, NULL);
                break;
            }

            if (4 == lParam) {
                PostMessage(hWnd, WM_COMMAND, CMD_GOTO_SEARCH, NULL);
                break;
            }

            // if we get here, 0 <= lParam <= 2. perfect.

            SetCursor(LoadCursor(NULL, IDC_WAIT));

            depth = 1;

            // if "favorites" tab, carefully construct the previous in history
            if (lParam == 0) {
                History[0].data.ID = 1;
                StringCchCopy(History[0].data.szPrimaryText, 
                    PRIMARY_TEXT_LENGTH, 
                    pSettings->favorite_category);
            }
            // if "all contacts" tab, carefully construct the previous in history
            else if (lParam == 2) {
                History[0].data.ID = 0;
                StringCchCopy(History[0].data.szPrimaryText, 
                    PRIMARY_TEXT_LENGTH, 
                    pSettings->allcontacts_string);
            }
            else {
                History[0].data.ID = 0;
                History[0].data.szPrimaryText[0] = 0;
            }
            History[0].screen = Screens[lParam].parent;
            History[0].scrolled = 0;
            History[0].selectedIndex = -1;

            History[1].screen = lParam;
            History[1].scrolled = -rListHeight;
            History[1].selectedIndex = -1;

            nHighlightedTab = nCurrentTab = lParam;
            bScrolling = false;
            bDragging = false;

            parent = depth > 0 ? &History[depth-1].data : NULL;
            ListLoad(parent, Screens[History[depth].screen],
                pSettings, true);
            CalculateHeights();

            if (nCurrentTab == 1 && isRecentsTainted 
                || nCurrentTab == 2 && isPoomTainted
                || nCurrentTab == 0 && isFavoritesTainted) {
                PostMessage(hWnd, WM_COMMAND, CMD_RELOAD, NULL);
            }

            SetCursor(NULL);

            if (History[depth].scrolled < minScrolled)
                ScrollTo(hWnd, minScrolled);
            if (bTransitioning)
                dwTransitionStart = ::GetTickCount();

            break;

        case CMD_RELOAD:
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            ReloadThread((LPVOID)hWnd);

            //HANDLE hThread = CreateThread(NULL, NULL,
            //    (LPTHREAD_START_ROUTINE)ReloadThread, (LPVOID)hWnd, NULL, NULL);
            //SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);

            if (History[depth].screen == 1)
                isRecentsTainted = false;
            else if (History[depth].screen == 2)
                isPoomTainted = false;
            else if (History[depth].screen == 0)
                isFavoritesTainted = false;

            SetCursor(NULL);

            break;

        case CMD_REFRESH:
            CalculateHeights();

            if (History[depth].scrolled < minScrolled)
                ScrollTo(hWnd, minScrolled);
            if (History[depth].scrolled > maxScrolled)
                ScrollTo(hWnd, maxScrolled);
            break;

        case CMD_CLICK_ITEM:
            if (!CanSelectItem(lParam))
                break;

            dItem = SelectItem(lParam);

            x = (float)ptMouseDown.x / rList.right;
            newScreen = History[depth].screen;
            hr = Screens[newScreen].fnClick(&dItem, x, &newScreen, pSettings);

            if (SUCCEEDED(hr)) {
                // "back"
                if (newScreen == -1) {
                    if (pSettings->doExitOnAction) {
                        DestroyWindow(hWnd);
                    }
                    else {
                        PostMessage(hWnd, WM_COMMAND, CMD_BACK, EXPAND_DETAILS_PERIOD);
                    }
                }

                // action is done, get outta here
                else if (newScreen == -2) {
                    if (pSettings->doExitOnAction) {
                        DestroyWindow(hWnd);
                    }
                    else {
                        PostMessage(hWnd, WM_COMMAND, CMD_REFRESH, NULL);
                    }
                }

                // "back" but upon deactivation
                // this is used in conjunction with CreateProcess
                // to avoid funky back action before the new window shows up
                else if (newScreen == -3) {
                    bBackOnDeactivate = true;
                }

                // "forward to screen x"
                else if (newScreen != History[depth].screen) {
                    PostMessage(hWnd, WM_COMMAND, CMD_FORWARD, newScreen);
                }

                else {
                    UnselectItem();
                }
            }
            break;

        case CMD_BACK:
            if (depth <= 0)
                break;

            SetCursor(LoadCursor(NULL, IDC_WAIT));

            depth--;
            parent = depth > 0 ? &History[depth-1].data : NULL;
            ListLoad(parent, Screens[History[depth].screen], pSettings, true);
            CalculateHeights();

            if (History[depth].screen <= 2)
                nHighlightedTab = nCurrentTab = History[depth].screen;
            else if (History[depth].screen == 3)
                nHighlightedTab = nCurrentTab = 2;

            StartTransition(hWnd, ttSlideRight, (int)lParam);

            SetCursor(NULL);
            break;

        case CMD_FORWARD:
            if (depth > 2 || GetCurrentItemIndex() == -1)
                break;

            // store the selected item in History
            memcpy(&History[depth].data, &GetCurrentItem(), sizeof(DataItem));

            parent = &History[depth++].data;

            //TODO: ? History[depth].data = ??
            History[depth].screen = lParam;
            History[depth].scrolled = 0;
            History[depth].selectedIndex = -1;

            ListLoad(parent, Screens[History[depth].screen], pSettings);
            CalculateHeights();

            if (History[depth].screen <= 2)
                nHighlightedTab = nCurrentTab = History[depth].screen;

            StartTransition(hWnd, ttSlideLeft, EXPAND_DETAILS_PERIOD);
            break;

        case CMD_ADD:
            if (Screens[History[depth].screen].fnAdd != NULL)
                Screens[History[depth].screen].fnAdd();
            break;
        
        case CMD_FAVORITE:
            if (Screens[History[depth].screen].fnToggleFavorite == NULL)
                break;

            SetCursor(LoadCursor(NULL, IDC_WAIT));
            Screens[History[depth].screen].fnToggleFavorite(
                &History[depth-1].data, pSettings);
            SetCursor(NULL);

            // Switch back to list of favorites, if they just
            // de-favorited one of them. Presumably, the one they
            // just favorited will be missing from the list now.
            if (nCurrentTab == 0) {
                PostMessage(hWnd, WM_COMMAND, CMD_BACK, EXPAND_DETAILS_PERIOD);
            }
            break;

        case CMD_CHANGE_SERVICE:
            NextIDialerService();
            break;

        case CMD_GREEN_BUTTON:
            // TODO: dial currently selected number...?
            RunDialer();
            break;
    }

    InvalidateRect(hWnd, &rScreen, FALSE);
    UpdateWindow(hWnd);

    return 0;
}

//-----------------------------------------------------------------------------
// DoDestroyMain - Process WM_DESTROY message for window
//
LRESULT DoDestroyMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) {
   
    // Uninitialize the COM classes
    CoUninitialize();

    // Clean up HREGNOTIFY's
    for (int i = 0; i < HR_NOTIFY_COUNT; i++) {
        if (NULL != hrNotify[i]) {
            RegistryCloseNotification(hrNotify[i]);
            hrNotify[i] = 0;
        }
    }

    // Clean up titlebar
	if (pSettings->doShowFullScreen)
	    DestroyTitlebar();

	// Clean up sensors
	SensorGestureUninit();
	SensorPollingUninit();

    // Quit
    PostQuitMessage (0);
    return 0;
}

//-----------------------------------------------------------------------------
// Screen Drawing Functions
//
void DrawScreenOn(HDC hdc, RECT rClip, HDC hdcTmp, int yListOffset) {
    RECT rect;

    // MAIN CONTENT
    if (Screens[History[depth].screen].hasMenus) {
        IntersectRect(&rect, &rList, &rClip);
        if (!IsRectEmpty(&rect))
            DrawContentOn(hdc, rect, hdcTmp, yListOffset);

		IntersectRect(&rect, &rMenubar, &rClip);
		if (!IsRectEmpty(&rect))
			DrawMenubarOn(hdc);
	}

    else {
		IntersectRect(&rect, &rContent, &rClip);
		if (!IsRectEmpty(&rect))
			DrawContentOn(hdc, rect, hdcTmp, yListOffset);
	}

    // TITLE BAR
    IntersectRect(&rect, &rTitlebar, &rClip);
    if (!IsRectEmpty(&rect))
	    DrawTitlebarOn(hdc, rTitlebar, hdcSkin, TitlebarFont, szWindowTitle);

    // HEADER BAR
    IntersectRect(&rect, &rHeader, &rClip);
    if (!IsRectEmpty(&rect))
        DrawHeaderOn(hdc, rHeader, hdcSkin);

}

void DrawContentOn(HDC hdc, RECT rect, HDC hdcTmp, int yOffset) {
	int         nFirstItem, nItem;
    DataItem    dItemTmp;
    TCHAR       buffer[SECONDARY_TEXT_LENGTH];
    int         count = GetItemCount();
    int         currentItemIndex = GetCurrentItemIndex();
    bool        canGetGroup = NULL != Screens[History[depth].screen].fnGetGroup;
    RECT        rItem = rList;
    int         ryOffset = yOffset + rect.top - rContent.top;

    // related to the icon for this list (the contact's picture)
    HBITMAP     hBitmap = NULL;
    int         iBitmapHeight = 0;
    int         iBitmapWidth = 0;
    
    if (depth > 1) {
        hBitmap = GetHBitmap(&History[depth-1].data,
            Screens[History[depth].screen], ITEM_DETAILS_PICTURE_SIZE * vga);
        iBitmapHeight = GetHBitmapHeight();
        iBitmapWidth = GetHBitmapWidth();
    }

#ifdef DEBUG_GRAPHICS_SPEED
    DWORD tTime1 = ::GetTickCount();
#endif

	// ******* draw list background (if any) that appears above the list items

    if (ryOffset < 0) {
        rItem.bottom = rect.top - ryOffset;
		DrawCanvasOn(hdc, rItem);
    }

#ifdef DEBUG_GRAPHICS_SPEED
    DWORD tTime2 = ::GetTickCount();
#endif

	SetBkMode(hdc, TRANSPARENT);

    // now start drawing the content
	// special case for 0 list items
	if (count == 0) {
		rItem.bottom = rItem.top;
	}

	else {
		// We want to draw all items, but for performance reasons,
		// there's no need to draw any items that are beyond the 
		// top of the screen. So determine the first visible item.
		nFirstItem = nItem = ryOffset <= 0 ? 0 : GetPixelToItem(ryOffset);
		rItem.bottom = rect.top + StartPosition[nItem] - ryOffset;

#ifdef DEBUG_GRAPHICS_SPEED
    DWORD tTime3 = ::GetTickCount();
#endif


		// ******* DRAW LIST ITEMS
		while (nItem < count && rItem.bottom < rect.bottom) {
			dItemTmp = GetItem(nItem);

			rItem.top = rItem.bottom;

			// This is for detecting if we have the start of a type group
			// (a bunch of diPhone, or diEmail, etc.)
			if (IsItemNewType(nItem)) {
				rItem.bottom = rItem.top
					+ StartPosition[nItem + CountSameTypeAs(nItem)]
					- StartPosition[nItem];

				DrawItemBackgroundOn(hdc, dItemTmp.type, rItem);

				// ******* Draw the associated bitmap, if there is one
				if (nItem <= 1 && NULL != hBitmap) {
					int left = ITEM_DETAILS_PADDING * vga;
					int right = left + iBitmapWidth;
					int top = rContent.top - yOffset
						+ (DEFAULT_ITEM_HEIGHT * vga * 2 - iBitmapHeight) / 2;
					int bottom = top + iBitmapHeight;

					// draw black square with a black border of 1
					RECT rcBitmap = { left - 1, top - 1, right + 1, bottom + 1};
					DrawRect(hdc, &rcBitmap, (COLORREF)0);

					// paint the bitmap on the DC
					TransparentImage(hdc, left, top, iBitmapWidth, iBitmapHeight, 
						hBitmap, 0, 0, iBitmapWidth, iBitmapHeight, (COLORREF)0);
				}
			}

			// special case: draw the background starting with one above
			// the first visible item, for proper "rounded corners" effect
			else if (nItem == nFirstItem) {
				rItem.bottom = rItem.top
					+ StartPosition[nItem + CountSameTypeAs(nItem)]
					- StartPosition[nItem];
				int offset = StartPosition[nItem] - StartPosition[nItem - 1];
				rItem.top -= offset;
				DrawItemBackgroundOn(hdc, dItemTmp.type, rItem);
				rItem.top += offset;
			}

			rItem.bottom = rItem.top + StartPosition[nItem + 1] 
				- StartPosition[nItem];

			if (hBitmap && nItem <= 1) {
				rItem.left = ITEM_DETAILS_PADDING * vga + iBitmapWidth;
			}

			if (nItem == currentItemIndex && !bMouseDown) {
				rClickRegion = rItem;
			}
			DrawItemOn(hdc, dItemTmp, rItem);

			rItem.left = rList.left;

			// ****** Group Header
			if (dItemTmp.iGroup 
				&& rItem.top >= rect.top
				&& canGetGroup
				&& IsItemNewGroup(nItem)) {

				DrawGroupHeaderOn(hdc, dItemTmp, rItem);
			} 

			// Next nItem
			nItem++;
		}
	}

#ifdef DEBUG_GRAPHICS_SPEED
    DWORD tTime4 = ::GetTickCount();
#endif

    // draw the list background (if any) that appears below the list
    // Also, now the "about" text might be visible
    if (rItem.bottom < rect.bottom) {
        rItem.top = rItem.bottom;
        rItem.bottom = rect.bottom;
		DrawCanvasOn(hdc, rItem);
        
        // "About" at bottom of screen
        SelectObject(hdc, SecondaryListFont);
        SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
        SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_HEADER_LOADING_TEXT));
        int y = Screens[History[depth].screen].hasMenus 
            ? rList.bottom - 2 : rContent.bottom - 2;
        ExtTextOut(hdc, rContent.left + 2, y, 
            ETO_CLIPPED, &rItem, SZ_ABOUT, _tcslen(SZ_ABOUT), 0);
    }

	if (count == 0)
		return;

	// Special: Draw the group of the list nItem that's at the top of the list
	dItemTmp = GetItem(nFirstItem);
	if (dItemTmp.iGroup && yOffset >= 0 && canGetGroup) {

		int nHeight = DEFAULT_GROUP_HEIGHT * vga;
		int nBottom = nHeight;

		RECT rTopGroup = {rContent.left, 0, rContent.right, nHeight};
		DrawGroupHeaderOn(hdcTmp, dItemTmp, rTopGroup);

		if (nFirstItem < count - 1 && IsItemNewGroup(nFirstItem + 1)) {
			nBottom = min(nBottom, StartPosition[nFirstItem + 1] - yOffset);
		}

		// account for the fact that the list
		// doesn't start at the top of the screen
		nBottom += rContent.top;

		int nLeft = rContent.left;
		int nWidth = rContent.right - rContent.left;
		int nTop = nBottom - nHeight;

		if (bScrolling && pSettings->doFastGraphics) {
			BitBlt(hdc, nLeft, nTop, nWidth, nHeight, hdcTmp, 0, 0, SRCCOPY);
		}
		else {
			BltAlpha(hdc, nLeft, nTop, nWidth, nHeight, hdcTmp, 200);
		}
	}

#ifdef DEBUG_GRAPHICS_SPEED
    DWORD tTime5 = ::GetTickCount();
#endif

    // Draw list indicator if scrolling
	if (bScrolling && (Velocity < -0.1 || Velocity > 0.1) && canGetGroup) {
		SelectObject(hdc, ListIndicatorFont);
	    SetTextAlign(hdc, TA_CENTER);
		SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_LIST_ITEM_TEXT));
		SetBkMode(hdc, TRANSPARENT);

        // dItemTmp is set previously to the first item
        Screens[History[depth].screen].fnGetGroup(
            &dItemTmp, buffer, SECONDARY_TEXT_LENGTH, pSettings);

        ExtTextOut(hdc, (rContent.right + rContent.left) / 2, 
            rContent.top + 10, NULL, NULL, buffer, _tcslen(buffer), 0);
	}

#ifdef DEBUG_GRAPHICS_SPEED
    DWORD tTime6 = ::GetTickCount();
    TCHAR tszTimes[1024];
    StringCchPrintf(tszTimes, 1024, TEXT("2:%d, 3:%d, 4:%d, 5:%d, 6:%d"), tTime2-tTime1, tTime3-tTime2, tTime4-tTime3, tTime5-tTime4, tTime6-tTime5);
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_LEFT | TA_TOP);
    SetTextColor(hdc, pSettings->rgbListItemText);
    ExtTextOut(hdc, rect.left + 2, rect.top + 2, 
        NULL, NULL, tszTimes, _tcslen(tszTimes), 0);
#endif

}

void DrawMenubarOn(HDC hdc) {
    // MENU BAR
    int rMenubarWidth = rMenubar.right - rMenubar.left;

	// this is only necessary if in landscape mode
	if (rMenubarWidth > DEFAULT_SCREEN_WIDTH * vga) {
		// Copy the first (1 * vga) columns of the 
		// menu bar from skin to memory
		BitBlt(hdc, 0, rMenubar.top, vga, SKIN_MENU_BAR_Y_OFFSET * vga,
			hdcSkin, 0, SKIN_MENU_BAR_HEIGHT * vga, SRCCOPY);

		// This will copy the first (1 * vga) columns of 
		// the menu bar fully across the screen
		for (int x = vga; x < rMenubarWidth; x *= 2) {
			int w = 2 * x > rMenubarWidth ? rMenubarWidth - x : x;
			BitBlt(hdc, x, rMenubar.top, w, SKIN_MENU_BAR_HEIGHT * vga,
				hdc, 0, rMenubar.top, SRCCOPY);
		}
	}

    // figure out the highlighted tab
    int tab = bMouseDown && ptMouseDown.y > rMenubar.top 
        ? ptMouseDown.x * 5 / rMenubarWidth : nHighlightedTab;

    // draw buttons
    for (int i = 0; i < 5; i++) {
        int xdest = rMenubar.left 
            + rMenubarWidth / 10 * (2 * i + 1) - MENU_BAR_ICON_WIDTH * vga / 2;
        int ydest = rMenubar.top;
        int xsrc = i * SKIN_MENU_BAR_ICON_WIDTH * vga;
        int ysrc = i == tab 
            ? SKIN_MENU_BAR_SEL_Y_OFFSET * vga 
            : SKIN_MENU_BAR_Y_OFFSET * vga;
        BitBlt(hdc, xdest, ydest, MENU_BAR_ICON_WIDTH * vga, SKIN_MENU_BAR_HEIGHT * vga, 
            hdcSkin, xsrc, ysrc, SRCCOPY);
    }
}

void DrawGroupHeaderOn(HDC hdc, DataItem dItem, RECT rItem) {
    RECT rHeader = rItem;
    rHeader.bottom = rHeader.top + DEFAULT_GROUP_HEIGHT * vga;
    TCHAR buffer[SECONDARY_TEXT_LENGTH];

    Screens[History[depth].screen].fnGetGroup(
        &dItem, buffer, SECONDARY_TEXT_LENGTH, pSettings);
    int length = _tcslen(buffer);

	// ****** GroupHeader background
	DrawRect(hdc, &rHeader, GetSkinRGB(SKIN_COLOR_LIST_GROUP_BACKGROUND));

    // separator
    RECT rSep = rHeader;
    rSep.top = rHeader.bottom - LIST_SEPARATOR_HEIGHT * vga;
    DrawRect(hdc, &rSep, GetSkinRGB(SKIN_COLOR_LIST_ITEM_SEPARATOR));
	SetTextAlign(hdc, TA_LEFT);

	// ******* Draw Group Header Text
	SelectObject(hdc, GroupFont);
   	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_LIST_GROUP_TEXT));
	ExtTextOut(hdc, rItem.left + LIST_GROUP_ITEM_INDENT * vga, rHeader.top - 1 
		+ ((DEFAULT_GROUP_HEIGHT - GROUP_ITEM_FONT_SIZE) * vga / 2),
        NULL, NULL, buffer, length, 0);
}

void DrawItemBackgroundOn(HDC hdc, DataItemType diType, RECT rect) {
    switch (diType) {
        case diListItem:
            // Item Background
            DrawRect(hdc, &rect, GetSkinRGB(SKIN_COLOR_LIST_ITEM_BACKGROUND));
            break;
        case diNothing:
        case diName:
        case diCompany:
        case diText:
            // Just draw the canvas
			DrawCanvasOn(hdc, rect);
            break;
        case diEmail:
        case diUrl:
        case diPhone:
        case diDetailsButton:
        case diCallButton:
        case diSmsButton:
        case diEditButton:
        case diSaveContactButton:
            // Draw the canvas
			DrawCanvasOn(hdc, rect);

            // Draw the button background
            // TODO: make the button prettier
            SelectObject(hdc, CreateSolidBrush(GetSkinRGB(
				SKIN_COLOR_LIST_ITEM_BACKGROUND)));
				
            SetTextColor(hdc, GetSkinRGB(
				SKIN_COLOR_LIST_ITEM_TEXT));
            RoundRect(hdc, 
                rect.left + ITEM_DETAILS_PADDING * vga / 2, 
                rect.top + ITEM_DETAILS_PADDING * vga / 3, 
                rect.right - ITEM_DETAILS_PADDING * vga / 2, 
                rect.bottom - ITEM_DETAILS_PADDING * vga / 3,
                ITEM_DETAILS_PADDING * vga, ITEM_DETAILS_PADDING * vga);
            break;
    }
}

void DrawItemOn(HDC hdc, DataItem dItem, RECT rItem) {
    RECT rClip, rSep;
    SIZE textSize;
    int y, slen;
    COLORREF color;

    rClip.left = rItem.left + ITEM_DETAILS_PADDING * vga / 2 + 2;
    rClip.right = rItem.right - ITEM_DETAILS_PADDING * vga / 2 - 2;
    rClip.top = rItem.top + ITEM_DETAILS_PADDING * vga / 2 + 2;
    rClip.bottom = rItem.bottom - ITEM_DETAILS_PADDING * vga / 2 - 2;

    switch (dItem.type) {
        case diListItem:
            // separator
            rSep = rItem;
            rSep.top = rItem.bottom - LIST_SEPARATOR_HEIGHT * vga;
            DrawRect(hdc, &rSep, GetSkinRGB(SKIN_COLOR_LIST_ITEM_SEPARATOR));

            // Item Primary Text
	        SetTextAlign(hdc, TA_LEFT);
	        SelectObject(hdc, PrimaryListFont);
            color = GetSkinRGB(dItem.isFavorite && nCurrentTab != 0 
                ? SKIN_COLOR_LIST_ITEM_FAVORITE
                : dItem.isMissed ? SKIN_COLOR_LIST_ITEM_MISSED
                : SKIN_COLOR_LIST_ITEM_TEXT);
	        SetTextColor(hdc, color);
	        ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT * vga, rItem.bottom - 2 
				- ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) * vga / 2),
                ETO_OPAQUE, NULL, dItem.szPrimaryText, 
                _tcslen(dItem.szPrimaryText), 0);

	        // Item Secondary Text
            slen = _tcslen(dItem.szSecondaryText);
            if (slen > 0) {
                SelectObject(hdc, SecondaryListFont);
                SetTextAlign(hdc, TA_RIGHT);
                SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_LIST_ITEM_TEXT));
                ExtTextOut(hdc, rItem.right - LIST_ITEM_INDENT * vga, rItem.bottom - 2 
					- ((DEFAULT_ITEM_HEIGHT + ITEM_SECONDARY_FONT_SIZE) * vga / 2),
	                ETO_OPAQUE, NULL, dItem.szSecondaryText, slen, 0);
            }

            break;

        case diName:
            SelectObject(hdc, PrimaryListFont);
            SetTextAlign(hdc, TA_LEFT | TA_TOP);

            // Display the shadow
            if (GetSkinRGB(SKIN_COLOR_DETAIL_MAIN_SHADOW) 
                != GetSkinRGB(SKIN_COLOR_DETAIL_MAIN_TEXT)) {

                SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_DETAIL_MAIN_SHADOW));
		        ExtTextOut(hdc, rItem.left + ITEM_DETAILS_PADDING * vga + 1, 
                    rItem.top + ITEM_DETAILS_PADDING * vga + 1,
                    ETO_CLIPPED, &rClip, dItem.szPrimaryText, 
                    _tcslen(dItem.szPrimaryText), NULL);
            }

            // Display the name
            SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_DETAIL_MAIN_TEXT));
		    ExtTextOut(hdc, rItem.left + ITEM_DETAILS_PADDING * vga, 
                rItem.top + ITEM_DETAILS_PADDING * vga,
                ETO_CLIPPED, &rClip, dItem.szPrimaryText, 
                    _tcslen(dItem.szPrimaryText), NULL);
            break;

        case diText:
		    SelectObject(hdc, ItemDetailsFont);
            SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_LIST_ITEM_TEXT));
            SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
            y = (rItem.bottom + rItem.top) / 2;

            // Draw the label: "Home", "Work", etc.
            rClip.right = rItem.left + ITEM_DETAILS_PADDING * vga * 5;
            ExtTextOut(hdc, rClip.right, 
                y + (ITEM_DETAILS_FONT_SIZE * vga / 2),
                ETO_CLIPPED, &rClip, dItem.szSecondaryText,
                _tcslen(dItem.szSecondaryText), NULL);

            rClip.left = rClip.right + 3;
            rClip.right = rItem.right - ITEM_DETAILS_PADDING * vga;

            // Now display the text
            SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
            SelectObject(hdc, PrimaryListFont);
		    ExtTextOut(hdc, rClip.left + ITEM_DETAILS_PADDING * vga / 3, 
                y + (ITEM_FONT_SIZE * vga / 2),
                ETO_CLIPPED, &rClip, dItem.szPrimaryText,
                _tcslen(dItem.szPrimaryText), NULL);

            break;

        case diCompany:
            // Display the company name
            SelectObject(hdc, SecondaryListFont);
            SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_DETAIL_MAIN_TEXT));
		    ExtTextOut(hdc, rItem.left + ITEM_DETAILS_PADDING * vga, rItem.top,
                NULL, NULL, dItem.szPrimaryText,
                _tcslen(dItem.szPrimaryText), NULL);
            break;

        case diPhone:
		    SelectObject(hdc, ItemDetailsFont);
            SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_LIST_ITEM_TEXT));
            SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
            y = (rItem.bottom + rItem.top) / 2;

            // Draw the label: "Home", "Work", etc.
            rClip.right = rItem.left + ITEM_DETAILS_PADDING * vga * 5;
            ExtTextOut(hdc, rClip.right, 
                y + (ITEM_DETAILS_FONT_SIZE * vga / 2),
                ETO_CLIPPED, &rClip, dItem.szSecondaryText,
                _tcslen(dItem.szSecondaryText), NULL);

            rClip.left = rClip.right + 3;
            rClip.right = rItem.right - ITEM_DETAILS_PADDING * vga;

            // Draw the right label: "SMS"
            rClip.right -= ITEM_DETAILS_PADDING * vga * 2;
            ExtTextOut(hdc, rItem.right - ITEM_DETAILS_PADDING * vga, 
                y + (ITEM_DETAILS_FONT_SIZE * vga / 2),
                NULL, NULL, pSettings->sms_string, 
                _tcslen(pSettings->sms_string), NULL);

            // Now display the phone number
            SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
            SelectObject(hdc, PrimaryListFont);
		    ExtTextOut(hdc, rClip.left + ITEM_DETAILS_PADDING * vga / 3, 
                y + (ITEM_FONT_SIZE * vga / 2),
                ETO_CLIPPED, &rClip, dItem.szPrimaryText,
                _tcslen(dItem.szPrimaryText), NULL);

            break;

        case diEmail:
        case diUrl:
            SelectObject(hdc, PrimaryListFont);

            GetTextExtentPoint(hdc, dItem.szPrimaryText,
                _tcslen(dItem.szPrimaryText), &textSize);

            // The text is too wide to fit, so left-justify & clip it
            if (textSize.cx > rClip.right - rClip.left) {
                SetTextAlign(hdc, TA_LEFT);
                ExtTextOut(hdc, rClip.left,
                    (rItem.bottom - rItem.top - ITEM_FONT_SIZE * vga) / 2 + rItem.top,
                    ETO_CLIPPED, &rClip, dItem.szPrimaryText,
                    _tcslen(dItem.szPrimaryText), 0);
            }

            else {
                SetTextAlign(hdc, TA_CENTER);
                ExtTextOut(hdc, (rClip.right - rClip.left) / 2 + rClip.left,
                    (rItem.bottom - rItem.top - ITEM_FONT_SIZE * vga) / 2 + rItem.top,
                    NULL, NULL, dItem.szPrimaryText,
                    _tcslen(dItem.szPrimaryText), 0);
            }
            break;

        case diDetailsButton:
        case diCallButton:
        case diSmsButton:
        case diEditButton:
        case diSaveContactButton:
            // Display the button text
            SelectObject(hdc, PrimaryListFont);
            SetTextAlign(hdc, TA_CENTER);

            ExtTextOut(hdc, (rItem.right - rItem.left) / 2 + rItem.left,
                (rItem.bottom - rItem.top - ITEM_FONT_SIZE * vga) / 2 + rItem.top,
                ETO_CLIPPED, &rClip, dItem.szPrimaryText, 
                    _tcslen(dItem.szPrimaryText), 0);



            break;
    }
}

void DrawKeyboardOn(HDC hdc, RECT rKeyboard) {
	SelectObject(hdc, KeyboardFont);
	SelectObject(hdc, CreatePen(PS_SOLID, 1, GetSkinRGB(SKIN_COLOR_KEYBOARD_GRID)));
    SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_KEYBOARD_TEXT));
	SetBkMode(hdc, TRANSPARENT);
    int x, y, g, h;

    DrawRect(hdc, &rKeyboard, GetSkinRGB(SKIN_COLOR_KEYBOARD_BACKGROUND));

    SetTextAlign(hdc, TA_CENTER);

    // Draw the horizontal lines
    for (h = 0; h < nKeyboardRows; h++) {
        y = rKeyboard.top + h 
            * (rKeyboard.bottom - rKeyboard.top) / nKeyboardRows;
        MoveToEx(hdc, rKeyboard.left, y, (LPPOINT) NULL);
        LineTo(hdc, rKeyboard.right, y);
    }
    MoveToEx(hdc, rKeyboard.left, rKeyboard.bottom - 1, (LPPOINT) NULL);
    LineTo(hdc, rKeyboard.right, rKeyboard.bottom - 1);

    // Draw the vertical lines
    for (g = 0; g < nKeyboardCols; g++) {
        x = rKeyboard.left + g 
            * (rKeyboard.right - rKeyboard.left) / nKeyboardCols;
        MoveToEx(hdc, x, rKeyboard.top, (LPPOINT) NULL);
        LineTo(hdc, x, rKeyboard.bottom);
    }
    MoveToEx(hdc, rKeyboard.right - 1, rKeyboard.top, (LPPOINT) NULL);
    LineTo(hdc, rKeyboard.right - 1, rKeyboard.bottom);

    // Draw the letters
    int i = 0;
	for (h = 0; h <= nKeyboardRows; h++) {
        y = rKeyboard.top 
            + ((GroupHeight - KEYBOARD_FONT_SIZE * vga) / 2) 
            + (GroupHeight * h);

        for (g = 0; g < nKeyboardCols; g++) {
            x = rKeyboard.left + (GroupWidth / 2) + (GroupWidth * g);

            if (i < nKeyboardLetters) {
                ExtTextOut(hdc, x, y, NULL, NULL, &alphabet[i++], 1, 0);
            }
		}
	}
}

void DrawHeaderOn(HDC hdc, RECT rHeader, HDC hdcSkin) {
    // The background of the header bar
    StretchBlt(hdc, rHeader.left, rHeader.top, 
        rHeader.right - rHeader.left, rHeader.bottom - rHeader.top,
        hdcSkin, 0, SKIN_HEADER_Y_OFFSET * vga, 1, SKIN_HEADER_HEIGHT * vga, SRCCOPY);

    if (!bTransitioning) {
        // The "back" button
        if (Screens[History[depth].screen].parent >= 0) {
            StretchBlt(hdc, rHeader.left, rHeader.top, 
                MENU_BAR_ICON_WIDTH * vga, HEADER_HEIGHT * vga,
                hdcSkin, 0, SKIN_HEADER_Y_OFFSET * vga, 
                SKIN_MENU_BAR_ICON_WIDTH * vga, SKIN_HEADER_HEIGHT * vga,
                SRCCOPY);
        }

        // The "+" to add a contact
        if (Screens[History[depth].screen].fnAdd != NULL) {
            StretchBlt(hdc, 
                rHeader.right - MENU_BAR_ICON_WIDTH * vga, rHeader.top,
                MENU_BAR_ICON_WIDTH * vga, HEADER_HEIGHT * vga,
                hdcSkin, 
                SKIN_MENU_BAR_ICON_WIDTH * 4 * vga, SKIN_HEADER_Y_OFFSET * vga, 
                SKIN_MENU_BAR_ICON_WIDTH * vga, SKIN_HEADER_HEIGHT * vga,
                SRCCOPY);
        }

        // The "favorite YES" icon
        if (Screens[History[depth].screen].fnToggleFavorite != NULL) {
            if (History[depth-1].data.isFavorite) {
                StretchBlt(hdc, 
                    rHeader.right - MENU_BAR_ICON_WIDTH * vga, rHeader.top, 
                    MENU_BAR_ICON_WIDTH * vga, HEADER_HEIGHT * vga,
                    hdcSkin, 
                    SKIN_MENU_BAR_ICON_WIDTH * 3 * vga, SKIN_HEADER_Y_OFFSET * vga, 
                    SKIN_MENU_BAR_ICON_WIDTH * vga, SKIN_HEADER_HEIGHT * vga,
                    SRCCOPY);
            }

            // The "favorite NO" icon
            else {
                StretchBlt(hdc, 
                    rHeader.right - MENU_BAR_ICON_WIDTH * vga, rHeader.top, 
                    MENU_BAR_ICON_WIDTH * vga, HEADER_HEIGHT * vga, hdcSkin, 
                    SKIN_MENU_BAR_ICON_WIDTH * 2 * vga, SKIN_HEADER_Y_OFFSET * vga, 
                    SKIN_MENU_BAR_ICON_WIDTH * vga, SKIN_HEADER_HEIGHT * vga,
                    SRCCOPY);
            }
        }
    }

    // The title
    SelectObject(hdc, PrimaryListFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextAlign(hdc, TA_LEFT);

    SetTextColor(hdc, GetSkinRGB(SKIN_COLOR_HEADER_TEXT));

    TCHAR szTitle[PRIMARY_TEXT_LENGTH] = {0};
    HRESULT hr = Screens[History[depth].screen].fnGetTitle(
        &History[depth-1].data, szTitle, PRIMARY_TEXT_LENGTH, pSettings);
    DrawText(hdc, szTitle, -1, &rHeader, DT_CENTER | DT_VCENTER); 
}

// TODO: this more efficiently
void DrawCanvasOn(HDC hdc, RECT rect) {
	int canvasHeight = SKIN_CANVAS_HEIGHT * vga;
    for (int i = rect.top; i < rect.bottom; i += canvasHeight) {
		int h = i + canvasHeight > rect.bottom ? rect.bottom - i : canvasHeight;
        BitBlt(hdc, rect.left, i, rect.right - rect.left, 
            canvasHeight, hdcCanvas, 0, 0, SRCCOPY);
    }
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
    nScreenHeight = rScreen.bottom - rScreen.top;
    int nScreenWidth = rScreen.right - rScreen.left;

    // Title bar, with date, carrier, battery, signal strength, etc.
	rTitlebar = rScreen;
	if (pSettings->doShowFullScreen) {
		rTitlebar.bottom = rTitlebar.top + TITLE_BAR_HEIGHT * vga;

		// Location of the name of the service
		rServiceTitle = rTitlebar;
		rServiceTitle.bottom *= 2;
		rServiceTitle.left += SIGNAL_WIDTH * vga;
		rServiceTitle.right = rServiceTitle.right / 2 - SIGNAL_WIDTH * vga;
	}
	else {
		// collapse new titlebar so it is not active
		rTitlebar.bottom = rTitlebar.top;
		rServiceTitle = rTitlebar;
	}

    // Header, with the "back" button, the "favorite" button, etc.
    rHeader = rScreen;
    rHeader.top = rTitlebar.bottom;
    rHeader.bottom = rHeader.top + HEADER_HEIGHT * vga;

    // Menu at the bottom of the screen
	rMenubar = rScreen;
	rMenubar.top = rMenubar.bottom - MENU_BAR_HEIGHT * vga;

    // From the header to the bottom of the screen
    rContent = rScreen;
    rContent.top = rHeader.bottom;

    // Between the header and the menu bar
	rList = rContent;
	rList.bottom = rMenubar.top;
    rListHeight = rList.bottom - rList.top;

    // Calculate how many rows / cols for the keyboard
    double screenRatio = (double)nScreenWidth / nScreenHeight;
    nKeyboardRows = 1;
    nKeyboardCols = 1;
    while (nKeyboardRows * nKeyboardCols < nKeyboardLetters) {
        if (nScreenWidth < nScreenHeight) {
            nKeyboardRows = (int)(++nKeyboardCols / screenRatio);
        }
        else {
            nKeyboardCols = (int)(++nKeyboardRows * screenRatio);
        }
    }
    if (nScreenWidth < nScreenHeight) {
        while ((nKeyboardRows - 1) * nKeyboardCols >= nKeyboardLetters) {
            nKeyboardRows--;
        }
    }
    else {
        while (nKeyboardRows * (nKeyboardCols - 1) >= nKeyboardLetters) {
            nKeyboardCols--;
        }
    }
    GroupWidth = nScreenWidth / nKeyboardCols;
    GroupHeight = nScreenHeight / nKeyboardRows;


    // Initialize the DCs and Bitmaps
    if (hdcMem)
        CBR(DeleteDC(hdcMem));
    hdcMem = CreateCompatibleDC(hdc);
    if (hbmMem)
        CBR(DeleteObject(hbmMem));
	hbmMem = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcMem, hbmMem);

    if (hdcTmp)
        CBR(DeleteDC(hdcTmp));
    hdcTmp = CreateCompatibleDC(hdc);
	if (hbmTmp)
        CBR(DeleteObject(hbmTmp));
	hbmTmp = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcTmp, hbmTmp);

    if (hdcPage1)
        CBR(DeleteDC(hdcPage1));
    hdcPage1 = CreateCompatibleDC(hdc);
    if (hbmPage1)
        CBR(DeleteObject(hbmPage1));
    hbmPage1 = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcPage1, hbmPage1);

    if (hdcPage2)
        CBR(DeleteDC(hdcPage2));
    hdcPage2 = CreateCompatibleDC(hdc);
    if (hbmPage2)
        CBR(DeleteObject(hbmPage2));
    hbmPage2 = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
    SelectObject(hdcPage2, hbmPage2);

    // Initialize skin
    if (!hbmSkin) {
		InitializeSkin(hdc);
	}

	InitializeCanvas();

    CBR(ReleaseDC(hWnd, hdc));

Error:
    ASSERT(SUCCEEDED(hr));
}

COLORREF GetSkinRGB(int index) {
	COLORREF c = GetPixel(hdcSkin, index * vga, 
		SKIN_COLORS_Y_OFFSET * vga);
	return c;
}

void InitializeSkin(HDC hdc) {
    HBITMAP hbmSkinFile = SHLoadImageFile(pSettings->skin_path);

	BITMAP bmp;
	GetObject(hbmSkinFile, sizeof(bmp), &bmp);

    // Load skin
	HGDIOBJ hTmpOld = SelectObject(hdcTmp, hbmSkinFile);

	// Create skin bitmap
	hbmSkin = CreateCompatibleBitmap(hdc,
		DEFAULT_SCREEN_WIDTH * vga, 
		DEFAULT_SKIN_HEIGHT * vga);

	hdcSkin = CreateCompatibleDC(hdc);
	SelectObject(hdcSkin, hbmSkin);

	// Stretch skin to properly fit the screen
	StretchBlt(hdcSkin, 0, 0, DEFAULT_SCREEN_WIDTH * vga, 
		DEFAULT_SKIN_HEIGHT * vga,
		hdcTmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

	// Restore the original hdcTmp bitmap
	SelectObject(hdcTmp, hTmpOld);
}

void InitializeCanvas() {

    int nScreenWidth = ::GetDeviceCaps(hdcSkin, HORZRES);
	int nScreenHeight = ::GetDeviceCaps(hdcSkin, VERTRES);
	
    if (hdcCanvas)
        DeleteDC(hdcCanvas);
    
	hdcCanvas = CreateCompatibleDC(hdcSkin);
    
	if (hbmCanvas)
        DeleteObject(hbmCanvas);
    
	hbmCanvas = CreateCompatibleBitmap(hdcSkin,
		nScreenWidth, DEFAULT_ITEM_HEIGHT * vga);

    SelectObject(hdcCanvas, hbmCanvas);

	int textureWidth = DEFAULT_SCREEN_WIDTH * vga;
    int textureHeight = SKIN_CANVAS_HEIGHT * vga;

    BitBlt(hdcCanvas, 0, 0, textureWidth, textureHeight,
        hdcSkin, 0, SKIN_CANVAS_Y_OFFSET * vga, SRCCOPY);

    // copy the texture to the full screen width (if in landscape mode)
    if (textureWidth < nScreenWidth) {
        BitBlt(hdcCanvas, textureWidth, 0, 
            nScreenWidth - textureWidth, textureHeight,
            hdcCanvas, 0, 0, SRCCOPY);
    }

    // copy the texture to the full DEFAULT_ITEM_HEIGHT * vga
	// several BitBlt's is faster than one StretchBlt
    for (int i = textureHeight; i < DEFAULT_ITEM_HEIGHT * vga; i += i) {
        int h = i + i > DEFAULT_ITEM_HEIGHT * vga 
			? DEFAULT_ITEM_HEIGHT * vga - i 
			: i;
        BitBlt(hdcCanvas, 0, i, nScreenWidth, h, hdcCanvas, 0, 0, SRCCOPY);
    }
}

void CalculateHeights() {
	int c = 0;

    TCHAR letter[2];
    TCHAR * pdest;
    int index;
    DataItem dItem;

    letter[1] = 0;
    int count = GetItemCount();

    nKeyboardLetters = _tcslen(pSettings->alphabet);
    bool bAutoAlphabet = nKeyboardLetters == 0;

    StringCchCopy(alphabet, ALPHABET_MAX_SIZE, pSettings->alphabet);

    for (int i = 0; i < ALPHABET_MAX_SIZE; i++) {
        GroupPosition[i] = -1;
    }

    if (count == 0) {
        ListHeight = maxScrolled = 0;
        AverageItemHeight = DEFAULT_ITEM_HEIGHT * vga;
        return;
    }

    for (int i = 0; i < count; i++) {
        StartPosition[i] = c;

        int h = DEFAULT_ITEM_HEIGHT * vga;

        dItem = GetItem(i);
        if (NULL != Screens[History[depth].screen].fnGetGroup
            && IsItemNewGroup(i) && dItem.iGroup) {

            h += DEFAULT_GROUP_HEIGHT * vga;

            Screens[History[depth].screen].fnGetGroup(&dItem, letter, 2, pSettings);

            pdest = _tcsstr(pSettings->alphabet, letter);
            index = (int)(pdest - pSettings->alphabet);
            if (index >= 0 && index < ALPHABET_MAX_SIZE) {
                GroupPosition[index] = c;
            }
            else if (bAutoAlphabet && nKeyboardLetters < ALPHABET_MAX_SIZE) {
                GroupPosition[nKeyboardLetters] = c;
                alphabet[nKeyboardLetters] = letter[0];
                nKeyboardLetters++;
            }
        }

        c += h;
    }

    if (GroupPosition[0] == -1)
        GroupPosition[0] = 0;

    for (int i = 1; i < ALPHABET_MAX_SIZE; i++) {
        if (GroupPosition[i] == -1)
            GroupPosition[i] = GroupPosition[i-1];
    }

    StartPosition[count] = c;

	ListHeight = c;
    int cListHeight = Screens[History[depth].screen].hasMenus
        ? rListHeight : rContent.bottom - rContent.top;
    maxScrolled = ListHeight > cListHeight ? ListHeight - cListHeight : 0;

    AverageItemHeight = ListHeight / count;
}

int GetPixelToItem(int y) {
    y = min(ListHeight - 1, y);
    y = max(0, y);

    // estimate based on DEFAULT_ITEM_HEIGHT * vga
    int guess = y / AverageItemHeight;
    int max = GetItemCount();
    if (guess > max)
        guess = max;

    while (y < StartPosition[guess] && guess > 0) {guess--;}

    while (y >= StartPosition[guess+1] && guess < max) {guess++;}

    return guess;
}

void ScrollBar(int y) {
	bScrolling = true;
	Velocity = 20;

    // if "Contacts", scroll by chunks of A,B,C,etc.
    if (nCurrentTab == 2) {
        int index = (y - rList.top) * nKeyboardLetters / rListHeight;
        History[depth].scrolled = index < 0 ? 0
            : index >= nKeyboardLetters ? maxScrolled
            : GroupPosition[index];
    }

    // otherwise, just do a normal scroll
    else {
        double pct = (double)(y - rList.top) 
            / (double)rListHeight;
	    History[depth].scrolled = (int)(maxScrolled * pct);
    }

    History[depth].scrolled = min(History[depth].scrolled, maxScrolled);
    History[depth].scrolled = max(History[depth].scrolled, 0);
}

void ScrollTo(HWND hWnd, int position, int duration) {
    if (position < minScrolled)
        position = minScrolled;
    if (position > maxScrolled)
        position = maxScrolled;

    Scroll_StartPosition = History[depth].scrolled;
	Scroll_Change = position - Scroll_StartPosition;
	Scroll_Duration = duration;
	Scroll_StartTime = ::GetTickCount();
	Scroll_TimeCounter = 0;
	SetTimer(hWnd, IDT_TIMER_SCROLL_TO, REFRESH_RATE, NULL);
}

void StartTransition(HWND hWnd, TransitionType tr, int duration) {
    if (duration == 0)
        return;

    bTransitioning = true;
    nTransitionDuration = duration;
    dTransitionPct = 0.0;
    trTransitionType = tr;

    // Save a snapshot of the current screen
    BitBlt(hdcPage1, rScreen.left, rScreen.top, 
        rScreen.right - rScreen.left, 
        rScreen.bottom - rScreen.top, 
        hdcMem, rScreen.left, rScreen.top, SRCCOPY);

    if (tr == ttSlideLeft || tr == ttSlideRight) {
        DrawScreenOn(hdcPage2, rScreen, hdcTmp, History[depth].scrolled);
    }
    else if (tr == ttKeyboardExpand || tr == ttKeyboardShrink) {
        DrawKeyboardOn(hdcPage2, rScreen);
    }

    dwTransitionStart = ::GetTickCount();

    InvalidateRect(hWnd, &rScreen, FALSE);

	SetTimer(hWnd, IDT_TIMER_TRANSITION, REFRESH_RATE, NULL);
}

void ParseCommandLine(HWND hWnd, LPTSTR lpCmdLine) {
    const struct CmdLineArg cmdLineArgs[] = {
        TEXT("-favorites"), CMD_GOTO_FAVORITES,
        TEXT("-recents"), CMD_GOTO_RECENTS,
        TEXT("-contacts"), CMD_GOTO_CONTACTS,
        TEXT("-dialer"), CMD_GOTO_DIALER,
        TEXT("-search"), CMD_GOTO_SEARCH,
    };

    if (_tcslen(lpCmdLine) == 0)
        return;

    for (int i = 0; i < ARRAYSIZE(cmdLineArgs); i++) {
        if (_tcsstr(lpCmdLine, cmdLineArgs[i].arg) != NULL) {
            PostMessage(hWnd, WM_COMMAND, cmdLineArgs[i].wparam, NULL);
            break;
        }
    }

    if (_tcsstr(lpCmdLine, TEXT("-add"))) {
        PostMessage(hWnd, WM_COMMAND, CMD_ADD, NULL);
    }
}

void CalculateClickRegion(POINT p) {
    // They clicked on the popup, no matter what screen
    if (bDisplayingPopup) {
    }

    // They clicked in the bottom menus
    else if (Screens[History[depth].screen].hasMenus 
        && PtInRect(&rMenubar, p)) {
        rClickRegion = rMenubar;
    }

    // "back" button in header bar
    else if (depth > 0 && p.y >= rHeader.top 
        && p.y < rHeader.top + HEADER_CLICK_HEIGHT * vga 
        && p.x <= HEADER_CLICK_HEIGHT * vga
        && Screens[History[depth].screen].parent >= 0) {

        rClickRegion.top = rHeader.top;
        rClickRegion.bottom = rHeader.top + HEADER_CLICK_HEIGHT * vga;
        rClickRegion.left = 0;
        rClickRegion.right = HEADER_CLICK_HEIGHT * vga;
    }

    // "+" button in header bar
    // --or--
    // "*" button in header bar
    else if (
        (Screens[History[depth].screen].fnAdd != NULL
        || Screens[History[depth].screen].fnToggleFavorite != NULL)
        && p.y >= rHeader.top 
        && p.y < rHeader.top + HEADER_CLICK_HEIGHT * vga 
        && p.x >= rList.right - HEADER_CLICK_HEIGHT * vga) {

        rClickRegion.top = rHeader.top;
        rClickRegion.bottom = rHeader.top + HEADER_CLICK_HEIGHT * vga;
        rClickRegion.left = rList.right - HEADER_CLICK_HEIGHT * vga;
        rClickRegion.right = rList.right;
    }
    
    // They clicked the service title
    else if (PtInRect(&rServiceTitle, p) && hasiDialerServices) {
        rClickRegion = rServiceTitle;
	}

    // They clicked in the titlebar
    // no matter what the screen type is
	else if (PtInRect(&rTitlebar, p)) {
        rClickRegion = rTitlebar;
	}

    // Clicked a list item
    else if (p.y >= rList.top) {
        int offsety = History[depth].scrolled - rList.top;
        int listy = p.y + offsety;

        int nItem = GetPixelToItem(listy);
        if (!CanSelectItem(nItem))
            goto NONCLICKABLE;

        rClickRegion.top = max(rList.top, StartPosition[nItem] - offsety);
        rClickRegion.bottom = StartPosition[nItem + 1] - offsety;
        if (Screens[History[depth].screen].hasMenus)
            rClickRegion.bottom = min(rClickRegion.bottom, rMenubar.top);
        rClickRegion.left = rList.left;
        rClickRegion.right = rList.right;

        if (p.y < rClickRegion.top || p.y > rClickRegion.bottom)
            goto NONCLICKABLE;
    }

    // Clicked on a non-clickable region
    else {
        goto NONCLICKABLE;
    }

    return;

NONCLICKABLE:
    rClickRegion.top = -1;
    rClickRegion.bottom = -1;
}

void GetIDialerServiceName() {
    TCHAR value[128] = {0};
    TCHAR key[32] = {0};

    LoadSetting(value, 128, SZ_IDIALER_REG_KEY, SERVICE_NUM, NULL);

    if (value[0] == 0) {
        // no iDialer setting found
        szWindowTitle[0] = 0;
        return;
    }

    int nService = _wtoi(value);

    StringCchPrintf(key, 32, SERVICE_TITLE_FORMAT, nService + 1);
    LoadSetting(value, 64, SZ_IDIALER_REG_KEY, key);
    StringCchCopy(szWindowTitle, 64, value);
}

void NextIDialerService() {
    TCHAR value[128] = {0};
    TCHAR key[32] = {0};

    LoadSetting(value, 128, SZ_IDIALER_REG_KEY, SERVICE_NUM, NULL);

    if (value[0] == 0) {
        // no iDialer setting found
        szWindowTitle[0] = 0;
        return;
    }

    // +1 because we want the next one
    int nService = _wtoi(value) + 1;

    StringCchPrintf(key, 32, SERVICE_TYPE_FORMAT, nService + 1);
    LoadSetting(value, 64, SZ_IDIALER_REG_KEY, key);

    // the next service doesn't exist, so drop down to 1
    if (value[0] == 0)
        nService = 0;

    StringCchPrintf(value, 128, TEXT("%d"), nService);
    SaveSetting(SZ_IDIALER_REG_KEY, value, SERVICE_NUM);
}

bool HasMultipleIDialerServices() {
    TCHAR value[128] = {0};
    TCHAR key[32] = {0};

    // If "service2type" exists, then we know they have > 1 iDialer services
    StringCchPrintf(key, 32, SERVICE_TYPE_FORMAT, 2);
    LoadSetting(value, 128, SZ_IDIALER_REG_KEY, key, NULL);

    return value[0] != 0;
}