// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: BrimstoneDoc.h 613 2008-09-14 18:47:53Z dglane001 $
#include <Series.h>
#include <Plan.h>
#ifdef USE_RTOPT
#include <Prescription.h>
#include <PlanOptimizer.h>
#endif

#pragma once

//////////////////////////////////////////////////////////////////////
// class CBrimstoneDoc
//
// manages loading / saving the model objects, importing, and dose
//		calculation
//////////////////////////////////////////////////////////////////////
class CBrimstoneDoc : public CDocument
{
protected: // create from serialization only
	CBrimstoneDoc();
	DECLARE_DYNCREATE(CBrimstoneDoc)

// Attributes
public:
	dH::Series::Pointer m_pSeries;
	dH::Plan::Pointer m_pPlan;

#ifdef USE_RTOPT
	// stores the PlanOptimizer object
	auto_ptr<dH::PlanOptimizer> m_pOptimizer;
#endif

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrimstoneDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~CBrimstoneDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBrimstoneDoc)
	afx_msg void OnGenbeamlets();
	afx_msg void OnFileImportDcm();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
