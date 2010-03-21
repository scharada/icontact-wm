#include <phone.h>

void Call(TCHAR * number, TCHAR * name);
void SendSMS(TCHAR * number, TCHAR * name);
void SendEMail(const TCHAR * account, TCHAR * to);
//TODO: void OpenGoogleMaps(CEOID ceoid);
void RunDialer();
void AddContactByNumber(TCHAR * pNumber);