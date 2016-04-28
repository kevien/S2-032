/*
 * Bruter - Parallel network login brute forcer
 * Copyright (C) 2010 Worawit Wangwarunyoo
 * Licensed under the new-BSD (LICENSE.txt) license
 */
#include "StdAfx.h"
#include "HttpRequest.h"
#include <Iphlpapi.h>
#include "adns.h"

#pragma comment(lib,"IPHlpApi.lib")

bool CHttpRequest::isInit = false;

SSL_CTX* CHttpRequest::client_ctx = NULL;

void CHttpRequest::Startup() 
{
	if(!isInit) 
	{
		//WSADATA wsaData;
		//WSAStartup(MAKEWORD(2, 0), &wsaData);
		
		//初始化SSL环境
		SSL_METHOD *meth;
		
		// add error string for debug code
		SSL_load_error_strings();
		//int ssl
		SSLeay_add_ssl_algorithms();
		
		//set the client ssl method
		meth = (SSL_METHOD *)SSLv23_client_method();
		
		//set the client ssl CTX
		client_ctx = SSL_CTX_new(meth);
		
		if(NULL != client_ctx)
		{			
			SSL_CTX_set_verify(client_ctx, 0, NULL); // 根据自己的需要设置CTX的属性   
		}
		isInit = true;
		
	}
}

void CHttpRequest::Cleanup() 
{
	if(isInit)
	{
		//WSACleanup();
		
		if(client_ctx != NULL)
		{
			SSL_CTX_free(client_ctx);
		}
		isInit = false;
	}
}

CHttpRequest::CHttpRequest(LPCSTR addr, int port, int timeout)
{
  isInitSSL =false;
  m_HostAddr = addr;
  m_port = port;
  m_TimeOut = timeout;
  sk = INVALID_SOCKET;
  ssl = NULL;

  if (m_TimeOut == 0)
    m_TimeOut = 3;

  useSSL = false;

 // TRACE("new start \n");
}

CHttpRequest::~CHttpRequest()
{
	Disconnect();
	if (isInitSSL) {
		SSL_free(ssl);
	}

 // TRACE("end request \n");
}


void CHttpRequest::InitSSL()
{
	if (isInitSSL)
		return;

	if (ssl == NULL) {
		if ((ssl = SSL_new(client_ctx)) == NULL) {
      isInitSSL = false;
		}
	}

	isInitSSL = true;
}



/* Connect()
 *
 * Establish a connection to a defined target in ConnectionParam.
 * This function checks a ConnectionParam and connection status, 
 * then call ConnectInternal() to establish a real connection.
 *
 * If connection cannot be established, throw NetworkExcpetion() or its subclass
 */
bool CHttpRequest::Connect(bool bssl, unsigned long addr/*=0*/)
{

	if (IsConnected())
		return false;

  if (!ConnectInternal(addr))
    return false;

  if (bssl){
    useSSL = true;
    if (!ConnectSSL())
      return false;
  }

  return true;

}



/**
 * Resolve a hostname to IPv4 address
 * Returns INADDR_NONE if a hostname cannot be resolved
 */
unsigned long CHttpRequest::HostnameToAddr(const char *str)
{
	if (NULL == str)
		return INADDR_NONE;
	unsigned long addr = INADDR_NONE;
	// check if str is IP address
	addr = inet_addr(str);
	if (addr == INADDR_NONE)
	{
		CString strIP = GetHostIp(str);
		if (!strIP.IsEmpty())
		{
			addr = inet_addr(strIP.GetBuffer(0));
			strIP.ReleaseBuffer();
		}
	}
	return addr;
}

/* ConnectInternal()
 *
 * Establish a real connection with a target defined in ConnectionParam.
 * If proxy is used and protocol is TCP, then this function will connect to proxyAddr:proxyPort
 */
bool CHttpRequest::ConnectInternal(unsigned long addr)
{
  sk = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

  if (sk != INVALID_SOCKET){

    DWORD TimeOut= m_TimeOut*1000; //设置超时
    if(setsockopt(sk,SOL_SOCKET,SO_SNDTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){
	  shutdown(sk,SD_BOTH);
      closesocket(sk);
      sk = INVALID_SOCKET;
      return false;
    }
    if(setsockopt(sk,SOL_SOCKET,SO_RCVTIMEO,(char *)&TimeOut,sizeof(TimeOut))==SOCKET_ERROR){
	  shutdown(sk,SD_BOTH);
      closesocket(sk);
      sk = INVALID_SOCKET;
      return false;
    }

    //绑定地址
    SOCKADDR_IN remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons((unsigned short)m_port);
	if (0 == addr)
	{
		remoteAddr.sin_addr.s_addr = HostnameToAddr(m_HostAddr.GetBuffer(0));
		m_HostAddr.ReleaseBuffer();
	}
	else
	{
		remoteAddr.sin_addr.s_addr = addr;
	}


    //设置非阻塞方式连接
    unsigned long ul = 1;
    int ret = ioctlsocket(sk, FIONBIO, (unsigned long*)&ul);

    if (ret < 0){
	  shutdown(sk,SD_BOTH);
      closesocket(sk);
      sk = INVALID_SOCKET;
      return false;
    }

    //连接
    connect(sk,(const struct sockaddr *)&remoteAddr,sizeof(remoteAddr));

    struct timeval timeout ;
    fd_set r; 

    FD_ZERO(&r);
    FD_SET(sk,&r);
    timeout.tv_sec = m_TimeOut; 
    timeout.tv_usec =0;
    ret = select(0, 0,&r, 0,&timeout);
    if ( ret<= 0 ) {
	  shutdown(sk,SD_BOTH);
      closesocket(sk);
      sk = INVALID_SOCKET;
      return false;
    }

    //再设回阻塞模式
    unsigned long ul1= 0 ;
    ret = ioctlsocket(sk, FIONBIO, (unsigned long*)&ul1);
    if(ret==SOCKET_ERROR){
	  shutdown(sk,SD_BOTH);
      closesocket (sk);
      sk = INVALID_SOCKET;   
      return false;
    }

    return true;
  }

  return false;
}



/* ConnectSSL()
 *
 * Create SSL session in existed connection
 * Call it after connection to a target has been established
 */
bool CHttpRequest::ConnectSSL()
{

  InitSSL();

  //与服务器进行SSL握手
  SSL_set_fd (ssl, sk);
  int err = SSL_connect(ssl);

  return err != SOCKET_ERROR ? true : false;

}

void CHttpRequest::Disconnect()
{
	if (sk != INVALID_SOCKET) 
	{
		if (useSSL && NULL != ssl) 
		{
			if (SSL_shutdown(ssl) == 0)
				SSL_shutdown(ssl);
			SSL_clear(ssl);
		}
		shutdown(sk,SD_BOTH);
		closesocket(sk);
		sk = INVALID_SOCKET;
	}
}




////////////////////////////////////////////////////////////////////////
// 功能:
//  发送数据
// 输入参数:
// char *buffer --- 数据缓冲
// int len      --- 发送数据的长度
// 输出参数:
//   ---
// 返回值:
//   ---
// 特别说明:
//   start by wuying. 2010/05/17
////////////////////////////////////////////////////////////////////////
BOOL CHttpRequest::SendData(char *buffer, int len)
{
	int n = 0;
	int nread = 0;
	
	
	if (useSSL) 
	{
		n =  SSL_write(ssl, buffer, len);
	}
	else
	{
		while (n < len) 
		{
			if ((nread = send(sk, (char *)buffer + n, len - n, 0)) < 0) 
			{
				break;
			}
			
			n += nread;
		}
	}
	
	return (n == len);
}

////////////////////////////////////////////////////////////////////////
// 功能:
//  发送数据
// 输入参数:
// char *buffer --- 数据缓冲
// int len      --- 接收数据的长度
// 输出参数:
//   ---
// 返回值:
//   ---
// 特别说明:
//   start by wuying. 2010/05/17
////////////////////////////////////////////////////////////////////////
BOOL CHttpRequest::RecvHttpHeader(CString & Header)
{
  char buffer[1024+1] = {0};
  int bRet = 1;

  char c = 0;
	int nIndex = 0;
	BOOL bEndResponse = FALSE;

  while(!bEndResponse && nIndex < 1024 && bRet > 0)
  {
    if (useSSL)
	    bRet = SSL_read(ssl,&c,1);
    else
      bRet = recv(sk,&c,1,0);


    if (bRet < 0)
      return FALSE;
	  buffer[nIndex++] = c;
	  if(nIndex >= 4)
	  {
		  if(buffer[nIndex - 4] == '\r' && buffer[nIndex - 3] == '\n'
			  && buffer[nIndex - 2] == '\r' && buffer[nIndex - 1] == '\n')
		  bEndResponse = TRUE;
	  }
  }

  Header = buffer;

  return TRUE;
}

////////////////////////////////////////////////////////////////////////
// 功能:
// 接收所有数据直到完毕
// 输入参数:
// char *buffer   ---接受缓冲
// int len --- 缓冲大小
// 输出参数:
// char *szValue --- 域值
// 返回值:
//   ---
// 特别说明:
//   start by wuying. 2010/03/19
////////////////////////////////////////////////////////////////////////
BOOL CHttpRequest::RecvHttpContent(char *buffer, int len, bool bCrack/* = false*/)
{
	int nread = 0;
	int n = 0;
	
	fd_set fdReads;
	
	struct timeval tm;
	tm.tv_sec  = 1;
	tm.tv_usec = 0;
	FD_ZERO(&fdReads);
	FD_SET(sk,&fdReads);
	DWORD dwLastData = time(NULL);
	while ( n < len)
	{
		fd_set readfd = fdReads;
		int ret = select(0,&readfd,NULL,NULL,&tm);
		if (ret > 0 )
		{
			if (useSSL)
				nread = SSL_read(ssl, buffer+n, len-n);
			else
				nread = recv(sk, buffer + n, len - n, 0);
			
			if (nread > 0)
				n += nread;
			else
				break;
			if (bCrack) //破解的话一旦收到数据则马上返回，认为一次就接收完
				break;
			dwLastData = time(NULL);
		}
		else if (SOCKET_ERROR == ret)
			break;
		else if ((time(NULL) - dwLastData) >= m_TimeOut)
			break;
		Sleep(1);
	}
	return n;
}


/////////////////////////////////////////////////////
//web 隧道特殊函数
//读完头后，数据部分以回车结尾
//////////////////////////////////////////////////////
BOOL CHttpRequest::RecvHttpContentByTunnel(char *buffer,int len)
{

  int bRet = 1;

  char c = 0;
	int nIndex = 0;
	BOOL bEndResponse = FALSE;

  while(!bEndResponse && nIndex < 1024 && bRet > 0)
  {
    if (useSSL)
	    bRet = SSL_read(ssl,&c,1);
    else
      bRet = recv(sk,&c,1,0);

    if (bRet < 0)
      return FALSE;
	  buffer[nIndex++] = c;
	  if(nIndex >= 4)
	  {
		  if(buffer[nIndex - 4] == '\r' && buffer[nIndex - 3] == '\n'
			  && buffer[nIndex - 2] == '\r' && buffer[nIndex - 1] == '\n')
		  bEndResponse = TRUE;
	  }
  }

  //读取头部完成
  if (bEndResponse)
  {
    bEndResponse = FALSE;

    while(!bEndResponse && nIndex < len && bRet > 0)
    {
      if (useSSL)
	      bRet = SSL_read(ssl,&c,1);
      else
        bRet = recv(sk,&c,1,0);

      if (bRet < 0)
        return FALSE;
	    buffer[nIndex++] = c;
	    if(nIndex >= 4) {
		    if(buffer[nIndex - 2] == '\r' && buffer[nIndex - 1] == '\n')
		      bEndResponse = TRUE;
	    }
    }
  }
  return TRUE;
}

BOOL CHttpRequest::RecvHttpReadLine(char *buffer,int len)
{
  int nread;
	int n = 0;
  char c = 0;
  int nIndex = 0; 

	do {

    if (useSSL)
		  nread = SSL_read(ssl, &c,1);
    else
      nread = recv(sk, &c,1, 0);

    if (nread > 0)
		  n += nread;

    buffer[nIndex++] = c;

    if(nIndex >= 2)
		  if(buffer[nIndex - 2] == '\r' && buffer[nIndex - 1] == '\n')
		    break;

	} while (nread > 0 && n < len);

  return n;
}

////////////////////////////////////////////////////////////////////////
// 功能:
// 返回某个域值,-1表示不成功
// 输入参数:
// const char* szSession --- 域名
// int nMaxLength --- 缓冲大小
// 输出参数:
// char *szValue --- 域值
// 返回值:
//   ---
// 特别说明:
//   start by wuying. 2010/03/19
////////////////////////////////////////////////////////////////////////
int	CHttpRequest::GetField(CString & Header,const char* szSession,CString &DataValue)
{
  //取得某个域值
	if(Header.GetLength() < 12) return -1;
	
	CString strRespons;
	strRespons = Header;
	int nPos = -1;
	nPos = strRespons.Find(szSession,0);
	if(nPos != -1)
	{
		nPos += strlen(szSession);
		nPos += 2;
		int nCr = strRespons.Find("\r\n",nPos);
		DataValue = strRespons.Mid(nPos,nCr - nPos);
		return (nCr - nPos);
	}
	else	{
		return -1;
	}  
}

int	CHttpRequest::GetFieldEx(CString & Header,const char* szSession,CString &DataValue)
{
  //取得某个域值
	if(Header.GetLength() < 12) return -1;
	
	CString strRespons;
	strRespons = Header;
	int nPos = -1;
	nPos = strRespons.Find(szSession,0);
	if(nPos != -1)
	{
		nPos += strlen(szSession);
		int nCr = strRespons.Find("\r\n",nPos);
		DataValue = strRespons.Mid(nPos,nCr - nPos);
		DataValue.TrimLeft(":");
		DataValue.TrimLeft(" ");
		return (nCr - nPos);
	}
	else	{
		return -1;
	}  
}

////////////////////////////////////////////////////////////////////////
// 功能:
// 返回状态
// 输入参数:
// CString & Header  ---头部信息
// 输出参数:
//  ---
// 返回值:
//   int ---状态值 (200 300 400...) 
// 特别说明:
//   start by wuying. 2010/03/19
////////////////////////////////////////////////////////////////////////
int	CHttpRequest::GetServerState(CString & Header)					
{
	//若没有取得响应头,返回失败
	if(Header.GetLength() < 12) 
    return -1;
	
	char szState[3];
	szState[0] = Header[9];
	szState[1] = Header[10];
	szState[2] = Header[11];

	return atoi(szState);
}
CString CHttpRequest::GetDefaultDns()
{
	DWORD  returnCheckError;
    PFIXED_INFO pFixedInfo;
    DWORD FixedInfoSize = 0;
	CString strRet;
    if ((returnCheckError = GetNetworkParams(NULL, &FixedInfoSize)) != 0)
    {
        if (returnCheckError != ERROR_BUFFER_OVERFLOW)
        {
            return strRet;
        }
    }

    // Allocate memory from sizing information
    if ((pFixedInfo = (PFIXED_INFO) malloc( FixedInfoSize)) == NULL)
	{
        return strRet;
    }

    if ((returnCheckError = GetNetworkParams(pFixedInfo, &FixedInfoSize)) == 0)
    {
	   strRet =  pFixedInfo->DnsServerList.IpAddress.String;
	}
	free(pFixedInfo);
	pFixedInfo = NULL;
	return strRet;
}
CString CHttpRequest::GetHostIp(CString strHost)
{
	CString strIP;
	unsigned long addr;
	// check if str is IP address
	addr = inet_addr(strHost.GetBuffer(0));
	strHost.ReleaseBuffer();
	if (addr == INADDR_NONE)
	{
		adns_state adns;
		adns_query query;
		adns_answer *answer = NULL;
		adns_init(&adns, adns_if_noenv, 0);
		adns_submit(adns, strHost.GetBuffer(0), adns_r_a,
		    (adns_queryflags)(adns_qf_quoteok_cname|adns_qf_cname_loose),
		     NULL, &query);
		strHost.ReleaseBuffer();
		
		DWORD dwStartTime = time(NULL);
		adns_wait(adns, &query, &answer, NULL);
		if (NULL != answer)
		{
			if (answer->rrs.inaddr != NULL)
			{
				strIP = inet_ntoa(*answer->rrs.inaddr);
			}
			else if ((time(NULL) - dwStartTime) < 8)
			{
				HOSTENT *host = gethostbyname(strHost.GetBuffer(0));
				strHost.ReleaseBuffer();
				if (host != NULL)
				{ 
					strIP = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
				}  
			}
			if (!strIP.IsEmpty())
			{
				if (strIP.CompareNoCase(GetDefaultDns()) == 0)
					strIP.Empty();
			}
			free(answer);
			answer = NULL;
		}
		adns_finish(adns);
	}
	else //IP
		strIP = strHost;
	return strIP;
}

