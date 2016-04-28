/*
 * Bruter - Parallel network login brute forcer
 * Copyright (C) 2010 Worawit Wangwarunyoo
 * Licensed under the new-BSD (LICENSE.txt) license
 */

#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include "winsock2.h"
#include <openssl/ssl.h>


class CHttpRequest
{
public:
	CHttpRequest(LPCSTR addr, int port = 80, int timeout = 3); 

	~CHttpRequest();

	// call Startup() method only once before using it
	static void Startup();
	static void Cleanup();
	
	const SOCKET GetSocket() const  { return sk; }

	/*
	 * Connect()
	 *   Connect to a target defined in ConnectionParam.
	 *   If an error occurs while establishing connection,
	 *   this method throws a NetworkException.
	 *
	 * Note:
	 *   This method will throw NetworkConnectedException if
	 *   connection has been established.
	 */
	bool Connect(bool bssl = false, unsigned long addr = 0);

	/*
	 * Disconnect()
	 *   Disconnect a established connection.
	 *   If a connection was not established, it does nothing.
	 */
	void Disconnect();
	/*
	 * IsConnected()
	 *   Check whether connection has been established or not.
	 *   If ConnectionParam is provided, this method also check whether
	 *   a same target and service or not.
	 */
  bool IsConnected() const {return (sk != INVALID_SOCKET);};
	
	BOOL SendData(char *buffer, int len);

  BOOL RecvHttpHeader(CString & Header);
  BOOL RecvHttpContent(char *buffer, int len, bool bCrack = false);
  BOOL RecvHttpReadLine(char *buffer,int len);
  BOOL RecvHttpContentByTunnel(char *buffer,int len);

  static int	GetField(CString & Header,const char* szSession,CString &DataValue);
  static int	GetFieldEx(CString & Header,const char* szSession,CString &DataValue);
  static int	GetServerState(CString & Header);
  static unsigned long HostnameToAddr(const char *str);
  static CString GetHostIp(CString strHost);
  static CString GetDefaultDns();
	bool IsUsingSSL() const    { return useSSL; }

protected:

	bool ConnectInternal(unsigned long addr);
	bool ConnectSSL();
	void InitSSL();
	
protected:
	SOCKET sk;
  CString m_HostAddr;
  int m_port;
  int m_TimeOut;

	bool isInitSSL;
	bool useSSL;

	SSL *ssl;

	static bool isInit;
  static SSL_CTX* client_ctx;
};

#endif