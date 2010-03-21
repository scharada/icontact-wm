#include <pimstore.h>

#include <windows.h>
#include <aygshell.h>
#include "resourceppc.h"

BOOL InitPoom(HWND hwnd);
void ShutdownPoom();
BOOL GetPoomFolder(int nFolder, IFolder ** ppFolder);
BOOL GetPoomApp(IPOutlookApp **ppOutApp);
