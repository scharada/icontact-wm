HWND CreateContactList(HWND, RECT, bool(*rePop)());
void DestroyList();
bool ClearList();
bool AddItem(int ID, WCHAR * tszTxt, bool GroupHeader, int ItemHeight, int Group, LONG oId);
void CalculateHeights();
void RedrawList();
void ResizeList(RECT r);
void GetItemData();
