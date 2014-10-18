// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: MainFrm.cpp 613 2008-09-14 18:47:53Z dglane001 $
#include "stdafx.h"
#include "Brimstone.h"

#include "BrimstoneDoc.h"
#include "BrimstoneView.h"
#include "PrescriptionToolbar.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

//////////////////////////////////////////////////////////////////////////////
CMainFrame::CMainFrame()
{
}

//////////////////////////////////////////////////////////////////////////////
CMainFrame::~CMainFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

//////////////////////////////////////////////////////////////////////////////
int 
	CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndPrescToolBar.Create(this, IDD_PRESCDLG, 
		CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_GRIPPER | CBRS_TOOLTIPS, 
		IDC_PRESCTOOLBAR))
	{
		TRACE0("Failed to create mainbar\n");
		return -1;      // fail to create
	}

	// called to initial update of dialog bar (binds controls)
	m_wndPrescToolBar.UpdateData(0);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	EnableDocking(CBRS_ALIGN_ANY);

	m_wndPrescToolBar.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	DockControlBar(&m_wndPrescToolBar, AFX_IDW_DOCKBAR_BOTTOM);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
void 
	CMainFrame::ActivateFrame(int nCmdShow) 
{
	CView *pView = GetActiveView();
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CBrimstoneView)));
	m_wndPrescToolBar.SetView((CBrimstoneView *) pView);

	CFrameWnd::ActivateFrame(nCmdShow);
}
