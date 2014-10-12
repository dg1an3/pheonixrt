// WarpTPS.h : main header file for the WARPTPS application
//

#if !defined(AFX_WARPTPS_H__E19073EE_7E93_423F_A3A1_FEEFA838EFE3__INCLUDED_)
#define AFX_WARPTPS_H__E19073EE_7E93_423F_A3A1_FEEFA838EFE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CWarpTPSApp:
// See WarpTPS.cpp for the implementation of this class
//

class CWarpTPSApp : public CWinApp
{
public:
	CWarpTPSApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWarpTPSApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CWarpTPSApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WARPTPS_H__E19073EE_7E93_423F_A3A1_FEEFA838EFE3__INCLUDED_)
