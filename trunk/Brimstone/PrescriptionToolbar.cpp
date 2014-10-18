// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: PrescriptionToolbar.cpp 650 2009-11-05 22:24:55Z dglane001 $
#include "stdafx.h"
#include "Brimstone.h"

#include "BrimstoneDoc.h"
#include "BrimstoneView.h"

#include "PrescriptionToolbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar

/////////////////////////////////////////////////////////////////////////////
CPrescriptionToolbar::CPrescriptionToolbar(CWnd *pParent)
{
}	// CPrescriptionToolbar::CPrescriptionToolbar

/////////////////////////////////////////////////////////////////////////////
CPrescriptionToolbar::~CPrescriptionToolbar()
{
}	// CPrescriptionToolbar::~CPrescriptionToolbar


/////////////////////////////////////////////////////////////////////////////
CBrimstoneDoc *
	CPrescriptionToolbar::GetDocument()
{
	return GetView() ? GetView()->GetDocument() : NULL;

}	// CPrescriptionToolbar::GetDocument

/////////////////////////////////////////////////////////////////////////////
dH::Structure * 
	CPrescriptionToolbar::GetSelectedStruct(void)
	// returns the currently selected structure
{
	int nIndex = m_cbSSelect.GetCurSel();
	if (nIndex != CB_ERR)
	{
		dH::Structure *pStruct = (dH::Structure *) m_cbSSelect.GetItemDataPtr(nIndex);
		return pStruct;
	}

	return NULL;

}	// CPrescriptionToolbar::GetSelectedStruct

/////////////////////////////////////////////////////////////////////////////
dH::VOITerm * 
	CPrescriptionToolbar::GetSelectedPresc(void)
{
#ifdef USE_RTOPT
	// set prescription info
	if (NULL != GetDocument())
	{
		dH::VOITerm *pVOIT = GetDocument()->m_pOptimizer->GetPrescription(0)
			->GetStructureTerm(GetSelectedStruct());
		return pVOIT;
	}
#endif

	return NULL;

}	// CPrescriptionToolbar::GetSelectedPresc


/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_STRUCTSELECT, m_cbSSelect);

	DDX_Control(pDX, IDC_PRIO_EDIT, m_editPrio);

	DDX_Control(pDX, IDC_CHECK_CONTOUR, m_btnContour);

	DDX_Control(pDX, IDC_STRUCT_VISIBLE, m_btnVisible);
	DDX_Control(pDX, IDC_STRUCT_HISTO_VISIBLE, m_btnHistogram);

	DDX_Control(pDX, IDC_STRUCTTYPE, m_cbStructType);


	DDX_Control(pDX, IDC_STRUCTWEIGHT, m_editWeight);

	DDX_Control(pDX, IDC_DOSE1_EDIT2, m_editDose1);
	DDX_Control(pDX, IDC_DOSE2_EDIT2, m_editDose2);
	DDX_Control(pDX, IDC_BTN_SETINTERVAL, m_btnEditInterval);

	CDialogBar::DoDataExchange(pDX);

}	// CPrescriptionToolbar::DoDataExchange



BEGIN_MESSAGE_MAP(CPrescriptionToolbar, CDialogBar)
	//{{AFX_MSG_MAP(CDialogBar)
	ON_CBN_SELCHANGE(IDC_STRUCTSELECT, OnSelchangeStructselectcombo)
	ON_CBN_DROPDOWN(IDC_STRUCTSELECT, OnDropdownStructselectcombo)
	ON_EN_CHANGE(IDC_PRIO_EDIT, &CPrescriptionToolbar::OnEnChangePrioEdit)
	ON_BN_CLICKED(IDC_STRUCT_VISIBLE, OnVisibleCheck)
	ON_BN_CLICKED(IDC_STRUCT_HISTO_VISIBLE, OnHistogramCheck)
	ON_EN_CHANGE(IDC_STRUCTWEIGHT, &CPrescriptionToolbar::OnEnChangeStructweight)
	ON_BN_CLICKED(IDC_BTN_SETINTERVAL, &CPrescriptionToolbar::OnBnClickedBtnSetinterval)
	ON_CBN_SELCHANGE(IDC_STRUCTTYPE, &CPrescriptionToolbar::OnPrescriptionChange)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_CONTOUR, &CPrescriptionToolbar::OnBnClickedCheckContour)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionToolbar message handlers

/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnSelchangeStructselectcombo() 
{
	dH::Structure *pStruct = GetSelectedStruct();

	// set buttons
	m_cbStructType.SetCurSel((int) pStruct->GetType());

	// set visibility check
	m_btnVisible.SetCheck(pStruct->GetVisible() ? 1 : 0);

	// set histogram check
	CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
	m_btnHistogram.SetCheck(NULL != pHisto ? 1 : 0);

#ifdef USE_RTOPT
	// set prescription info
	dH::KLDivTerm *pKLDT = static_cast<dH::KLDivTerm *>(GetSelectedPresc());
	if (NULL != pKLDT)
	{
		//m_sliderWeight.SetPos((int) (pKLDT->GetWeight() * 20.0));
		CString strWeight;
		strWeight.Format(_T("%6.2lf"), pKLDT->GetWeight());
		m_editWeight.SetWindowText(strWeight);

		CString strPrio;
		strPrio.Format(_T("%i"), pKLDT->GetVOI()->GetPriority());

		int nDose1 = (int) floor(pKLDT->GetMinDose() * 100.0 + 0.5);
		CString strDose1;
		strDose1.Format(_T("%i"), nDose1);

		int nDose2 = (int) floor(pKLDT->GetMaxDose() * 100.0 + 0.5);
		CString strDose2;
		strDose2.Format(_T("%i"), nDose2);

		// FUCKED UP these all have to change at same time, because of trigger to fuckin UpdatePresc
		m_editPrio.SetWindowText(strPrio);
		m_editDose1.SetWindowText(strDose1);
		m_editDose2.SetWindowText(strDose2);
	}
	else
	{
		// make sure type is set
		ASSERT(dH::Structure::eNONE == pStruct->GetType());

		m_editPrio.SetWindowText(_T(""));
		m_editDose1.SetWindowText(_T(""));
		m_editDose2.SetWindowText(_T(""));
	}
#endif

	// updates other controls
	OnPrescriptionChange();

}	// CPrescriptionToolbar::OnSelchangeStructselectcombo

/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnDropdownStructselectcombo() 
{
	if (GetDocument())
	{
		m_cbSSelect.ResetContent();

		dH::Series::Pointer pSeries = GetDocument()->m_pSeries;
		for (int nStruct = 0; nStruct < pSeries->GetStructureCount(); nStruct++)
		{
			dH::Structure *pStruct = pSeries->GetStructureAt(nStruct);
			int nIndex = m_cbSSelect.AddString(CString(pStruct->GetName().c_str()));
			m_cbSSelect.SetItemDataPtr(nIndex, (void *) pStruct);
		}
	}

}	// CPrescriptionToolbar::OnDropdownStructselectcombo


/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnVisibleCheck() 
{
	dH::Structure *pStruct = GetSelectedStruct();
	if (NULL != pStruct)
	{
		// set visibility flag
		pStruct->SetVisible(m_btnVisible.GetCheck() == 1);

		// update views
		GetView()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

}	// CPrescriptionToolbar::OnVisibleCheck

/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnHistogramCheck()
{
	dH::Structure *pStruct = GetSelectedStruct();
	if (NULL != pStruct)
	{
		if (m_btnHistogram.GetCheck() == 1) 
		{
			// trigger generation of histogram
			GetView()->AddHistogram(pStruct);
		}
		else
		{
			// TODO: check that no prescription is present
			GetView()->RemoveHistogram(pStruct);
		}
	}

}	// CPrescriptionToolbar::OnHistogramCheck

/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnPrescriptionChange()
{
	// create a new prescription object
	dH::Structure *pStruct = GetSelectedStruct();
	if (NULL == pStruct)
	{
		return;
	}

	// set structure type
	pStruct->SetType((dH::Structure::StructType) m_cbStructType.GetCurSel());

#ifdef USE_RTOPT
	// update include flags to reflect changed types
	GetDocument()->m_pOptimizer->GetPrescription(0)->SetElementInclude();

	// set visibility / histogram options
	if (dH::Structure::eNONE != pStruct->GetType())
	{
		// see if prescription exists
		dH::VOITerm *pVOIT = GetSelectedPresc();
		if (NULL == pVOIT)
		{
			// need to create new term
			dH::KLDivTerm::Pointer pKLDT = dH::KLDivTerm::New(); // pStruct); // , 2.5);
			pKLDT->SetVOI(pStruct);
			pKLDT->SetWeight(2.5);

			// NOTE: must AddStructureTerm before SetInterval, because it sets up binning parameters
			GetView()->AddStructTerm(pKLDT);
		
			REAL dose1 = (dH::Structure::eTARGET == pStruct->GetType()) ? 0.60 : 0.0;
			REAL dose2 = (dH::Structure::eTARGET == pStruct->GetType()) ? 0.70 : 0.30;

			// sets the term prescription interval
			pKLDT->SetInterval(dose1, dose2, 1.0, TRUE);

			// update tool bar
			OnSelchangeStructselectcombo();

			// update other views
			GetView()->AddHistogram(pStruct);
		}

		// make sure visible
		m_btnVisible.SetCheck(1);
		pStruct->SetVisible(true);

		// make sure histogram
		m_btnHistogram.SetCheck(1);
	}
#endif

	// make sure histogram enabled
	m_btnHistogram.EnableWindow(dH::Structure::eNONE == pStruct->GetType());

	// set prescription control options
	m_editWeight.EnableWindow(dH::Structure::eNONE != pStruct->GetType());

	// set dose edit windows
	if (m_btnEditInterval.GetCheck() == 1)
	{
		if (dH::Structure::eNONE != pStruct->GetType())
		{
			m_editDose1.SetReadOnly(0);
			m_editDose2.SetReadOnly(0);
		}
	}
	else
	{
		m_editDose1.SetReadOnly(1);
		m_editDose2.SetReadOnly(1);
	}

}	// CPrescriptionToolbar::OnPrescriptionChange

/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnEnChangePrioEdit()
{
#ifdef USE_RTOPT
	dH::VOITerm *pVOIT = GetSelectedPresc();
	if (NULL == pVOIT)
	{
		return;
	}

	CString strPrio;
	m_editPrio.GetWindowText(strPrio);

	int nPrio = 1;		
	_stscanf_s(strPrio, _T("%i"), &nPrio);
	pVOIT->GetVOI()->SetPriority(nPrio);
#endif
}	// CPrescriptionToolbar::OnEnChangePrioEdit


/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnBnClickedBtnSetinterval()
{
#ifdef USE_RTOPT
	if (m_btnEditInterval.GetCheck() == 1)
	{
		if (dH::Structure::eNONE != GetSelectedStruct()->GetType())
		{
			m_editDose1.SetReadOnly(0);
			m_editDose2.SetReadOnly(0);
		}
		return;
	}
	m_editDose1.SetReadOnly(1);
	m_editDose2.SetReadOnly(1);
	
	dH::VOITerm *pVOIT = GetSelectedPresc();
	if (NULL == pVOIT)
	{
		return;
	}

	CString strDose1;
	m_editDose1.GetWindowText(strDose1);

	int nDose1 = 0;
	_stscanf_s(strDose1, _T("%i"), &nDose1);

	CString strDose2;
	m_editDose2.GetWindowText(strDose2);

	int nDose2 = 0;
	_stscanf_s(strDose2, _T("%i"), &nDose2);

	if (nDose1 < nDose2)
	{
		dH::KLDivTerm *pKLDT = static_cast<dH::KLDivTerm *>(pVOIT);
		pKLDT->SetInterval((REAL) nDose1 / 100.0, 
			(REAL) nDose2 / 100.0, 1.0, TRUE);
		ASSERT((int) floor(pKLDT->GetMinDose() * 100.0 + 0.5) == nDose1);
		ASSERT((int) floor(pKLDT->GetMaxDose() * 100.0 + 0.5) == nDose2);
	}
#endif
}	// CPrescriptionToolbar::OnBnClickedBtnSetinterval

/////////////////////////////////////////////////////////////////////////////
void 
	CPrescriptionToolbar::OnCbnSelchangeStructtype()
{
	OnPrescriptionChange();
}

/////////////////////////////////////////////////////////////////////////////
void CPrescriptionToolbar::OnEnChangeStructweight()
{
#ifdef USE_RTOPT
	// get term, if one is selected
	dH::VOITerm *pVOIT = GetSelectedPresc();
	if (NULL != pVOIT)
	{
		CString strWeight;
		m_editWeight.GetWindowText(strWeight);

		double weight = 2.5;
		_stscanf_s(strWeight.GetBuffer(), _T("%lf"), &weight);

		pVOIT->SetWeight(weight);
	}
#endif
}

void CPrescriptionToolbar::OnBnClickedCheckContour()
{
	if (m_btnContour.GetCheck())
	{
		GetView()->m_wndPlanarView.SetSelectedStructure(GetSelectedStruct());
		GetView()->m_wndPlanarView.SetSelectedContour(NULL);
		GetView()->m_wndPlanarView.SetSelectedVertex(NULL);

		// TODO: put Invalidate in SetSelectedStructure
		GetView()->m_wndPlanarView.Invalidate();
	}
	else
	{
		GetView()->m_wndPlanarView.SetSelectedStructure(NULL);
		GetView()->m_wndPlanarView.SetSelectedContour(NULL);
		GetView()->m_wndPlanarView.SetSelectedVertex(NULL);

		GetView()->m_wndPlanarView.Invalidate();
	}
}
