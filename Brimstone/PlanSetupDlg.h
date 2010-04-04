// PlanSetupDlg.h : header file
//

#pragma once

#include <Plan.h>
#include <BeamDoseCalc.h>

/////////////////////////////////////////////////////////////////////////////
// CPlanSetupDlg dialog

class CPlanSetupDlg : public CDialog
{
// Construction
public:
	CPlanSetupDlg(dH::Plan *pPlan, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPlanSetupDlg)
	enum { IDD = IDD_PLANSETUPDLG };
	UINT	m_nBeamCount;
	double	m_isoX;
	double	m_isoY;
	double	m_isoZ;
	double m_resolution;
	double m_energy;
	double m_termDist;
	//}}AFX_DATA

	CEdit m_edtAtBeamlet;
	CEdit m_edtAtBeam;

	// the plan for the dose calc
	DeclareMemberSPtr(Plan, dH::Plan);

	// manages the thread object
	CWinThread *m_pDCThread;

	// the array of dose calc objects for each beam
	std::vector< dH::BeamDoseCalc::Pointer > m_arrDoseCalculators;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlanSetupDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDoseCalcThreadUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDoseCalcThreadDone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedGo();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

