// iContact.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "iContact.h"
#include "Settings.h"
#include "ListData.h"
#include "ListDataCallLog.h"
#include "ListDataPoom.h"
#include "GraphicFunctions.h"
#include "PhoneUtils.h"
#include "Titlebar.h"

//TODO: fix memory leak when updating contact pictures

//-----------------------------------------------------------------------------
// Global data
//
HINSTANCE   hInst;                     // Program instance handle

Settings *  pSettings = NULL;
ListData *  pListData = NULL;

ScreenType  stScreenType = stList;
int         nCurrentTab = 2;

int		    ListHeight = 0;
int         AverageItemHeight = DEFAULT_ITEM_HEIGHT;
int         StartPosition[MAX_LIST_ITEMS];
int         GroupPosition[ALPHABET_MAX_SIZE];
POINT       ptMouseDown = { -1, -1 };

// Graphic
RECT		rScreen;
int         nScreenHeight;
RECT        rTitlebar;
RECT        rHeader;
RECT		rMenubar;
RECT		rList;
RECT        rContent;
int         rListHeight;

HFONT		PrimaryListFont;
HFONT		SecondaryListFont;
HFONT		GroupFont;
HFONT		ItemDetailsFont;
HFONT		ListIndicatorFont;
HFONT		KeyboardFont;

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

// Scrolling
bool		bDragging = false;
bool		bScrolling = false;
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
TransitionType trTransitionType = ttSlideLeft;

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
    WM_PAINT, DoPaintMain,
    WM_DESTROY, DoDestroyMain,
    WM_ACTIVATE, DoActivate,
    WM_SIZE, DoSize,
    WM_LBUTTONDOWN, DoLButtonDown,
    WM_MOUSEMOVE, DoMouseMove,
    WM_LBUTTONUP, DoLButtonUp,
    WM_TIMER, DoTimer,
    WM_KEYDOWN, DoKeyDown,
    WM_TITLEBAR, DoTitlebarCallback,
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

    // Initialize titlebar callbacks
    InitTitlebar(hWnd);

    // Create fonts
	PrimaryListFont = BuildFont(ITEM_FONT_SIZE, FALSE, FALSE);
	SecondaryListFont = BuildFont(ITEM_SECONDARY_FONT_SIZE, TRUE, FALSE);
	ItemDetailsFont = BuildFont(ITEM_DETAILS_FONT_SIZE, FALSE, FALSE);
	GroupFont = BuildFont(GROUP_ITEM_FONT_SIZE, TRUE, FALSE);
    ListIndicatorFont = BuildFont(LIST_INDICATOR_FONT_SIZE, TRUE, FALSE);
	KeyboardFont = BuildFont(KEYBOARD_FONT_SIZE, TRUE, FALSE);

    // Create data lists
    pSettings = new Settings();
    SwitchTab(hWnd, 2);
    InitSurface(hWnd);

    // Standard show and update calls
    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);
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
    for (i = 0; i < dim(MainMessages); i++) {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoPaintMain - Process WM_PAINT message for window.
//
LRESULT DoPaintMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                     LPARAM lParam) {

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
            if (trTransitionType == ttSlideLeft)
                width1 = rScreenWidth - width1;
            int width2 = rScreenWidth - width1;

            BitBlt(hdcMem, rContent.left, rContent.top, width1, rHeight, 
                hdcPage1, width2, rContent.top, SRCCOPY);

            BitBlt(hdcMem, width1, rContent.top, width2, rHeight,
                hdcPage2, rContent.left, rContent.top, SRCCOPY);

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
    }

    // not transitioning
    else {
        DrawScreenOn(hdcMem, stScreenType, hdcTmp, rect, Scrolled);

        if (bDisplayingPopup) {
            switch (ptPopupType) {
                case ptKeyboard:
	                // draw the keyboard
	                DrawKeyboardOn(hdcTmp, rScreen);
                    BltAlpha(hdcMem, rScreen.left, rScreen.top, rScreenWidth, rScreenHeight,
	                    hdcTmp, 0, 0, rScreenWidth, rScreenHeight, 220);
                    break;
            }
        }
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
LRESULT DoActivate (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {

    DWORD dwState;
    RECT rc;

    if (wParam == WA_CLICKACTIVE || wParam == WA_ACTIVE) {
        // To switch to full screen mode, first hide all of the shell parts.
        dwState = (SHFS_HIDETASKBAR 
            | SHFS_HIDESTARTICON 
            | SHFS_HIDESIPBUTTON);

        SHFullScreen(hWnd, dwState);

        // Next resize the main window to the size of the screen.
        SetRect(&rc, 0, 0, GetSystemMetrics(SM_CXSCREEN), 
            GetSystemMetrics(SM_CYSCREEN));
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
LRESULT DoSize (HWND hWnd, UINT wMsg, WPARAM wParam,
                LPARAM lParam) {

	InitSurface(hWnd);

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoLButtonDown - Process WM_LBUTTONDOWN message for window
//
LRESULT DoLButtonDown (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

	LastX = ptMouseDown.x = LOWORD(lParam);
	LastY = ptMouseDown.y = HIWORD(lParam);
    tStartTime = ::GetTickCount();

    if (bScrolling) {
        KillTimer(hWnd, IDT_TIMER_SCROLL);
        KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
        Velocity = 0;
        bScrolling = false;
    }

    if (bDisplayingPopup || bTransitioning) {
        return 0;
    }

    if (stScreenType == stList 
        && LastX > rList.right - 20 
        && LastY >= rHeader.bottom 
        && LastY < rMenubar.top) {

        bScrolling = true;
        ScrollBar(LastY);
        InvalidateRect(hWnd, &rList, false);
        UpdateWindow(hWnd);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DoMouseMove - Process WM_MOUSEMOVE message for window
//
LRESULT DoMouseMove (HWND hWnd, UINT wMsg, WPARAM wParam,
                     LPARAM lParam) {

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
    int t = ::GetTickCount();

    if (bDisplayingPopup || bTransitioning) {
        return 0;
    }

    if (stScreenType == stList && ptMouseDown.y >= rMenubar.top) {
        return 0;
    }

    if (ptMouseDown.y < rHeader.bottom) {
        return 0;
    }

    // "back" button in header bar
    //TODO: back to categories from stScreenType == stList
    if (ptMouseDown.y < rHeader.top + HEADER_CLICK_HEIGHT 
        && ptMouseDown.x <= HEADER_CLICK_HEIGHT
        && stScreenType == stDetails) {

        return 0;
    }

    // "+" button in header bar, or * in detail view
    if (ptMouseDown.y < rHeader.top + HEADER_CLICK_HEIGHT 
        && ptMouseDown.x >= rList.right - HEADER_CLICK_HEIGHT 
        && (
            stScreenType == stList && pListData->CanAdd()
            || stScreenType == stDetails && pListData->CanFavorite()
        ) ) {

        return 0;
    }

	if (bScrolling) {
        ScrollBar(y);
        InvalidateRect(hWnd, &rList, false);
        UpdateWindow(hWnd);
		return 0;
	}

	if (abs(y - ptMouseDown.y) > SCROLL_THRESHOLD) {
        if (stScreenType == stList)
            pListData->UnselectItem();
        else if (stScreenType == stDetails)
            pListData->SelectDetail(-1);
        bDragging = true;
	}

    // SCROLL
    Scrolled = Scrolled - y + LastY;
	LastY = y;
	LastX = x;

    Velocity = (double)(LastY - ptMouseDown.y) / (t - tStartTime);
    tEndTime = t;

    InvalidateRect(hWnd, 
        (stScreenType == stDetails ? &rContent : &rList), false);

    UpdateWindow(hWnd);

	return 0;
}

//-----------------------------------------------------------------------------
// DoLButtonUp - Process WM_LBUTTONUP message for window
//
LRESULT DoLButtonUp (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

    POINT pt;

    if (bTransitioning) {
        return 0;
    }

    if (bScrolling) {
        bScrolling = false;
        InvalidateRect(hWnd, &rList, FALSE);
        UpdateWindow(hWnd);
        return 0;
    }

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
    
    int minScroll = 0;
    int maxScroll = ListHeight > rListHeight ? ListHeight - rListHeight : 0;


    // They clicked on the popup, no matter what screen
    if (bDisplayingPopup) {
        StartTransition(hWnd, ttKeyboardShrink, SHRINK_KEYBOARD_PERIOD);

        int keyboardIndex = (pt.y / GroupHeight) 
            * (rScreen.right - rScreen.left) / GroupWidth 
            + (pt.x - rScreen.left) / GroupWidth;
        if (keyboardIndex < nKeyboardLetters) {
            pListData->UnselectItem();
            ScrollTo(hWnd, GroupPosition[keyboardIndex]);
        }
    }


    // They clicked in the titlebar
    // no matter what the screen type is
	else if (PtInRect(&rTitlebar, ptMouseDown) && PtInRect(&rTitlebar, pt)) {
        if (pSettings->doExitOnMinimize) {
            DestroyWindow(hWnd);
        }
        else {
            ShowWindow(hWnd, SW_MINIMIZE);
        }
	}


    // They clicked in the bottom menus (in list screen)
    else if (stScreenType == stList 
        && PtInRect(&rMenubar, ptMouseDown) && PtInRect(&rMenubar, pt)) {
        int tabWidth = (rMenubar.right - rMenubar.left) / 5;

        if (pt.x < tabWidth * 3) {
            SwitchTab(hWnd, pt.x / tabWidth);
        }

        else if (pt.x < tabWidth * 4) { // Dialer
            RunDialer();
            if (pSettings->doExitOnAction)
                DestroyWindow(hWnd);
        }

        //else { // Voicemail
        //    CallVmail();
        //}
        else { // Keyboard
            if (nCurrentTab == 2) {
                StartTransition(hWnd, ttKeyboardExpand, EXPAND_KEYBOARD_PERIOD);
            }
        }

        InvalidateRect(hWnd, &rMenubar, FALSE);
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
        SetTimer(hWnd, IDT_TIMER_SCROLL, 
            REFRESH_RATE, (TIMERPROC) NULL);
        bScrolling = true;
        bDragging = false;
        return 0;
    } 

    else {
        int pos = 0;

        // This is the normal *click* event
        switch (stScreenType) {
            case stList:

                // "back" button in header bar
                if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HEADER_CLICK_HEIGHT 
                    && pt.x <= HEADER_CLICK_HEIGHT) {

                    //TODO: back to categories
                }

                // "+" button in header bar
                else if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HEADER_CLICK_HEIGHT 
                    && pt.x >= rList.right - HEADER_CLICK_HEIGHT 
                    && pListData->CanAdd()) {

			        pListData->AddItem();

                    // This will refresh the data in the current tab
                    SwitchTab(hWnd, nCurrentTab);
		        }

                // Clicked a list item
                else if (pt.y >= rList.top) {
                    pos = pt.y + Scrolled - rList.top;

                    pListData->SelectItem(GetPixelToItem(pos));
                    StartTransition(hWnd, ttSlideLeft, EXPAND_DETAILS_PERIOD);
                }
                break;

            case stDetails:

                // "back" button in header bar
                if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HEADER_CLICK_HEIGHT 
                    && pt.x <= HEADER_CLICK_HEIGHT) {

                    // Back to list
                    StartTransition(hWnd, ttSlideRight, EXPAND_DETAILS_PERIOD);
                }

                // "*" button in header bar
                else if (pt.y >= rHeader.top 
                    && pt.y < rHeader.top + HEADER_CLICK_HEIGHT 
                    && pt.x >= rList.right - HEADER_CLICK_HEIGHT 
                    && pListData->CanFavorite()) {

                    pListData->ToggleFavorite();
                    pListData->PopulateDetails();
                    pListData->GetItem(pListData->GetCurrentItemIndex());
                    InvalidateRect(hWnd, &rHeader, false);
		        }

                // Clicked a list item
                else if (pt.y >= rList.top) {
			        // HANDLE SUBLIST EVENTS
                    pos = pt.y + Scrolled - rContent.top;
                    int subListIndex = pos / DEFAULT_ITEM_HEIGHT;
                    if (!pListData->SelectDetail(subListIndex))
                        break;

                    InvalidateRect(hWnd, &rContent, false);
                    UpdateWindow(hWnd);

                    // TODO: is this the best way to send SMS?
                    int column = pt.x > rList.right - 40 ? 2 : 1;

                    HRESULT hr = pListData->PerformCurrentDetailAction(column);

                    if (SUCCEEDED(hr)) {
                        if (pSettings->doExitOnAction)
                            DestroyWindow(hWnd);
                        else
                            pListData->PopulateDetails();
                    }
                    else {
                        SwitchTab(hWnd, nCurrentTab);
                    }
    		        InvalidateRect(hWnd, &rScreen, FALSE);
		        }
		        pListData->SelectDetail(-1);

	            break;
        }
    }

    UpdateWindow(hWnd);

	return 0;
}

//-----------------------------------------------------------------------------
// DoTimer - Process WM_TIMER message for window
//
LRESULT DoTimer (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {

    DWORD t = ::GetTickCount();
    DWORD dt = 0;
    double s = 0.0;

    int minScrolled = 0;
    int maxScrolled = ListHeight <= rListHeight ? 0 : ListHeight - rListHeight;

	switch (wParam)	{

        ///// TIMER for scrolling
	    case IDT_TIMER_SCROLL:

            // Time
            dt = t - tEndTime;

            // Velocity
            if (Scrolled < minScrolled)
                Velocity = (double)(Scrolled - minScrolled) / 2 / dt;
            else if (Scrolled > maxScrolled)
                Velocity = (double)(Scrolled - maxScrolled) / 2 / dt;
            else {
                double dv = Velocity * FRICTION_COEFF * dt;
                if (fabs(dv) > fabs(Velocity)) 
                    Velocity = 0;
                else 
			        Velocity = Velocity - dv;
            }

            // Displacement
            s = Velocity * dt;
            if (s < 0 && s > -1 && Scrolled < minScrolled)
                s = -1;
            else if (s > 0 && s < 1 && Scrolled > maxScrolled)
                s = 1;
            
            // We're done scrolling
            if ((int)s == 0) {
                KillTimer(hWnd, IDT_TIMER_SCROLL);
		        bScrolling = false;
		        Velocity = 0;
            }

            Scrolled = Scrolled - (int)s;
            tEndTime = t;
            if (stScreenType == stList)
                InvalidateRect(hWnd, &rList, false);
            else
                InvalidateRect(hWnd, &rContent, false);
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
                Scrolled = Scroll_StartPosition + (int)amount;
            }
            else {
                bScrolling = false;
                Velocity = 0;
                KillTimer(hWnd, IDT_TIMER_SCROLL_TO);
                Scrolled = Scroll_Change + Scroll_StartPosition;
            }
            if (stScreenType == stList)
                InvalidateRect(hWnd, &rList, false);
            else
                InvalidateRect(hWnd, &rContent, false);
            break;

        case IDT_TIMER_TRANSITION:
            dTransitionPct = (double)(t - dwTransitionStart) 
                / nTransitionDuration;

            if (dTransitionPct >= 1.0) {
                dTransitionPct = 1.0;
                bTransitioning = false;

                if (trTransitionType == ttSlideRight) {
                    stScreenType = stList;
                    Scrolled = ListScrolled;
                    pListData->UnselectItem();
                    CalculateHeights();
                }
                else if (trTransitionType == ttSlideLeft) {
                    stScreenType = stDetails;
                    CalculateHeights();
                }
                else if (trTransitionType == ttKeyboardExpand) {
                    bDisplayingPopup = true;
                }
                else if (trTransitionType == ttKeyboardShrink) {
                    bDisplayingPopup = false;
                }

                KillTimer(hWnd, IDT_TIMER_TRANSITION);
            }
            InvalidateRect(hWnd, &rScreen, false);
            break;

        case IDT_TIMER_LOADLIST:
            pListData = 
                // Favorites
                nCurrentTab == 0 ? (ListData *)new ListDataPoom(pSettings, true)

                // Call Log
                : nCurrentTab == 1 ? (ListData *)new ListDataCallLog(pSettings)

                // Contacts
                : (ListData *)new ListDataPoom(pSettings);

            CalculateHeights();
            InvalidateRect(hWnd, &rHeader, false);
            KillTimer(hWnd, IDT_TIMER_LOADLIST);
            Scrolled = -rListHeight;
            ScrollTo(hWnd, 0);
            break;
	} 

	UpdateWindow(hWnd);
	return 0;
}

//-----------------------------------------------------------------------------
// DoKeyDown - Process WM_KEYDOWN message for window
//
LRESULT DoKeyDown (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {
    int top = 0;
    int bot = 0;
    bool bRepeating = (lParam & (1 << 30)) != 0;
    int index;

    if (bTransitioning) {
        return 0;
    }

	switch (stScreenType) {

		case stList:
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
                    pListData->SelectPreviousItem(
                        GetPixelToItem(Scrolled + rListHeight), bRepeating);

					// make sure the selected item is visible
                    index = pListData->GetCurrentItemIndex();
					top = StartPosition[index];
					bot = StartPosition[index + 1];

                    if (bScrolling) {
                        Scrolled = max(0, min(StartPosition[index], 
                            ListHeight - rListHeight));
                    }
					else if (!bScrolling && (top < Scrolled 
                        || bot > Scrolled + rListHeight)) {

                        Scrolled = max(0, bot - rListHeight);
                    }
    
					break;

			    case VK_DOWN:
                    pListData->SelectNextItem(
                        GetPixelToItem(Scrolled), bRepeating);

					// make sure the selected item is visible
                    index = pListData->GetCurrentItemIndex();
					top = StartPosition[index];
					bot = StartPosition[index + 1];

                    if (bScrolling) {
                        Scrolled = max(0, min(StartPosition[index], 
                            ListHeight - rListHeight));
                    }
                    else if (!bScrolling && (top < Scrolled 
                        || bot > Scrolled + rListHeight)) {
                            Scrolled = min(top, ListHeight - rListHeight);
                    }
    
					break;

                case VK_LEFT:
                    if (nCurrentTab > 0)
                        SwitchTab(hWnd, nCurrentTab - 1);
                    break;

                case VK_RIGHT:
                    if (nCurrentTab < 2)
                        SwitchTab(hWnd, nCurrentTab + 1);
                    break;

			    case VK_TACTION:
					if (pListData->GetCurrentItemIndex() >= 0)
                        StartTransition(hWnd, ttSlideLeft, 
                            EXPAND_DETAILS_PERIOD);
					break;

			}
            InvalidateRect(hWnd, &rList, FALSE);
			break;


		case stDetails:
			switch (wParam) {
                case VK_BACK:
				case VK_LEFT:
					pListData->SelectDetail(-1);
                    StartTransition(hWnd, ttSlideRight);
					break;

				case VK_UP:
					pListData->IncrementDetailIndex(-1);
					pListData->IncrementDetailIndex(-1);
                    // there's no "break" here on purpose, code optimization
				case VK_DOWN:
					pListData->IncrementDetailIndex(1);
                    top = DEFAULT_ITEM_HEIGHT 
                        * pListData->GetCurrentDetailIndex();
                    bot = top + DEFAULT_ITEM_HEIGHT;
                    if (top - Scrolled < 0)
                        ScrollTo(hWnd, max(0, 
                            rContent.top - rContent.bottom + bot));
                    if (bot - Scrolled > rContent.bottom - rContent.top)
                        ScrollTo(hWnd, max(
                            ListHeight - rContent.bottom + rContent.top, 
                            top));
					break;

                case VK_RIGHT:
			    case VK_TACTION:
                    HRESULT hr = pListData->PerformCurrentDetailAction(
                        wParam == VK_RIGHT ? 2 : 1);

                    if (SUCCEEDED(hr)) {
                        pListData->PopulateDetails();
                        if (pSettings->doExitOnAction)
                            DestroyWindow(hWnd);
                    }
					break;

			}
            InvalidateRect(hWnd, &rContent, FALSE);
			break;
	}
	UpdateWindow(hWnd);

    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// DoDestroyMain - Process WM_DESTROY message for window
//
LRESULT DoDestroyMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                       LPARAM lParam) {
   
    // Uninitialize the COM classes
    CoUninitialize();

    // Quit
    PostQuitMessage (0);
    return 0;
}

//-----------------------------------------------------------------------------
// Screen Drawing Functions
//
void DrawScreenOn(HDC hdc, ScreenType st, HDC hdcTmp, 
    RECT rClip, int yListOffset) {

    // MAIN CONTENT
    switch (st) {
        case stList:
            if (rClip.bottom > rList.top || rClip.top < rList.bottom)
                DrawListOn(hdc, hdcTmp, rList, yListOffset);

            // MENU BAR
            if (rClip.bottom > rMenubar.top) {
                int rMenubarWidth = rMenubar.right - rMenubar.left;
                int numItems = nCurrentTab == 2 ? 5 : 4;

                // draw the background of the menu bar
                // This will stretch the first column of the menu bar
                // fully across the screen
                StretchBlt(hdc, rMenubar.left, rMenubar.top, 
                    rMenubarWidth, rMenubar.bottom - rMenubar.top,
                    hdcSkin, 0, MENU_BAR_Y_OFFSET, 1, MENU_BAR_HEIGHT, SRCCOPY);

                // draw buttons
                for (int i = 0; i < numItems; i++) {
                    int xdest = rMenubar.left 
                        + rMenubarWidth / 10 * (2 * i + 1) - 24;
                    int ydest = rMenubar.top;
                    int xsrc = i * 48;
                    int ysrc = i == nCurrentTab 
                        ? MENU_BAR_SELECTED_Y_OFFSET 
                        : MENU_BAR_Y_OFFSET;
                    StretchBlt(hdc, xdest, ydest, 48, MENU_BAR_HEIGHT, 
                        hdcSkin, xsrc, ysrc, 48, MENU_BAR_HEIGHT,
                        SRCCOPY);
                }
            }
            break;
        case stDetails:
            DrawItemDetailsOn(hdc, pListData->GetCurrentItem(), yListOffset);
            break;
    }

    // TITLE BAR
    if (rClip.top < rTitlebar.bottom)
	    DrawTitlebarOn(hdc, rTitlebar, hdcSkin, 
            pSettings->rgbTitlebarBackground, pSettings->rgbTitlebarText,
            pSettings->rgbTitlebarSignal, pSettings->rgbTitlebarBattery,
            pSettings->rgbTitlebarBatteryCharge);

    // HEADER BAR
    DrawHeaderOn(hdc, st, rHeader, hdcSkin);
}

void DrawListOn(HDC hdc, HDC hdcTmp, RECT rList, int yOffset) {
	int nFirstItem, nItem;
    Data dItemTmp;
    TCHAR buffer[16];
    int count = NULL == pListData ? 0 : pListData->GetItemCount();

    nFirstItem = nItem = yOffset < 0 ? 0 : GetPixelToItem(yOffset);

    RECT rItem;
    rItem = rList;

    rItem.bottom = rList.top + StartPosition[nItem] - yOffset;

	// ******* DRAW LIST BACKGROUND
	FillRect(hdc, &rList, pSettings->hbrListBackground);
	SetBkMode(hdc, TRANSPARENT);

    // "About" at bottom of screen
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_LEFT);
    SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
    ExtTextOut(hdc, rList.left + 2, rList.bottom - 15, 
        NULL, NULL, SZ_ABOUT, ABOUT_LENGTH, 0);

    if (count == 0) {
        //TODO: maybe indicate empty list?
        return;
    }

    // ******* DRAW LIST ITEMS
	while (nItem < count && rItem.bottom < rList.bottom) {
		dItemTmp = pListData->GetItem(nItem);

        rItem.top = rItem.bottom;
        rItem.bottom = rItem.top + StartPosition[nItem+1] 
            - StartPosition[nItem];
        
        if (nItem == pListData->GetCurrentItemIndex() 
            && stScreenType == stList) {
            DrawItemSelectedOn(hdc, dItemTmp, rItem);
		}
        else {
            DrawItemOn(hdc, dItemTmp, rItem);
		}

        // ****** Group Header
		if (stScreenType == stList
			&& pListData->IsItemNewGroup(nItem)
            && dItemTmp.wcGroup && rItem.top >= rList.top) {

            DrawGroupHeaderOn(hdc, nItem, rItem);
		} 

        // Next nItem
        nItem++;
	}

    // Special: Draw the group of the list nItem that's at the top of the list
    dItemTmp = pListData->GetItem(nFirstItem);
    if (dItemTmp.wcGroup && yOffset >= 0) {
        RECT rTopGroup = {rList.left, 0, rList.right, DEFAULT_GROUP_HEIGHT};
        DrawGroupHeaderOn(hdcTmp, nFirstItem, rTopGroup);

        int nHeight = DEFAULT_GROUP_HEIGHT;
        int nBottom = nHeight;

        if (pListData->IsItemNewGroup(nFirstItem + 1)) {
            nBottom = min(nBottom, StartPosition[nFirstItem + 1] - yOffset);
        }

        // account for the fact that the list
        // doesn't start at the top of the screen
        nBottom += rList.top;

        int nLeft = rList.left;
        int nWidth = rList.right - rList.left;
        int nTop = nBottom - nHeight;

        if (bScrolling && pSettings->doFastGraphics) {
            BitBlt(hdc, nLeft, nTop, nWidth, nHeight, hdcTmp, 0, 0, SRCCOPY);
        }
        else {
            BltAlpha(hdc, nLeft, nTop, nWidth, nHeight, hdcTmp, 200);
        }
    }

    // Draw list indicator if scrolling quickly
	if (bScrolling) {
		SelectObject(hdc, ListIndicatorFont);
	    SetTextAlign(hdc, TA_CENTER);
		SetTextColor(hdc, pSettings->rgbListGroupText);
		SetBkMode(hdc, TRANSPARENT);

        pListData->GetItemGroup(nFirstItem, buffer);
        int length = _tcslen(buffer);

        ExtTextOut(hdc, (rList.right - rList.left) / 2 + rList.left, 
            rList.top + 10, NULL, NULL, buffer, length, 0);
	}
}

void DrawGroupHeaderOn(HDC hdc, int index, RECT rItem) {
    RECT rHeader = rItem;
    rHeader.bottom = rHeader.top + DEFAULT_GROUP_HEIGHT;
    TCHAR buffer[10];

    pListData->GetItemGroup(index, buffer);
    int length = _tcslen(buffer);

	// ****** GroupHeader background
	FillRect(hdc, &rHeader, pSettings->hbrListGroupBackground);

    // separator
    RECT rSep = rHeader;
    rSep.top = rHeader.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

	// ******* Draw Group Header Text
	SelectObject(hdc, GroupFont);
   	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, pSettings->rgbListGroupText);
	ExtTextOut(hdc, rItem.left + LIST_GROUP_ITEM_INDENT, 
        rHeader.top - 1 + ((DEFAULT_GROUP_HEIGHT - GROUP_ITEM_FONT_SIZE) / 2),
        NULL, NULL, buffer, length, 0);
}

void DrawItemSelectedOn(HDC hdc, Data dItem, RECT rItem) {
	// ******* DRAW ITEM BACKGROUND
	DrawGradientGDI(hdc, rItem,
        pSettings->rgbListItemSelectedBackground1,
        pSettings->rgbListItemSelectedBackground2);

	// ****** Draw Item Text
	SelectObject(hdc, PrimaryListFont);
	SetTextAlign(hdc, TA_LEFT);

    // Item Shadow Text
	SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

    // Item Text
	SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT, 
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);
}

void DrawItemOn(HDC hdc, Data dItem, RECT rItem) {
    // Item Background
    FillRect(hdc, &rItem, pSettings->hbrListItemBackground);

    // separator
    RECT rSep = rItem;
    rSep.top = rItem.bottom - LIST_SEPARATOR_HEIGHT;
    FillRect(hdc, &rSep, pSettings->hbrListItemSeparator);
	SetTextAlign(hdc, TA_LEFT);

    // Item Primary Text
	SelectObject(hdc, PrimaryListFont);
	SetTextColor(hdc, dItem.rgbPrimaryText);
	ExtTextOut(hdc, rItem.left + LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_FONT_SIZE) / 2),
        ETO_OPAQUE, NULL, dItem.szPrimaryText, dItem.nPrimaryTextLength, 0);

	// Item Secondary Text
    if (dItem.nSecondaryTextLength == 0)
        return;
    SelectObject(hdc, SecondaryListFont);
    SetTextAlign(hdc, TA_RIGHT);
    SetTextColor(hdc, pSettings->rgbListItemText);
    ExtTextOut(hdc, rItem.right - LIST_ITEM_INDENT,
        rItem.bottom - 2 - ((DEFAULT_ITEM_HEIGHT + ITEM_SECONDARY_FONT_SIZE) / 2),
	    ETO_OPAQUE, NULL, dItem.szSecondaryText, dItem.nSecondaryTextLength, 0);

}

void DrawItemDetailsOn(HDC hdc, Data dItem, int yOffset) {
    int padding = 10;
    int nameHeight = 0;
    int nPictureSize = 0;
    DataDetail dd = {0};
    RECT rRow, rClip;

    SelectObject(hdc, PrimaryListFont);
    SetTextAlign(hdc, TA_LEFT);
    SetBkMode(hdc, TRANSPARENT);

    int iItemCount = pListData->GetItemDetailCount();
    int iCurrentIndex = pListData->GetCurrentDetailIndex();
    int iBitmapHeight = 0;
    int iBitmapWidth = 0;

	// ******* DRAW ITEM BACKGROUND
	DrawGradientGDI(hdc, rContent,
        pSettings->rgbListItemSelectedBackground1,
        pSettings->rgbListItemSelectedBackground2);

    // ******* Draw the current item's picture, if it exists
    HBITMAP hBitmap = pListData->GetHBitmap();
    if (NULL != hBitmap) {
        iBitmapHeight = pListData->GetHBitmapHeight();
        iBitmapWidth = pListData->GetHBitmapWidth();

        int left = padding;
        int top = (DEFAULT_ITEM_HEIGHT * 2 - iBitmapHeight) / 2 + rContent.top - yOffset;
        int right = left + iBitmapWidth;
        int bottom = top + iBitmapHeight;

        // draw black square with a black border of 1
        RECT rcBitmap = { left - 1, top - 1, right + 1, bottom + 1};
        DrawRect(hdc, &rcBitmap, (COLORREF)0);

        // paint the bitmap on the DC
        TransparentImage(hdc, left, top, iBitmapWidth, iBitmapHeight, 
            hBitmap, 0, 0, iBitmapWidth, iBitmapHeight, (COLORREF)0);
    }

    // ******* Now, draw the item data
	for (int c = 0; c < iItemCount; c++) {
        dd = pListData->GetItemDetail(c);

        if (dd.type == diNothing)
            continue;

        rRow.top = c * DEFAULT_ITEM_HEIGHT + rContent.top - yOffset;
        rRow.bottom = rRow.top + DEFAULT_ITEM_HEIGHT;
        if (dd.type == diName)
            rRow.bottom += DEFAULT_ITEM_HEIGHT;

        rRow.left = rContent.left;

        if (rRow.top + yOffset < iBitmapHeight + rContent.top + padding / 2 
            && iBitmapWidth > 0)

            rRow.left += iBitmapWidth + padding;

        rRow.right = rContent.right;

        rClip.left = rRow.left + padding;
        rClip.right = rRow.right - padding;
        rClip.top = rRow.top + padding / 2 + 1;
        rClip.bottom = rRow.bottom - padding / 2 - 1;

        // If this is the first item of a new group of items, draw a rectangle
        // around the group
        if ((dd.type == diPhone 
            || dd.type == diEmail 
            || dd.type == diUrl
            || dd.type == diDetailsButton 
            || dd.type == diCallButton
            || dd.type == diSmsButton
            || dd.type == diEditButton
            || dd.type == diSaveContactButton) 
            && c > 0 
            && pListData->GetItemDetail(c-1).type != dd.type) {

            int nRows = 1;
            while (c + nRows < iItemCount 
                && pListData->GetItemDetail(c + nRows).type == dd.type)
                nRows++;

            SelectObject(hdc, pSettings->hbrListItemBackground);
            SetTextColor(hdc, pSettings->rgbListItemText);
            RoundRect(hdc, rRow.left + 6, rRow.top + 3, 
                rRow.right - 6, 
                rRow.bottom - 3 + DEFAULT_ITEM_HEIGHT * (nRows - 1),
                padding, padding);
        }

        if (rRow.bottom < 0)
            continue;

        // Indicate the currently selected item
	    if (c == iCurrentIndex) {
            SelectObject(hdc, pSettings->hbrListItemSelectedBackground);
            RoundRect(hdc, rRow.left + 6, rRow.top + 3, 
                rRow.right - 6, rRow.bottom - 3, padding, padding);
            SetTextColor(hdc, pSettings->rgbListItemSelectedText);
	    }
        else {
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
    	    SetTextColor(hdc, pSettings->rgbListItemText);
        }

        if (dd.type == diName) {
            SelectObject(hdc, PrimaryListFont);
            SetTextAlign(hdc, TA_LEFT | TA_TOP);

            // Display the shadow
            SetTextColor(hdc, pSettings->rgbListItemSelectedShadow);
		    ExtTextOut(hdc, rRow.left + padding + 1, rRow.top + padding + 1,
                ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), NULL);

            // Display the name
            SetTextColor(hdc, pSettings->rgbListItemSelectedText);
		    ExtTextOut(hdc, rRow.left + padding, rRow.top + padding,
                ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), NULL);
        }

        else if (dd.type == diCompany) {
            SelectObject(hdc, SecondaryListFont);
            SetTextColor(hdc, pSettings->rgbListItemSelectedText);
		    ExtTextOut(hdc, rRow.left + padding, rRow.top,
                NULL, NULL, dd.text, _tcslen(dd.text), NULL);
        }

        else if (dd.type == diDetailsButton 
            || dd.type == diEditButton
            || dd.type == diCallButton
            || dd.type == diSmsButton
            || dd.type == diSaveContactButton) {
		    
            SelectObject(hdc, PrimaryListFont);
            SetTextAlign(hdc, TA_CENTER);

            ExtTextOut(hdc, (rRow.right - rRow.left) / 2 + rRow.left,
                (rRow.bottom - rRow.top - ITEM_FONT_SIZE) / 2 + rRow.top,
                ETO_CLIPPED, &rClip, dd.text,
                _tcslen(dd.text), 0);

        }

        else if (dd.type == diEmail
                || dd.type == diUrl) {
            SelectObject(hdc, PrimaryListFont);
            SetTextAlign(hdc, TA_CENTER);

            ExtTextOut(hdc, (rRow.right - rRow.left) / 2 + rRow.left,
                (rRow.bottom - rRow.top - ITEM_FONT_SIZE) / 2 + rRow.top,
                ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), 0);
        }

        else {
		    SelectObject(hdc, ItemDetailsFont);
            SetTextAlign(hdc, TA_RIGHT | TA_BOTTOM);
            int y = (rRow.bottom - rRow.top) / 2 + rRow.top;

            rClip.right = rRow.left + 49;
            ExtTextOut(hdc, rRow.left + 49, 
                y + (ITEM_DETAILS_FONT_SIZE / 2),
                ETO_CLIPPED, &rClip, dd.label, _tcslen(dd.label), NULL);

            rClip.left = rClip.right + 3;
            rClip.right = rRow.right - padding;

            if (dd.type == diPhone) {
                rClip.right -= 28;
                ExtTextOut(hdc, rRow.right - 12, 
                    y + (ITEM_DETAILS_FONT_SIZE / 2),
                    NULL, NULL, pSettings->sms_string, 
                    _tcslen(pSettings->sms_string), NULL);
            }

            SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
            SelectObject(hdc, PrimaryListFont);

		    ExtTextOut(hdc, rRow.left + 52, y + (ITEM_FONT_SIZE / 2),
                ETO_CLIPPED, &rClip, dd.text, _tcslen(dd.text), NULL);
        }
	}
}

void DrawKeyboardOn(HDC hdc, RECT rKeyboard) {
	SelectObject(hdc, KeyboardFont);
	SelectObject(hdc, pSettings->hpenKeyboardGrid);
    SetTextColor(hdc, pSettings->rgbKeyboardText);
	SetBkMode(hdc, TRANSPARENT);
    int x, y, g, h;

    FillRect(hdc, &rKeyboard, pSettings->hbrKeyboardBackground);

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
            + ((GroupHeight - KEYBOARD_FONT_SIZE) / 2) 
            + (GroupHeight * h);

        for (g = 0; g < nKeyboardCols; g++) {
            x = rKeyboard.left + (GroupWidth / 2) + (GroupWidth * g);

            if (i < nKeyboardLetters) {
                ExtTextOut(hdc, x, y, NULL, NULL, &alphabet[i++], 1, 0);
            }
		}
	}
}

void DrawHeaderOn(HDC hdc, ScreenType st, RECT rHeader, HDC hdcSkin) {

    // The background of the header bar
    StretchBlt(hdc, rHeader.left, rHeader.top, 
        rHeader.right - rHeader.left, rHeader.bottom - rHeader.top,
        hdcSkin, 0, HEADER_Y_OFFSET, 1, HEADER_HEIGHT, SRCCOPY);

    if (!bTransitioning) {
        // The "back" button
        if (st == stDetails) {
            BitBlt(hdc, rHeader.left, rHeader.top, rHeader.left + 26, 
                rHeader.bottom - rHeader.top,
                hdcSkin, 0, HEADER_Y_OFFSET, SRCCOPY);
        }

        // The "+" to add a contact
        if (st == stList && pListData != NULL && pListData->CanAdd()) {
            BitBlt(hdc, rHeader.right - 40, rHeader.top, 40, 
                rHeader.bottom - rHeader.top,
                hdcSkin, 192, HEADER_Y_OFFSET, SRCCOPY);
        }

        // The "favorite YES" icon
        if (st == stDetails && pListData->CanFavorite()) {
            Data dItem = pListData->GetCurrentItem();
            if (dItem.isFavorite) {
                BitBlt(hdc, rHeader.right - 40, rHeader.top, 40, 
                    rHeader.bottom - rHeader.top,
                    hdcSkin, 144, HEADER_Y_OFFSET, SRCCOPY);
            }

            // The "favorite NO" icon
            else {
                BitBlt(hdc, rHeader.right - 40, rHeader.top, 40, 
                    rHeader.bottom - rHeader.top,
                    hdcSkin, 96, HEADER_Y_OFFSET, SRCCOPY);
            }
        }
    }

    // The title
    SelectObject(hdc, PrimaryListFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextAlign(hdc, TA_LEFT);

    if (st == stList) {
        SetTextColor(hdc, 
            NULL == pListData 
            ? pSettings->rgbHeaderLoading
            : pSettings->rgbHeader
        );

        DrawText(hdc, 
            ( nCurrentTab == 0 ? pSettings->favorite_category
            : nCurrentTab == 1 ? pSettings->recents_string
            : pSettings->allcontacts_string),
            -1, &rHeader, DT_CENTER | DT_VCENTER
        );
    }
    else if (st == stDetails) {
        DrawText(hdc, pListData->GetCurrentDetailTitle(),
        -1, &rHeader, DT_CENTER | DT_VCENTER);
    }
/*        DrawText(hdc, 
        ( nCurrentTab == 0 ? pSettings->favorite_category
        : nCurrentTab == 1 ? pSettings->recents_string
        : st == stList ?     pSettings->allcontacts_string
        :                    pSettings->details_string
        ),
    }
*/
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
	rTitlebar.bottom = rTitlebar.top + TITLE_BAR_HEIGHT;

    // Header, with the "back" button, the "favorite" button, etc.
    rHeader = rScreen;
    rHeader.top = rTitlebar.bottom;
    rHeader.bottom = rHeader.top + HEADER_HEIGHT;

    // Menu at the bottom of the screen
	rMenubar = rScreen;
	rMenubar.top = rMenubar.bottom - MENU_BAR_HEIGHT;

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

    // Calculate skin filename
    if (!hbmSkin) {
	    TCHAR szSkinFileName[MAX_PATH];
	    GetModuleFileName(NULL, szSkinFileName, MAX_PATH);
	    TCHAR * pstr = _tcsrchr(szSkinFileName, '\\');
	    if (pstr) *(++pstr) = '\0';
	    StringCchCat(szSkinFileName, MAX_PATH, pSettings->skin_name);
        StringCchCat(szSkinFileName, MAX_PATH, TEXT(".png"));

        hbmSkin = SHLoadImageFile(szSkinFileName);

	    // Load skin
	    hdcSkin = CreateCompatibleDC(hdc);
        SelectObject(hdcSkin, hbmSkin);
    }

    CBR(ReleaseDC(hWnd, hdc));

Error:
    ASSERT(SUCCEEDED(hr));
}

void CalculateHeights() {
	int c = 0;

    TCHAR letter[2];
    TCHAR * pdest;
    int index;

    letter[1] = 0;
    int count = 0;

    if (stScreenType == stDetails) {
        if (NULL != pListData) {
            count = pListData->GetItemDetailCount();
            c = (count - 1) * DEFAULT_ITEM_HEIGHT;
        }
    }
    else {
        if (NULL != pListData)
            count = pListData->GetItemCount();

        nKeyboardLetters = _tcslen(pSettings->alphabet);
        bool bAutoAlphabet = nKeyboardLetters == 0;

        StringCchCopy(alphabet, ALPHABET_MAX_SIZE, pSettings->alphabet);

        for (int i = 0; i < ALPHABET_MAX_SIZE; i++) {
            GroupPosition[i] = -1;
            StartPosition[i] = 0;
        }

        if (count == 0) {
            ListHeight = 0;
            AverageItemHeight = DEFAULT_ITEM_HEIGHT;
            return;
        }

	    for (int i = 0; i < count; i++) {
            StartPosition[i] = c;

            int h = DEFAULT_ITEM_HEIGHT;

            if (pListData->IsItemNewGroup(i) && pListData->GetItem(i).wcGroup) {
                h += DEFAULT_GROUP_HEIGHT;

                letter[0] = pListData->GetItem(i).wcGroup;

                pdest = wcsstr(pSettings->alphabet, letter);
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
    }

	ListHeight = c;
    AverageItemHeight = ListHeight / count;
}

int GetPixelToItem(int y) {
    y = min(ListHeight - 1, y);
    y = max(0, y);

    // estimate based on DEFAULT_ITEM_HEIGHT
    int guess = y / AverageItemHeight;
    int max = NULL == pListData ? 0 : pListData->GetItemCount();
    if (guess > max)
        guess = max;

    while (y < StartPosition[guess] && guess > 0) {guess--;}

    while (y >= StartPosition[guess+1] && guess < max) {guess++;}

    return guess;
}

void ScrollBar(int y) {
	bScrolling = true;
	Velocity = 20;
    int maxScrolled = ListHeight <= rListHeight ? 0 : ListHeight - rListHeight;

    // if "Contacts", scroll by chunks of A,B,C,etc.
    if (nCurrentTab == 2) {
        int index = (y - rList.top) * nKeyboardLetters / rListHeight;
        Scrolled = index < 0 ? 0
            : index >= nKeyboardLetters ? maxScrolled
            : GroupPosition[index];
    }

    // otherwise, just do a normal scroll
    else {
        double pct = (double)(y - rList.top) 
            / (double)rListHeight;
	    Scrolled = (int)(maxScrolled * pct);
    }

    Scrolled = min(Scrolled, maxScrolled);
    Scrolled = max(Scrolled, 0);
}

void ScrollTo(HWND hWnd, int position, int duration) {
    int minScrolled = 0;
    int maxScrolled = ListHeight <= rListHeight ? 0 : ListHeight - rListHeight;
    if (position < minScrolled)
        position = minScrolled;
    if (position > maxScrolled)
        position = maxScrolled;

    Scroll_StartPosition = Scrolled;
	Scroll_Change = position - Scroll_StartPosition;
	Scroll_Duration = duration;
	Scroll_StartTime = ::GetTickCount();
	Scroll_TimeCounter = 0;
	SetTimer(hWnd, IDT_TIMER_SCROLL_TO, REFRESH_RATE, NULL);
}

void StartTransition(HWND hWnd, TransitionType tr, int duration) {
    if (tr == ttSlideLeft)
        if (FAILED(pListData->PopulateDetails()))
            return;

    bTransitioning = true;
    nTransitionDuration = duration;
    dTransitionPct = 0.0;
    trTransitionType = tr;
    dwTransitionStart = ::GetTickCount();

    InvalidateRect(hWnd, &rScreen, FALSE);

    if (tr == ttSlideLeft || tr == ttSlideRight) {
        if (tr == ttSlideLeft) {
            ListScrolled = Scrolled;
            Scrolled = 0;
        }
        DrawScreenOn(hdcPage1, stList, hdcTmp, rScreen, ListScrolled);
        DrawScreenOn(hdcPage2, stDetails, hdcTmp, rScreen, Scrolled);
    }
    else if (tr == ttKeyboardExpand || tr == ttKeyboardShrink) {
        DrawScreenOn(hdcPage1, stList, hdcTmp, rScreen, Scrolled);
        DrawKeyboardOn(hdcPage2, rScreen);
    }

	SetTimer(hWnd, IDT_TIMER_TRANSITION, REFRESH_RATE, NULL);
}

void SwitchTab(HWND hWnd, int which) {
    if (pListData) {
        pListData->Release();
        delete pListData;
    }
    stScreenType = stList;
    nCurrentTab = which;
    Scrolled = 0;
    bScrolling = false;
    bDragging = false;
    InvalidateRect(hWnd, &rMenubar, false);
    InvalidateRect(hWnd, &rHeader, false);
    InvalidateRect(hWnd, &rList, false);

    pListData = NULL;
    SetTimer(hWnd, IDT_TIMER_LOADLIST, REFRESH_RATE, NULL);
}
