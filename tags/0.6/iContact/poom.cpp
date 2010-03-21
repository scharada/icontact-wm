// Includes
#include <stdafx.h>
#include <initguid.h>
#include <pimstore.h>
#include "poom.h"
#include "iContact.h"

// Globals
IPOutlookApp * g_polApp = NULL;


BOOL InitPoom(HWND hwnd)
{
	BOOL bSuccess = FALSE;
	
	if (SUCCEEDED(CoInitializeEx( NULL, 0)))
	{
		// Now, let's get the main outlook application
		   if (SUCCEEDED(CoCreateInstance(CLSID_Application, NULL, CLSCTX_INPROC_SERVER, 
			IID_IPOutlookApp, reinterpret_cast<void **>(&g_polApp))))
		   {
	
			// login to the Pocket Outlook object model
			if(SUCCEEDED(g_polApp->Logon(NULL)))
			{
				// can't login to the app
				bSuccess = TRUE;
			}
		}

	}	
	return bSuccess;
}

// **************************************************************************
// Function Name: GetPoomFolder
// 
// Purpose: Gets the default folder for the specified POOM item type
//
// Arguments:
//    IN int nFolder - POOM folder type to return, olFolderTasks,
//		olFolderContacts, or olFolderCalendar in this app
//	  OUT IFolder ** ppFolder - pointer to IFolder interface returned. 
//			Must be released by caller of GetPoomFolder
//
// Return Values:
//    BOOL
//    returns TRUE if the folder interface was retrieved, FALSE otherwise
// 
// Description:  
//  This function simply encapsulates a call on the global Outlook app interface. The 
//	returned pointer is simply passed through this function.
BOOL GetPoomFolder(int nFolder,   IFolder ** ppFolder)
{
	if (SUCCEEDED(g_polApp->GetDefaultFolder(nFolder, ppFolder)))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// **************************************************************************
// Function Name: GetPoomApp
// 
// Purpose: Gets pointer to POOM application interface
//
// Arguments:
//	  OUT IFolder ** ppFolder - pointer to IPOutlookApp interface returned. 
//			Must be released by caller of GetPoomApp
//
// Return Values:
//    BOOL
//    returns TRUE if the folder interface was retrieved, FALSE otherwise
// 
// Description:  
//  This function simply returns a pointer to the global Outlook app interface 
//	The returned pointer is simply passed through this function after being AddRef'd.
BOOL GetPoomApp(IPOutlookApp **ppOutApp)
{
	if (g_polApp)
	{
		g_polApp->AddRef();
		*ppOutApp = g_polApp;
		return TRUE;
	}
	else 
	{
		return FALSE;
	}
}

void ShutdownPoom()
{
	if (g_polApp)
	{
		g_polApp->Logoff();
		g_polApp->Release();
	}
	CoUninitialize();
}