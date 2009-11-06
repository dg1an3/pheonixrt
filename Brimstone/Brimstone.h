// Brimstone.h : main header file for the BRIMSTONE application
//

#if !defined(AFX_BRIMSTONE_H__A39174C8_E3F8_4153_8330_B7C58DD22FAA__INCLUDED_)
#define AFX_BRIMSTONE_H__A39174C8_E3F8_4153_8330_B7C58DD22FAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneApp:
// See Brimstone.cpp for the implementation of this class
//

class CBrimstoneApp : public CWinApp
{
public:
	CBrimstoneApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrimstoneApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CBrimstoneApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRIMSTONE_H__A39174C8_E3F8_4153_8330_B7C58DD22FAA__INCLUDED_)
