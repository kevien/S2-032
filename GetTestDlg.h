// GetTestDlg.h : header file
//

#if !defined(AFX_GETTESTDLG_H__791A0580_F75F_4E68_B526_3D0DAB6AF6EF__INCLUDED_)
#define AFX_GETTESTDLG_H__791A0580_F75F_4E68_B526_3D0DAB6AF6EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CGetTestDlg dialog

class CGetTestDlg : public CDialog
{
// Construction
public:
	CGetTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CGetTestDlg)
	enum { IDD = IDD_GETTEST_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGetTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGetUrl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETTESTDLG_H__791A0580_F75F_4E68_B526_3D0DAB6AF6EF__INCLUDED_)
