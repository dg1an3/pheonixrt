// PlanSetupDlg.cpp : implementation file
//
#include "stdafx.h"
#include "Brimstone.h"
#include "PlanSetupDlg.h"


#define WM_DOSECALC_UPDATE WM_APP+9
#define WM_DOSECALC_DONE WM_APP+10

/////////////////////////////////////////////////////////////////////////////
// CPlanSetupDlg dialog

//////////////////////////////////////////////////////////////////////////////
CPlanSetupDlg::CPlanSetupDlg(dH::Plan *pPlan, CWnd* pParent /*=NULL*/)
	: CDialog(CPlanSetupDlg::IDD, pParent)
{
	SetPlan(pPlan);

	//{{AFX_DATA_INIT(CPlanSetupDlg)
	m_nBeamCount = 1;
	m_isoX = 230.0;
	m_isoY = 200.0;
	m_isoZ = -172.0;
	m_resolution = 2.0;
	m_energy = 15.0;
	m_termDist = 0.2;
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
	DDX_Text(pDX, IDC_EDIT_RESOLUTION, m_resolution);
	DDX_Text(pDX, IDC_ENERGYCOMBO, m_energy);
	DDX_Text(pDX, IDC_EDIT_TERMDIST, m_termDist);
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

	for (int nAtBeam = 0; nAtBeam < pPSD->m_arrDoseCalculators.size(); nAtBeam++)
	{
		dH::BeamDoseCalc *pDoseCalc = pPSD->m_arrDoseCalculators[nAtBeam];
		bool bContinue = true;
		while (bContinue)
		{
			dH::BasisGroupType::IndexType currentIndex;
			bContinue = pDoseCalc->CalcNextBeamlet(currentIndex);
			pPSD->PostMessage(WM_DOSECALC_UPDATE, (WPARAM) nAtBeam, (LPARAM) currentIndex[0]);
		}
	}

	// post done message
	pPSD->PostMessage(WM_DOSECALC_DONE, NULL, NULL);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
void 
	CPlanSetupDlg::OnBnClickedGo()
{
	if (!UpdateData(TRUE))
		return;

	// set the plan dose resolution
	GetPlan()->SetDoseResolution(m_resolution);

	// set up the isocentric beams
	GetPlan()->CreateEquidistantBeams(m_nBeamCount, 
		MakeVector<3>(m_isoX, m_isoY, m_isoZ));

	// set up the kernel to be shared by the dose calculators
	dH::EnergyDepKernel::Pointer pKernel = dH::EnergyDepKernel::New();
	pKernel->SetEnergy(m_energy);
	pKernel->SetTerminateDistance(m_termDist);
	pKernel->LoadKernel();

	// create the beams
	for (int nAt = 0; nAt < GetPlan()->GetBeamCount(); nAt++)
	{
		// create the new beam dose calculator
		dH::BeamDoseCalc::Pointer pDoseCalc = dH::BeamDoseCalc::New();

		dH::Beam *pBeam = GetPlan()->GetBeamAt(nAt);
		pDoseCalc->SetBeam(pBeam);
		pDoseCalc->SetKernel(pKernel);

		// and add to the array
		m_arrDoseCalculators.push_back(pDoseCalc);
	}

	m_edtAtBeam.SetWindowText(_T("0"));
	m_edtAtBeamlet.SetWindowText(_T("*"));

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

