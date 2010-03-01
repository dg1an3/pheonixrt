// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: OptThread.h 612 2008-09-14 18:38:53Z dglane001 $
#pragma once

// #include <Optimizer.h>
#include <ConjGradOptimizer.h>

#define WM_OPTIMIZER_START WM_APP+7
#define WM_OPTIMIZER_STOP  WM_APP+8
#define WM_OPTIMIZER_UPDATE WM_APP+9
#define WM_OPTIMIZER_DONE WM_APP+10

class COptimizer;

namespace dH
{
	class PlanOptimizer;
}

//////////////////////////////////////////////////////////////////////
// class COptThread
//
// thread to run the optimization
//////////////////////////////////////////////////////////////////////
class COptThread : public CWinThread
{
	DECLARE_DYNCREATE(COptThread)

protected:
	COptThread();           // protected constructor used by dynamic creation
	virtual ~COptThread();

public:
	// helpers to initialize / clean up
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// stores pointer to the current (internal) optimizer prescription
	DECLARE_ATTRIBUTE_PTR(PlanOpt, dH::PlanOptimizer);

	// pointer to window to be notified
	DECLARE_ATTRIBUTE_PTR(MsgTarget, CWnd);

	// callback for optimizer iterations
	static BOOL OnIteration(DynamicCovarianceOptimizer *pOpt, void *pParam);

	// stores data for current iteration
	class COptIterData
	{
	public:
		COptIterData();

		int m_nLevel;
		int m_nIteration;
		REAL m_ofvalue;
		CVectorN<> m_vParam;
		CVectorN<> m_vGrad;
	};

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnOptimizerStart(WPARAM wParam, LPARAM lParam);
	afx_msg void OnOptimizerStop(WPARAM wParam, LPARAM lParam);

};	// class COptThread


