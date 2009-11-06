// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: MainFrm.h 613 2008-09-14 18:47:53Z dglane001 $
#if !defined(AFX_MAINFRM_H__E3FE8154_27D1_4055_B299_92E1770789F5__INCLUDED_)
#define AFX_MAINFRM_H__E3FE8154_27D1_4055_B299_92E1770789F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  
	// control bar embedded members
	CPrescriptionToolbar m_wndPrescToolBar;
	CStatusBar  m_wndStatusBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__E3FE8154_27D1_4055_B299_92E1770789F5__INCLUDED_)
