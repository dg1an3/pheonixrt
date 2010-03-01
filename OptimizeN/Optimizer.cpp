// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: Optimizer.cpp 606 2008-09-14 18:06:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "Optimizer.h"

//////////////////////////////////////////////////////////////////////
// COptimizer::COptimizer
// 
// base class for all optimizers
//////////////////////////////////////////////////////////////////////
COptimizer::COptimizer(vnl_cost_function/*CObjectiveFunction*/ *pFunc)
	: // m_tolerance(0.5f),
		//num_iterations_(0),
		m_pFunc(pFunc),
		m_pCallbackFunc(NULL),
		m_pCallbackParam(NULL)/*,
		m_bUseGradientInfo(FALSE)*/
{
}	// COptimizer::COptimizer


//////////////////////////////////////////////////////////////////////
// COptimizer::~COptimizer
// 
// destroy the optimizer
//////////////////////////////////////////////////////////////////////
COptimizer::~COptimizer()
{
}	// COptimizer::~COptimizer


///////////////////////////////////////////////////////////////////////////////
// COptimizer::SetCallback
// 
// sets the callback function
///////////////////////////////////////////////////////////////////////////////
void COptimizer::SetCallback(OptimizerCallback *pCallback, void *pParam)
{
	m_pCallbackFunc = pCallback;
	m_pCallbackParam = pParam;

}	// COptimizer::SetCallback


//////////////////////////////////////////////////////////////////////
// COptimizer::UseGradientInfo
// 
// returns the flag to indicate that gradient information should
//		be used
//////////////////////////////////////////////////////////////////////
//BOOL COptimizer::UseGradientInfo() const
//{
//	return m_bUseGradientInfo;
//
//}	// COptimizer::UseGradientInfo
//
//
////////////////////////////////////////////////////////////////////////
//// COptimizer::SetUseGradientInfo
//// 
//// sets the flag to indicate that gradient information should
////		be used
////////////////////////////////////////////////////////////////////////
//void COptimizer::SetUseGradientInfo(BOOL bUseGradientInfo)
//{
//	m_bUseGradientInfo = bUseGradientInfo;
//
//}	// COptimizer::SetUseGradientInfo


//////////////////////////////////////////////////////////////////////
// COptimizer::GetTolerance
// 
// returns the tolerance for exit from optimization loop
//////////////////////////////////////////////////////////////////////
//REAL COptimizer::GetTolerance() const
//{
//	return xtol; // m_tolerance;
//
//}	// COptimizer::GetTolerance
//
//
////////////////////////////////////////////////////////////////////////
//// COptimizer::SetTolerance
//// 
//// sets the tolerance for exit from optimization loop
////////////////////////////////////////////////////////////////////////
//void COptimizer::SetTolerance(REAL tol)
//{
//	// m_tolerance 
//		xtol = tol;
//
//}	// COptimizer::SetTolerance


//////////////////////////////////////////////////////////////////////
// COptimizer::GetIterations
// 
// returns the number of iterations needed for the previous 
//		optimization
//////////////////////////////////////////////////////////////////////
//int COptimizer::GetIterations() const
//{
//	return num_iterations_; // m_nIteration;
//
//}	// COptimizer::GetIterations
//

//////////////////////////////////////////////////////////////////////
// COptimizer::GetFinalValue
// 
// holds the final value of the optimization
//////////////////////////////////////////////////////////////////////
REAL COptimizer::GetFinalValue() const
{
	return m_finalValue;

}	// COptimizer::GetFinalValue


//////////////////////////////////////////////////////////////////////
// COptimizer::GetFinalParameter
// 
// holds the final value of the parameters for the minimum f
//////////////////////////////////////////////////////////////////////
// const CVectorN<>& 
const vnl_vector<REAL>& COptimizer::GetFinalParameter() const
{
	return // const_cast<CVectorN<>&>(m_vFinalParam).GetVnlVector();
		m_vFinalParam;

}	// COptimizer::GetFinalParameter

