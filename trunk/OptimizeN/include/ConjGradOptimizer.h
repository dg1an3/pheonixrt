// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ConjGradOptimizer.h 605 2008-09-14 18:06:23Z dglane001 $
#if !defined(CONJGRADOPTIMIZER_H)
#define CONJGRADOPTIMIZER_H

// base class includes
#include "Optimizer.h"

// subordinate brent optimizer
#include "BrentOptimizer.h"

// the line function for the Brent optimizer
#include "LineFunction.h"

#include "MatrixNxM.h"

//////////////////////////////////////////////////////////////////////
// class CConjGradOptimizer
// 
// optimizer that implements the Powell algorithm explained in
//		Numerical Recipes
//////////////////////////////////////////////////////////////////////
class CConjGradOptimizer : public COptimizer
{
public:
	// construct a gradient optimizer for an objective function
	CConjGradOptimizer(CObjectiveFunction *pFunc);

	// returns a reference to the embedded Brent optimizer
	vnl_brent_minimizer& GetBrentOptimizer();

	// optimize the objective function
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

	// flag to indicate that line opt tolerance should always be same
	DECLARE_ATTRIBUTE(LineToleranceEqual, bool);

	// used to set up the variance min / max calculation
	void SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax);

private:
	// line function that projects the objective function along 
	//		a given line
	CLineFunction m_lineFunction;

	// points to the line optimizer to be used
	// COptimizer *m_pLineOptimizer;

	// brent optimizer along the line function
	vnl_brent_minimizer m_optimizeBrent;

	// "statics" for the optimization routine
	CVectorN<> m_vGrad;
	CVectorN<> m_vGradPrev;
	CVectorN<> m_vDir;
	CVectorN<> m_vDirPrev;
	CVectorN<> m_vLambdaScaled;

	// flag to indicate adaptive variance calculation
	bool m_bCalcVar;

	// AV min and max
	REAL m_varMin;
	REAL m_varMax;

	// stores orthogonal basis for searched directions (used to calculate adaptive variance)
	CMatrixNxM<> m_mOrthoBasis;
	CMatrixNxM<> m_mSearchedDir;

	// stores the calculated AV
	CVectorN<> m_vAdaptVariance;

};	// class CConjGradOptimizer


#endif
