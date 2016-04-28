#include <winsock2.h>
#ifndef PUBLICFUNCTION_H
#define PUBLICFUNCTION_H
unsigned long HostnameToAddr(const char *str);
SOCKET ConnectRemoteServer(const char *server,const unsigned short port);
SOCKET ConnectRemoteServer(unsigned long addr, const unsigned short port, DWORD dwTimeOut);
CString GetWebDataPublic(const char* lpszHost, UINT nPort, BOOL bSSL, BOOL bOnlyHeader, CString strDataSend);
//具有转向处理功能
CString GetWebDataEx(const char* lpszHost, UINT nPort, BOOL bSSL, BOOL bOnlyHeader, CString strDataSend, CString* pStrRealHost=NULL, CString* pStrRealDir=NULL);
#endif //PUBLICFUNCTION_H
