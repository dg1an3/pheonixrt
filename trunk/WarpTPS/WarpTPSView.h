// WarpTPSView.h : interface of the CWarpTPSView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WARPTPSVIEW_H__2DAB2AE8_C5E5_497F_BC5F_159207454250__INCLUDED_)
#define AFX_WARPTPSVIEW_H__2DAB2AE8_C5E5_497F_BC5F_159207454250__INCLUDED_

#include <DibView.h>	// Added by ClassView
#include <MorphSlider.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CWarpTPSView : public CView
{
protected: // create from serialization only
	CWarpTPSView();
	DECLARE_DYNCREATE(CWarpTPSView)

// Attributes
public:
	CWarpTPSDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWarpTPSView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	// returns the rectangle for the three views
	CRect GetRect(int nArea);

	// the subordinate views
	CDibView m_arrViews[3];

	MorphSlider m_morphSlider;

	// the transform for the view
	CTPSTransform * m_pTransform;

	// the inverse transform for the view
	CTPSTransform * m_pInverseTransform;

	// destructor
	virtual ~CWarpTPSView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWarpTPSView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowmarks();
	afx_msg void OnUpdateShowmarks(CCmdUI* pCmdUI);
	afx_msg void OnFileOpenImages();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in WarpTPSView.cpp
inline CWarpTPSDoc* CWarpTPSView::GetDocument()
   { return (CWarpTPSDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WARPTPSVIEW_H__2DAB2AE8_C5E5_497F_BC5F_159207454250__INCLUDED_)
