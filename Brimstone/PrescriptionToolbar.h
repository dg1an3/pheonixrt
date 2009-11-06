// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PrescriptionToolbar.h 640 2009-06-13 05:06:50Z dglane001 $
#if !defined(AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_)
#define AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_

#ifdef USE_RTOPT
#include <Prescription.h>
#endif

#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar window

class CPrescriptionToolbar : public CDialogBar
{
// Construction
public:
	CPrescriptionToolbar(CWnd *pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CPrescriptionToolbar)
	enum { IDD = IDD_PRESCDLG };

	CComboBox m_cbSSelect;
	
	CEdit m_editPrio;	// precedence edit control

	CButton m_btnContour;

	CButton m_btnVisible;
	CButton m_btnHistogram;

	CComboBox m_cbStructType;

	CEdit m_editWeight;

	CButton m_btnEditInterval;

	CEdit m_editDose1;
	CEdit m_editDose2;
	//}}AFX_DATA

// Attributes
public:
	DECLARE_ATTRIBUTE_PTR(View, CBrimstoneView);
	CBrimstoneDoc *GetDocument();

	// returns the currently selected structure
	dH::Structure * GetSelectedStruct(void);

	// returns presc for currently selected structure (if possible)
	dH::VOITerm * GetSelectedPresc(void);

// Operations
public:
	virtual void DoDataExchange(CDataExchange* pDX);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrescriptionToolbar)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPrescriptionToolbar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPrescriptionToolbar)
	afx_msg void OnSelchangeStructselectcombo();
	afx_msg void OnDropdownStructselectcombo();
	afx_msg void OnEnChangePrioEdit();	
	afx_msg void OnVisibleCheck();
	afx_msg void OnHistogramCheck();
	afx_msg void OnCbnSelchangeStructtype();
	afx_msg void OnEnChangeStructweight();
	afx_msg void OnBnClickedBtnSetinterval();
	afx_msg void OnDose1Changed();
	afx_msg void OnDose2Changed();
	afx_msg void OnPrescriptionChange();
	afx_msg void OnBnClickedCheckContour();
	// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};	// class CPrescriptionToolbar


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRECRIPTIONTOOLBAR_H__73F1EAAB_9018_41BD_B854_E43564255E66__INCLUDED_)
