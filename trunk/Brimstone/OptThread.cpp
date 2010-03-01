// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: OptThread.cpp 612 2008-09-14 18:38:53Z dglane001 $
#include "stdafx.h"

#include "OptThread.h"

#ifdef USE_RTOPT
#include <PlanOptimizer.h>
#include <ConjGradOptimizer.h>
#endif

// COptThread

IMPLEMENT_DYNCREATE(COptThread, CWinThread)

//////////////////////////////////////////////////////////////////////////////
COptThread::COptThread()
	: m_pPlanOpt(NULL)
{
}

//////////////////////////////////////////////////////////////////////////////
COptThread::~COptThread()
{
}

//////////////////////////////////////////////////////////////////////////////
BOOL 
	COptThread::InitInstance()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
int COptThread::ExitInstance()
{
	return CWinThread::ExitInstance();
}

//////////////////////////////////////////////////////////////////////////////
COptThread::COptIterData::COptIterData(void)
	: m_nLevel(0)
	, m_nIteration(0)
{
}

BEGIN_MESSAGE_MAP(COptThread, CWinThread)
	ON_THREAD_MESSAGE(WM_OPTIMIZER_START, &COptThread::OnOptimizerStart)
	ON_THREAD_MESSAGE(WM_OPTIMIZER_STOP, &COptThread::OnOptimizerStop)
END_MESSAGE_MAP()


// COptThread message handlers

//////////////////////////////////////////////////////////////////////////////
BOOL 
	COptThread::OnIteration(DynamicCovarianceOptimizer *pOpt, void *pParam)
{
#ifdef USE_RTOPT
	COptThread *pThread = static_cast<COptThread*>(pParam);
	dH::PlanOptimizer *pPlanOpt = pThread->GetPlanOpt();

	// stores the data from the iteration
	COptIterData *pOID = NULL;

	// now find the level that we are at
	for (int nLevel = dH::PlanPyramid::MAX_SCALES-1; nLevel >= 0; nLevel--)
	{
		DynamicCovarianceOptimizer *pOptLevel = pPlanOpt->GetOptimizer(nLevel);			
		if (pOpt == pOptLevel)
		{
			// construct the iteration data object
			pOID = new COptIterData();
			pOID->m_nLevel = nLevel;
			pOID->m_nIteration = pOpt->get_num_iterations/*GetIterations*/();
			pOID->m_ofvalue = pOpt->GetFinalValue();
			pOID->m_vParam.SetDim(pOpt->GetFinalParameter().size/*GetDim*/());
			pOID->m_vParam = pOpt->GetFinalParameter();

			// transform the final parameter
			pPlanOpt->GetPrescription(nLevel)->Transform(&pOID->m_vParam);

			// if we are still above G0, then proceed to filter down to G0
			for (; nLevel > 0; nLevel--)
			{
				// now filter to proper level
				CVectorN<> vTemp;
				pPlanOpt->InvFilterStateVector(nLevel, pOID->m_vParam, vTemp); // pOID->m_vParam);
				pOID->m_vParam.SetDim(vTemp.GetDim());
				pOID->m_vParam = vTemp;
			}
		}
	}
	ASSERT(pOID != NULL);

	pThread->GetMsgTarget()->PostMessage(WM_OPTIMIZER_UPDATE, 0, (LPARAM) pOID);

	// get current thread state
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// and determine whether any messages are pending, if so...
	bool bContinueOptimizer = 
		!PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE);
	if (bContinueOptimizer)
	{
		// sleep for 50 msec to allow the UI to update
		Sleep(50);
	}

	// quit optimizer
	return bContinueOptimizer;
#else
	return FALSE;
#endif
}

//////////////////////////////////////////////////////////////////////////////
void 
	COptThread::OnOptimizerStart(WPARAM wParam, LPARAM lParam)
{
#ifdef USE_RTOPT
	COptIterData *pOID = new COptIterData();
	pOID->m_nLevel = 0;
	pOID->m_ofvalue = GetPlanOpt()->Optimize(pOID->m_vParam, &COptThread::OnIteration, this);

	GetMsgTarget()->PostMessage(WM_OPTIMIZER_DONE, 0, (LPARAM) pOID);

	// end the thread
	// return 0; // ExitInstance();
#endif
}

//////////////////////////////////////////////////////////////////////////////
void 
	COptThread::OnOptimizerStop(WPARAM wParam, LPARAM lParam)
{
	m_pMsgTarget->PostMessage(WM_OPTIMIZER_STOP, NULL, NULL);
}
