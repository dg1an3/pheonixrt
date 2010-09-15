// Copyright (C) 2nd Messenger Systems
// $Id: PlanSetupDlg.cpp 650 2009-11-05 22:24:55Z dglane001 $
#include "stdafx.h"
#include "brimstone.h"
#include "PlanSetupDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_DOSECALC_UPDATE WM_APP+9
#define WM_DOSECALC_DONE WM_APP+10

/////////////////////////////////////////////////////////////////////////////
// CPlanSetupDlg dialog

//////////////////////////////////////////////////////////////////////////////
CPlanSetupDlg::CPlanSetupDlg(dH::PlanPyramid *pPlanPyramid, CWnd* pParent /*=NULL*/)
	: CDialog(CPlanSetupDlg::IDD, pParent)
	, m_pPlanPyramid(pPlanPyramid)
	//, m_pDCThread(NULL)
{
	//{{AFX_DATA_INIT(CPlanSetupDlg)
	m_nBeamCount = 1;
	m_isoX = 0.0;
	m_isoY = 0.0;
	m_isoZ = 0.0;
	//}}AFX_DATA_INIT
}

//////////////////////////////////////////////////////////////////////////////
void 
	CPlanSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlanSetupDlg)
	DDX_Text(pDX, IDC_EDIT_BEAMCOUNT, m_nBeamCount);
	DDV_MinMaxUInt(pDX, m_nBeamCount, 1, 99);
	DDX_Text(pDX, IDC_EDIT_ISO_OFS_X, m_isoX);
	DDX_Text(pDX, IDC_EDIT_ISO_OFS_Y, m_isoY);
	DDX_Text(pDX, IDC_EDIT_ISO_OFS_Z, m_isoZ);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_ATBEAM, m_edtAtBeam);
	DDX_Control(pDX, IDC_ATBEAMLET, m_edtAtBeamlet);
}


BEGIN_MESSAGE_MAP(CPlanSetupDlg, CDialog)
	//{{AFX_MSG_MAP(CPlanSetupDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_GO, &CPlanSetupDlg::OnBnClickedGo)
	ON_MESSAGE(WM_DOSECALC_UPDATE, OnDoseCalcThreadUpdate)
	ON_MESSAGE(WM_DOSECALC_DONE, OnDoseCalcThreadDone)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlanSetupDlg message handlers

UINT __cdecl CalculateBeamlets( LPVOID pParam )
{
	CPlanSetupDlg *pPSD = (CPlanSetupDlg *) pParam;

	for (int nAtBeam = pPSD->m_arrBDC.GetCount()-1; nAtBeam >= 0; nAtBeam--)
	{
		CBeamDoseCalc *pDoseCalc = pPSD->m_arrBDC[nAtBeam];

		// iterate for level 0 beamlets
		int nBeamletCount = // 5; //3; // 0;
			19;		// TODO: set beamlet count based on spacing and dose calc region
			// TODO: reconcile this with nBeamletCount used in PlanPyramid
		for (int nAtBeamlet = -nBeamletCount; nAtBeamlet <= nBeamletCount; nAtBeamlet++)
		{
			pPSD->PostMessage(WM_DOSECALC_UPDATE, (WPARAM) nAtBeam, (LPARAM) nAtBeamlet);
			pDoseCalc->CalcBeamlet(nAtBeamlet);
		}

		pPSD->m_pPlanPyramid->CalcPencilSubBeamlets(nAtBeam);
	}

	// post done message
	pPSD->PostMessage(WM_DOSECALC_DONE, NULL, NULL);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
void 
	CPlanSetupDlg::OnBnClickedGo()
{
	UpdateData(TRUE);

	// TODO: delete existing beams?
	ASSERT(m_pPlanPyramid->GetPlan()->GetBeamCount() == 0);

	// create the beams
	for (int nAt = m_nBeamCount-1; nAt >= 0; nAt--)
	{
		// create the new beam
		CBeam::Pointer pBeam = dH::Beam::New(); // new CBeam();
		m_pPlanPyramid->GetPlan()->AddBeam(pBeam);

		// calculate gantry for the beam
		// TODO: this calculation (+90 degrees) should be moved in to the beam
		double gantry;
		gantry = 90.0 + (double) nAt * 360.0 / (double) m_nBeamCount;
		pBeam->SetGantryAngle(gantry * PI / 180.0);

		// calculate iso position for the beam
		pBeam->SetIsocenter(MakeVector<3>(m_isoX, m_isoY, m_isoZ));

		CBeamDoseCalc *pDoseCalc = new CBeamDoseCalc(pBeam, m_pPlanPyramid->GetPlan()->m_pKernel);

		// now trigger calculation
		pDoseCalc->InitCalcBeamlets();

		// and add to the array
		m_arrBDC.Add(CAutoPtr<CBeamDoseCalc>(pDoseCalc));
	}

	// this generates all sub-beamlets
	m_pPlanPyramid->SetPlan(m_pPlanPyramid->GetPlan());

	m_pDCThread = AfxBeginThread(CalculateBeamlets, (LPVOID) this, 
			THREAD_PRIORITY_BELOW_NORMAL,	// priority
			0,								// stack size
			CREATE_SUSPENDED				// flags
		);
	m_pDCThread->m_bAutoDelete = true;
	m_pDCThread->ResumeThread();
}

//////////////////////////////////////////////////////////////////////////////
LRESULT 
	CPlanSetupDlg::OnDoseCalcThreadUpdate(WPARAM wParam, LPARAM lParam)
{
	CString strBeams;
	strBeams.Format(_T("%i"), (int) wParam);
	m_edtAtBeam.SetWindowText(strBeams);

	strBeams.Format(_T("%i"), (int) lParam);
	m_edtAtBeamlet.SetWindowText(strBeams);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
LRESULT 
	CPlanSetupDlg::OnDoseCalcThreadDone(WPARAM wParam, LPARAM lParam)
{
	m_edtAtBeam.SetWindowText(_T("Done"));
	m_edtAtBeamlet.SetWindowText(_T("Done"));
	return 0;
}

