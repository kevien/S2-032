// GetTest.h : main header file for the GETTEST application
//

#if !defined(AFX_GETTEST_H__82E8A209_7B5F_44C1_8672_7FE82A677724__INCLUDED_)
#define AFX_GETTEST_H__82E8A209_7B5F_44C1_8672_7FE82A677724__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CGetTestApp:
// See GetTest.cpp for the implementation of this class
//

class CGetTestApp : public CWinApp
{
public:
	CGetTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGetTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GETTEST_H__82E8A209_7B5F_44C1_8672_7FE82A677724__INCLUDED_)
