// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: Optimizer.h 605 2008-09-14 18:06:23Z dglane001 $
#if !defined(OPTIMIZER_H)
#define OPTIMIZER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// include for the objective function
#include "ObjectiveFunction.h"
#include <vnl/vnl_nonlinear_minimizer.h>

class COptimizer;

// callback function
typedef BOOL OptimizerCallback(COptimizer *pOpt, void *pParam);

//////////////////////////////////////////////////////////////////////
// class COptimizer
// 
// base template class for all optimizers
//////////////////////////////////////////////////////////////////////
class COptimizer : public vnl_nonlinear_minimizer
{
public:
	// construct a new COptimizer object
	COptimizer(CObjectiveFunction *pFunc);

	// destroy the optimizer
	virtual ~COptimizer();

	// sets the callback function
	void SetCallback(OptimizerCallback *pCallback, void *pParam = NULL);

	// the flag to indicate that gradient information should
	//		be used
	BOOL UseGradientInfo() const;
	void SetUseGradientInfo(BOOL bUseGradientInfo);

	// defines the tolerance for exit from optimization loop
	REAL GetTolerance() const;
	void SetTolerance(REAL tol);

	// holds the number of iterations needed for the optimization
	int GetIterations() const;

	// holds the final value of the optimization
	REAL GetFinalValue() const;

	// holds the final value of the parameters for the minimum f
	const CVectorN<>& GetFinalParameter() const;

	// function to actually perform the optimization
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit) = 0;

protected:
	// the objective function over which optimization is to occur
	CObjectiveFunction *m_pFunc;

	// stores the callback info
	OptimizerCallback *m_pCallbackFunc;
	void *m_pCallbackParam;

	// holds the tolerance for the optimization
	// REAL m_tolerance;

	// holds the number of iterations for the optimization
	// int m_nIteration;

	// holds the value of the objective function at the final point
	REAL m_finalValue;

	// holds the final input parameter to the objective function (same as
	//		returned by Optimize)
	CVectorN<> m_vFinalParam;

private:
	// flag to indicate whether gradient information should be used
	BOOL m_bUseGradientInfo;

};	// class COptimizer


#endif // !defined(OPTIMIZER_H)
