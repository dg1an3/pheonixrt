// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: BrimstoneView.h 640 2009-06-13 05:06:50Z dglane001 $
#if !defined(AFX_BRIMSTONEVIEW_H__315F9461_92CF_4D86_B8C6_304D8C253E91__INCLUDED_)
#define AFX_BRIMSTONEVIEW_H__315F9461_92CF_4D86_B8C6_304D8C253E91__INCLUDED_

#include <Graph.h>
#include "PlanarView.h"	// Added by ClassView
#include "OptThread.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace dH
{
	class VOITerm;
}

//////////////////////////////////////////////////////////////////////
// class CBrimstoneView
//
// manages the view of the plan data
//////////////////////////////////////////////////////////////////////
class CBrimstoneView : public CView
{
protected: // create from serialization only
	CBrimstoneView();
	DECLARE_DYNCREATE(CBrimstoneView)

// Attributes
public:
	CBrimstoneDoc* GetDocument();

	// planar view of images / contours / dose
	CPlanarView m_wndPlanarView;

	// colormap for the dose display
	CArray<COLORREF, COLORREF> m_arrColormap;

	// graph to display the histogram
	CGraph m_graphDVH;

	// graph to display iterations
	CGraph m_graphIterations;
	CDataSeries::Pointer m_dsIter;

	// stores data series for iteration graph
	CDataSeries::Pointer m_pIterDS[dH::Structure::MAX_SCALES];

	// generates a histogram for the specified structure
	void AddHistogram(dH::Structure * pStruct);
	void RemoveHistogram(dH::Structure * pStruct);

	// add new structure term
	void AddStructTerm(dH::VOITerm * pVOIT);

// Operations
public:

	// scans the beamlets for the given level
	void ScanBeamlets(int nLevel);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrimstoneView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBrimstoneView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// my optimizer thread pointer
	COptThread *m_pOptThread;

	// flag when optimizer should continue running
	bool m_bOptimizerRun;

// Generated message map functions
public:
	//{{AFX_MSG(CBrimstoneView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnOptimize();
	afx_msg void OnUpdateOptimize(CCmdUI *pCmdUI);
	// custom handlers for optimizer thread messages
	afx_msg LRESULT OnOptimizerThreadUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOptimizerThreadDone(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnViewScanbeamlets();
};

#ifndef _DEBUG  // debug version in BrimstoneView.cpp
inline CBrimstoneDoc* CBrimstoneView::GetDocument()
   { return (CBrimstoneDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BRIMSTONEVIEW_H__315F9461_92CF_4D86_B8C6_304D8C253E91__INCLUDED_)
