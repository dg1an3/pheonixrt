// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: BrimstoneView.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"
#include "Brimstone.h"

#include <HistogramDataSeries.h>
#ifdef USE_RTOPT
#include <TargetDVHSeries.h>
#endif

#include "BrimstoneDoc.h"
#include "BrimstoneView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView

IMPLEMENT_DYNCREATE(CBrimstoneView, CView)

BEGIN_MESSAGE_MAP(CBrimstoneView, CView)
	//{{AFX_MSG_MAP(CBrimstoneView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_MESSAGE(WM_OPTIMIZER_UPDATE, OnOptimizerThreadUpdate)
	ON_MESSAGE(WM_OPTIMIZER_STOP, OnOptimizerThreadDone)
	ON_MESSAGE(WM_OPTIMIZER_DONE, OnOptimizerThreadDone)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(IDC_BTNOPTIMIZE, &CBrimstoneView::OnOptimize)
	ON_UPDATE_COMMAND_UI(IDC_BTNOPTIMIZE, &CBrimstoneView::OnUpdateOptimize)
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_SCANBEAMLETS, &CBrimstoneView::OnViewScanbeamlets)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView construction/destruction

/////////////////////////////////////////////////////////////////////////////
CBrimstoneView::CBrimstoneView()
	: m_pOptThread(NULL)
	, m_bOptimizerRun(false)
	// , m_pIterDS(NULL)
{
	m_pOptThread = static_cast<COptThread*>(
		AfxBeginThread(RUNTIME_CLASS(COptThread), 
			THREAD_PRIORITY_BELOW_NORMAL,	// priority
			0,								// stack size
			CREATE_SUSPENDED				// flags
		));
	m_pOptThread->m_bAutoDelete = true;
	m_pOptThread->SetMsgTarget(this);
	m_pOptThread->ResumeThread();
}

/////////////////////////////////////////////////////////////////////////////
CBrimstoneView::~CBrimstoneView()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL 
	CBrimstoneView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::AddHistogram(dH::Structure * pStruct)
	// generates a histogram for the specified structure
{
	CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, true);
	ASSERT(pHisto != NULL);

	CHistogramDataSeries::Pointer pHistogramSeries = CHistogramDataSeries::New();
	pHistogramSeries->SetHistogram(pHisto);

	pHistogramSeries->m_pGraph = &m_graphDVH;
	pHistogramSeries->SetColor(pStruct->GetColor());

	CDataSeries::Pointer pSeries = pHistogramSeries;
	m_graphDVH.AddDataSeries(pSeries);
	m_graphDVH.AutoScale();
	m_graphDVH.SetAxesMin(MakeVector<2>(0.0f, 0.0f));
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::RemoveHistogram(dH::Structure * pStruct)
	// removes histogram for designated structure
{
	CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
	ASSERT(pHisto != NULL);

	for (int nAt = 0; nAt < m_graphDVH.GetDataSeriesCount(); nAt++)
	{
		CHistogramDataSeries *pSeries = 
			static_cast<CHistogramDataSeries *>(m_graphDVH.GetDataSeriesAt(nAt));
		if (pSeries->GetHistogram() == pHisto)
		{
			m_graphDVH.RemoveDataSeries(nAt);
			m_graphDVH.AutoScale();
			m_graphDVH.SetAxesMin(MakeVector<2>(0.0f, 0.0f));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::AddStructTerm(dH::VOITerm * pVOIT)
	// add new structure term
{
#ifdef USE_RTOPT
	// add to the prescription object
	GetDocument()->m_pOptimizer->/*GetPrescription(0)->*/AddStructureTerm(pVOIT);

	// get the struct
	dH::Structure *pStruct = pVOIT->GetVOI();

	// form the data series
	CTargetDVHSeries::Pointer pTargetDvhSeries = CTargetDVHSeries::New();
	pTargetDvhSeries->SetForKLDivTerm((dH::KLDivTerm*)pVOIT);
	pTargetDvhSeries->OnKLDTChanged();

	pTargetDvhSeries->SetColor(pStruct->GetColor());
	pTargetDvhSeries->SetPenStyle(PS_DASHDOT);
	pTargetDvhSeries->SetHasHandles(TRUE);

	CDataSeries::Pointer pSeries = pTargetDvhSeries;
	m_graphDVH.AddDataSeries(pSeries);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView drawing

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneView::OnDraw(CDC* pDC)
{
	CBrimstoneDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CView::OnDraw(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView diagnostics

#ifdef _DEBUG
void CBrimstoneView::AssertValid() const
{
	CView::AssertValid();
}

void CBrimstoneView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBrimstoneDoc* CBrimstoneView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBrimstoneDoc)));
	return (CBrimstoneDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBrimstoneView message handlers

/////////////////////////////////////////////////////////////////////////////
int 
	CBrimstoneView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// create the planar window
	m_wndPlanarView.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
		CRect(0, 0, 200, 200), this, /* nID */ 111);

	// load the colormap for dose display
	CDib colormap;
	BOOL bResult = colormap.Load(GetModuleHandle(NULL), IDB_RAINBOW);	
	CSize size = colormap.GetSize();
	m_arrColormap.SetSize(/* size.cx */ size.cy);

	CArray<UCHAR, UCHAR&> arrRaw;
	arrRaw.SetSize(size.cx * size.cy * 3);
	colormap.GetBitmapBits(size.cx * size.cy * 3, arrRaw.GetData());

	int nAtRaw = 0;
	for (int nAt = 0; nAt < m_arrColormap.GetSize(); nAt++)
	{ 
		m_arrColormap[nAt] = RGB(arrRaw[nAtRaw+2], arrRaw[nAtRaw+1], arrRaw[nAtRaw]);
		nAtRaw += (3 * size.cx);
	}

	m_wndPlanarView.SetLUT(m_arrColormap, 1); 

	m_wndPlanarView.SetWindowLevel((REAL) 1.0 / 0.8, 0.4, 1);

	// create the graph window
	m_graphDVH.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
		CRect(0, 200, 200, 400), this, /* nID */ 113);

	m_graphDVH.SetLegendLUT(m_arrColormap, 
		m_wndPlanarView.m_window[1], m_wndPlanarView.m_level[1]);

	// create the graph window
	m_graphIterations.Create(NULL, NULL, WS_BORDER | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
		CRect(0, 200, 200, 400), this, /* nID */ 114);

	for (int nD = 0; nD < dH::Structure::MAX_SCALES; nD++)
		m_pIterDS[nD] = CDataSeries::New();
	m_pIterDS[0]->SetColor(RGB(255, 0, 0));
	m_graphIterations.AddDataSeries(m_pIterDS[0]);
	m_pIterDS[1]->SetColor(RGB(0, 255, 0));
	m_graphIterations.AddDataSeries(m_pIterDS[1]);
	m_pIterDS[2]->SetColor(RGB(0, 0, 255));
	m_graphIterations.AddDataSeries(m_pIterDS[2]);
	m_pIterDS[3]->SetColor(RGB(255, 0, 255));
	m_graphIterations.AddDataSeries(m_pIterDS[3]);


	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnDestroy()
{
	m_pOptThread->PostThreadMessage(WM_QUIT, NULL, NULL);
	CView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
#ifdef USE_RTOPT
	m_pOptThread->SetPlanOpt(GetDocument()->m_pOptimizer.get());
#endif

	if (GetDocument()->m_pSeries)
	{
		m_wndPlanarView.SetSeries(GetDocument()->m_pSeries);
		//m_wndPlanarView.SetVolume(GetDocument()->m_pSeries->GetDensity(), 0);
		//m_wndPlanarView.m_pSeries = GetDocument()->m_pSeries.get();

	}
	if (GetDocument()->m_pPlan)
	{
		m_wndPlanarView.SetVolume(GetDocument()->m_pPlan->GetDoseMatrix(), 1);
	}
	m_wndPlanarView.Invalidate(TRUE);

	// set the initial view center
	m_wndPlanarView.InitZoomCenter();
	if (GetDocument()->m_pPlan
		&& GetDocument()->m_pPlan->GetBeamCount() > 0)
	{
		Vector<REAL> vIsocenter = GetDocument()->m_pPlan->GetBeamAt(0)->GetIsocenter();
		m_wndPlanarView.SetCenter(vIsocenter);
	}

	// delete histograms
	m_graphDVH.RemoveAllDataSeries();

	// generate ADDHISTO events
	for (int nAt = 0; nAt < GetDocument()->m_pSeries->GetStructureCount(); nAt++)
	{
		dH::Structure *pStruct = GetDocument()->m_pSeries->GetStructureAt(nAt);
		CHistogram *pHisto = GetDocument()->m_pPlan->GetHistogram(pStruct, false);
		if (NULL != pHisto)
		{
			AddHistogram(pStruct);
		}
	}

	m_graphDVH.Invalidate(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	m_wndPlanarView.MoveWindow(0, 0, 5 * cx / 8, cy);

	// reposition the graph window
	m_graphDVH.MoveWindow(5 * cx / 8, 0, 3 * cx / 8, cy / 2);	

	// reposition the iterations window
	m_graphIterations.MoveWindow(5 * cx / 8, cy / 2, 3 * cx / 8, cy / 2);	
}


static int m_nTotalIter;
static FILE *m_pOutFile = NULL;

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnOptimize()
{
	if (!m_bOptimizerRun)
	{
		m_nTotalIter = 0;

		// clear iteration data matrix
		CMatrixNxM<> mEmpty;
		for (int nL = 0; nL < dH::Structure::MAX_SCALES; nL++)
			m_pIterDS[nL]->SetDataMatrix(mEmpty);

		m_pOptThread->PostThreadMessage(WM_OPTIMIZER_START, 0, 0);
		m_bOptimizerRun = true;
	}
	else
	{
		m_pOptThread->PostThreadMessage(WM_OPTIMIZER_STOP, 0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CBrimstoneView::OnUpdateOptimize(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bOptimizerRun ? 1 : 0);
}

/////////////////////////////////////////////////////////////////////////////
LRESULT 
	CBrimstoneView::OnOptimizerThreadUpdate(WPARAM wParam, LPARAM lParam)
{
#ifdef USE_RTOPT
	USES_CONVERSION;
	
	const bool bUpdateDisplay = true;
	if (bUpdateDisplay)
	{
		// check if another WM_OPTIMIZER_UPDATE message is waiting
		MSG msg;
		const bool bPeekMessage = false;
		if (bPeekMessage
			&& ::PeekMessage(&msg, (HWND) INVALID_HANDLE_VALUE, WM_OPTIMIZER_UPDATE, WM_OPTIMIZER_UPDATE, PM_REMOVE))
		{
			// if so, skip processing this one
			return 0;
		}

		COptThread::COptIterData *pOID = (COptThread::COptIterData *) lParam;
		ASSERT(pOID != NULL);

		if (pOID->m_ofvalue > 0.1)
		{
			m_pIterDS[pOID->m_nLevel]->AddDataPoint(MakeVector<2>(m_nTotalIter, -log10(pOID->m_ofvalue - 0.1)));
		}

		const int nUpdateEvery = 1;
		if (m_nTotalIter % nUpdateEvery == 0)
		{
			GetDocument()->m_pOptimizer->SetStateVectorToPlan(pOID->m_vParam);
		}
		m_nTotalIter++;

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
LRESULT 
	CBrimstoneView::OnOptimizerThreadDone(WPARAM wParam, LPARAM lParam)
{
#ifdef USE_RTOPT
	COptThread::COptIterData *pOID = (COptThread::COptIterData *) lParam;
	if (pOID != NULL)
	{
		GetDocument()->m_pOptimizer->SetStateVectorToPlan(pOID->m_vParam);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	m_bOptimizerRun = false;
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::ScanBeamlets(int nLevel)
{
	dH::Plan::Pointer pPlan = GetDocument()->m_pPlan;

	if (nLevel == 0)
	{
		for (int nAt = pPlan->GetBeamAt(0)->GetBeamletCount()-1; 
			nAt >= // 0; // 
				pPlan->GetBeamAt(0)->GetBeamletCount()/2; 
			nAt--)
		{
			CVectorN<> vWeights;
			vWeights.SetDim(pPlan->GetBeamAt(0)->GetBeamletCount());
			vWeights.SetZero();
			vWeights[nAt] = (REAL) 0.8;

			for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
			{
				CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
				pBeam->SetIntensityMap(vWeights);
			}
			//CVolume<VOXEL_REAL> *pDose = 
			pPlan->GetDoseMatrix();		// triggers recalc
			//VOXEL_REAL maxDose = pDose->GetMax();
			//TRACE("maxDose = %f\n", maxDose);

			m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}	
	}
	//else
	//{
	//	for (int nAtBeam = 0; nAtBeam < pPlan->GetBeamCount(); nAtBeam++)
	//	{
	//		CBeam *pBeam = pPlan->GetBeamAt(nAtBeam);
	//		for (int nShift = -pBeam->GetBeamletCount(nLevel) / 2; 
	//				nShift<= pBeam->GetBeamletCount(nLevel) / 2; nShift++)
	//		{
	//			CVolume<VOXEL_REAL> *pBeamlet = pBeam->GetBeamlet(nShift);
	//			m_wndPlanarView.SetVolume(pBeamlet, 1);
	//			m_wndPlanarView.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	//		}

	//	}	
	//}
}

////////////////////////////////////////////////////////////////////////////
void 
	CBrimstoneView::OnViewScanbeamlets()
{
	ScanBeamlets(0);

}
