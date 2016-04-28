#include "stdafx.h"
#include "publicFunction.h"
#include "HttpRequest.h"

/*****************
��������:ConnectRemoteServer
����:����Զ������
����:
[in] server:����IP��������
[in] port���˿�
����ֵ���ɹ����ش�����SOCKET�����򷵻�INVALID_SOCKET
********************/
SOCKET ConnectRemoteServer(const char *server,const unsigned short port)
{
	short nProtocolPort;
	SOCKADDR_IN sockAddr;
	SOCKET hServerSocket = INVALID_SOCKET;
	char ip[64] = {0};
	// If the user input is an alpha name for the host, use gethostbyname()
	// If not, get host by addr (assume IPv4)
	sprintf(ip, "%s", CHttpRequest::GetHostIp(server));

	if(strlen(ip) > 0)
	{
		if((hServerSocket = socket(PF_INET, SOCK_STREAM,0)) != INVALID_SOCKET)
		{
			if(port != NULL)
				nProtocolPort = htons(port);
			
			sockAddr.sin_family = AF_INET;
			sockAddr.sin_port = nProtocolPort;
			sockAddr.sin_addr.s_addr = inet_addr(ip);/**((LPIN_ADDR)*lpHostEnt->h_addr_list);*/
			if(connect(hServerSocket,(PSOCKADDR)&sockAddr,sizeof(sockAddr)) == SOCKET_ERROR)
			{
				closesocket(hServerSocket);
				//m_oError = CSMTP_WSA_CONNECT;
				hServerSocket = INVALID_SOCKET;
			}
		}
		else
		{
			//m_oError = CSMTP_WSA_INVALID_SOCKET;
			return INVALID_SOCKET;
		}
	}
	else
	{
	//	m_oError = CSMTP_WSA_GETHOSTBY_NAME_ADDR;
		return INVALID_SOCKET;
	}
	return hServerSocket;
}
/*****************
��������:ConnectRemoteServer
����:����Զ������,���Զ��峬ʱֵ
����:
[in] addr:����IP����������ADDR
[in] port���˿�
[in] dwTimeOut����ʱֵ���ա��������Ӷ��������ʱֵ
����ֵ���ɹ����ش�����SOCKET�����򷵻�INVALID_SOCKET
********************/
SOCKET ConnectRemoteServer(unsigned long addr, const unsigned short port, DWORD dwTimeOut)
{
	SOCKET hServerSocket = INVALID_SOCKET;
	hServerSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	
	if (hServerSocket == INVALID_SOCKET)
	{
		return hServerSocket;
	}
	DWORD dwTimeOutSet = dwTimeOut*1000; //���ó�ʱ
	if(setsockopt(hServerSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&dwTimeOutSet,sizeof(dwTimeOutSet))==SOCKET_ERROR)
	{
		shutdown(hServerSocket,SD_BOTH);
		closesocket(hServerSocket);
		hServerSocket = INVALID_SOCKET;
		return hServerSocket;
	}
	if(setsockopt(hServerSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&dwTimeOutSet,sizeof(dwTimeOutSet))==SOCKET_ERROR)
	{
		shutdown(hServerSocket,SD_BOTH);
		closesocket(hServerSocket);
		hServerSocket = INVALID_SOCKET;
		return hServerSocket;
	}
	//�󶨵�ַ
	SOCKADDR_IN remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(port);
	remoteAddr.sin_addr.s_addr = addr;
	
	//���÷�������ʽ����
	unsigned long ul = 1;
	int ret = ioctlsocket(hServerSocket, FIONBIO, (unsigned long*)&ul);
	if (ret < 0)
	{
		shutdown(hServerSocket,SD_BOTH);
		closesocket(hServerSocket);
		hServerSocket = INVALID_SOCKET;
		return hServerSocket;
	}
	
	//����
	connect(hServerSocket,(const struct sockaddr *)&remoteAddr,sizeof(remoteAddr));
	
	struct timeval timeout ;
	fd_set r; 
	
	FD_ZERO(&r);
	FD_SET(hServerSocket,&r);
	timeout.tv_sec = dwTimeOut; 
	timeout.tv_usec =0;
	ret = select(0, 0,&r, 0,&timeout);
	if ( ret<= 0 ) 
	{
		shutdown(hServerSocket,SD_BOTH);
		closesocket(hServerSocket);
		hServerSocket = INVALID_SOCKET;
		return hServerSocket;
	}
	
	//���������ģʽ
	unsigned long ul1= 0 ;
	ret = ioctlsocket(hServerSocket, FIONBIO, (unsigned long*)&ul1);
	if(ret==SOCKET_ERROR)
	{
		shutdown(hServerSocket,SD_BOTH);
		closesocket (hServerSocket);
		hServerSocket = INVALID_SOCKET;   
		return hServerSocket;
	}
	return hServerSocket;
}
//����ת������
CString GetWebDataEx(const char* lpszHost, UINT nPort, BOOL bSSL, BOOL bOnlyHeader, CString strDataSend, CString* pStrRealHost/*=NULL*/, CString* pStrRealDir/*=NULL*/)
{
	CString strRet;
	if (NULL == lpszHost)
		return strRet;
	int start = -1, end = -1, iFind = -1;
	//��ȡ���������(GET XXX HTTP/1.1)
	start = strDataSend.Find(" ");
	end = strDataSend.Find(" ", start + 1);
	if (-1 == start || -1 == end)
		return strRet;
	CString strRequestPage = strDataSend.Mid(start + 1, end - start - 1);
	iFind = strRequestPage.Find("://");
	if (iFind != -1)
	{
		iFind = strRequestPage.Find("/", iFind + strlen("://"));
		if (iFind != -1)
			strRequestPage = strRequestPage.Right(strRequestPage.GetLength() - iFind);
	}
	strRet = GetWebDataPublic(lpszHost, nPort, bSSL, bOnlyHeader, strDataSend);
	int nTry = 3; //֧��3����ת
	CString strNewHost = lpszHost;
	CString strRealDir = "/"; //������ַ��Ŀ¼
	CString strNewPage;
	CString strHostSec;
	strHostSec.Format("Host: %s\r\n", lpszHost);
	while (strRet.GetLength() > 0 && CHttpRequest::GetServerState(strRet) != 200 && nTry-- > 0)
	{
		start = -1;
		end = -1;
		iFind = strRet.Find("\r\n\r\n");
		if (-1 == iFind)
			break;
		CString strHeader = strRet.Left(iFind + 4);
		strHeader.MakeUpper();
		
		start = strHeader.Find("LOCATION");
		end = strHeader.Find("\r\n", start + 8);
		if (start == -1 || end == -1)
			break;
		CString strLocal = strRet.Mid(start + strlen("LOCATION"), end - start - strlen("LOCATION"));
		strLocal.Remove(' ');
		strLocal.TrimLeft(':');
		
		if (strLocal.Find("//") == -1)
		{
			if (strLocal.Left(1).CompareNoCase("/") != 0)
				strLocal.Insert(0,"/");
			iFind = strLocal.ReverseFind('.');
			if (-1 == iFind && strLocal.Right(1).CompareNoCase("/") != 0)//û��"."��������һ�����/xx Ҫ�ں������"/"
				strLocal += "/";
			strNewPage = strLocal;
			iFind = strLocal.ReverseFind('/');
			strLocal = strLocal.Left(iFind);
			strRealDir = strLocal;
			
			//strNewPage = strLocal + strRequestPage;
		}
		else
		{
			//ȡHOST
			if (strLocal.Find("https://") != -1)
			{
				nPort = 443;
				bSSL = TRUE;
			}
			start = strLocal.Find("//");
			end = strLocal.Find("/", start + 2);
			strNewHost = strLocal.Mid(start + 2, end - start -2);
			iFind = strLocal.Find("://");
			if (iFind != -1)
			{
				iFind = strLocal.Find("/", iFind + strlen("://"));
				if (iFind != -1)
					strLocal = strLocal.Right(strLocal.GetLength() - iFind);
				iFind = strLocal.ReverseFind('.');
				if (-1 == iFind && strLocal.Right(1).CompareNoCase("/") != 0)//û��"."��������һ�����/xx Ҫ�ں������"/"
					strLocal += "/";
				iFind = strLocal.ReverseFind('/');
				strLocal = strLocal.Left(iFind);
				strRealDir = strLocal;
				strNewPage = strLocal + strRequestPage;
			}
		}
		//�滻�������ݣ���һ���϶���"GET/POST XXXXX\r\n"
		BOOL bGet = TRUE;
		if (strDataSend.Left(4).CompareNoCase("POST") == 0)
			bGet = FALSE;

		CString strRequest;
		if (bGet)
			strRequest.Format("GET %s HTTP/1.1\r\n", strNewPage);
		else
			strRequest.Format("POST %s HTTP/1.1\r\n", strNewPage);
		iFind = strDataSend.Find("\r\n");
		if (-1 == iFind)
			break;
		strDataSend = strDataSend.Right(strDataSend.GetLength() - iFind - 2);
		strDataSend = strRequest + strDataSend;
		CString strNewHostSec;
		strNewHostSec.Format("Host: %s\r\n", strNewHost);
		strDataSend.Replace(strHostSec, strNewHostSec);
		strHostSec = strNewHostSec;
		strRet = GetWebDataPublic(strNewHost.GetBuffer(0), nPort, bSSL, bOnlyHeader, strDataSend);
		strNewHost.ReleaseBuffer();
	}
	if (strRet.GetLength() < 1024)
	{
		CString strTemp = strRet;
		strTemp.MakeLower();
		if ((start = strTemp.Find("<meta http-equiv=\"refresh\" content=\"")) != -1) //ҳ��ˢ��ת��
		{
			end = strTemp.Find("\"", start + strlen("<meta http-equiv=\"refresh\" content=\""));
			strTemp = strRet.Mid(start + strlen("<meta http-equiv=\"refresh\" content=\""), end - start - strlen("<meta http-equiv=\"refresh\" content=\""));
			strTemp.MakeLower();
			start = strTemp.Find("url=");
			if (-1 != start)
			{
				strTemp = strTemp.Right(strTemp.GetLength() - start - 4);
				CString strLocal = strTemp;
				//ȡHOST
				start = strLocal.Find("//");
				end = strLocal.Find("/", start + 2);
				strNewHost = strLocal.Mid(start + 2, end - start -2);
				iFind = strLocal.Find("://");
				if (iFind != -1)
				{
					iFind = strLocal.Find("/", iFind + strlen("://"));
					if (iFind != -1)
						strLocal = strLocal.Right(strLocal.GetLength() - iFind);
					iFind = strLocal.ReverseFind('.');
					if (-1 == iFind && strLocal.Right(1).CompareNoCase("/") != 0)//û��"."��������һ�����/xx Ҫ�ں������"/"
						strLocal += "/";
					iFind = strLocal.ReverseFind('/');
					strLocal = strLocal.Left(iFind);
					strRealDir = strLocal;
					strNewPage = strLocal + strRequestPage;
				}
				//�滻�������ݣ���һ���϶���"GET/POST XXXXX\r\n"
				BOOL bGet = TRUE;
				if (strDataSend.Left(4).CompareNoCase("POST") == 0)
					bGet = FALSE;
				
				CString strRequest;
				if (bGet)
					strRequest.Format("GET %s HTTP/1.1\r\n", strNewPage);
				else
					strRequest.Format("POST %s HTTP/1.1\r\n", strNewPage);
				iFind = strDataSend.Find("\r\n");
				if (-1 != iFind)
				{
					strDataSend = strDataSend.Right(strDataSend.GetLength() - iFind - 2);
					strDataSend = strRequest + strDataSend;
					CString strNewHostSec;
					strNewHostSec.Format("Host: %s\r\n", strNewHost);
					strDataSend.Replace(strHostSec, strNewHostSec);
					strHostSec = strNewHostSec;
					strRet = GetWebDataPublic(strNewHost.GetBuffer(0), nPort, bSSL, bOnlyHeader, strDataSend);
					strNewHost.ReleaseBuffer();			
				}
			}
		}
	}
	if (NULL != pStrRealHost)
		*pStrRealHost = strNewHost;
	if (NULL != pStrRealDir)
	{
		if (strRealDir.Right(1).CompareNoCase("/") != 0)
			strRealDir += "/";
		*pStrRealDir = strRealDir;
	}
	return strRet;
}
CString GetWebDataPublic(const char* lpszHost, UINT nPort, BOOL bSSL, BOOL bOnlyHeader, CString strDataSend)
{
	CString strRet;
	if (NULL == lpszHost)
		return strRet;

	CHttpRequest httpReq(lpszHost,nPort,PLUGIN_TIME_OUT);
	if (!httpReq.Connect(bSSL))
	{
		httpReq.Disconnect();
		if (!httpReq.Connect(bSSL))
		{
			return strRet;
		}
	}
	
	if (!httpReq.SendData(strDataSend.GetBuffer(0), strDataSend.GetLength()))
	{
		strDataSend.ReleaseBuffer();
		return strRet;
	}
	strDataSend.ReleaseBuffer();
	long totalSize = 10240;
	char* pAllData = (char*)calloc(1, totalSize);
	if (NULL == pAllData)
	{
		return strRet;
	}
	int  ret = 0;
	long readBytes = 0;
	char bufrecv[8192] = {0};
	while ((ret = httpReq.RecvHttpContent(bufrecv,8192)) > 0)
	{
		if (readBytes + ret >= totalSize)
		{
			totalSize = readBytes + ret + 10240;
			char *pAllocTemp = (char*)realloc(pAllData, totalSize);
			if (NULL == pAllocTemp)
				break;
			pAllData = pAllocTemp;
		}
		memcpy(pAllData + readBytes, bufrecv, ret);
		readBytes += ret;
		memset(bufrecv, 0, sizeof (bufrecv));
		if (bOnlyHeader)
		{
			pAllData[readBytes] = 0x00;
			CString strTemp = pAllData;
			if (-1 != strTemp.Find("\r\n\r\n"))
			{
				break;
			}
		}
	}
	
	if (pAllData != NULL)
	{
		pAllData[readBytes] = 0x00; //realloc�����ȳ�ʼ��
		strRet = pAllData;
		free(pAllData);
		pAllData = NULL;
		if (bOnlyHeader)
		{
			int iFind = strRet.Find("\r\n\r\n");
			if (-1 != iFind)
				strRet = strRet.Left(iFind + 4);
		}
	}
	return strRet;
}