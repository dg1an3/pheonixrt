// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ConjGradOptimizer.h 605 2008-09-14 18:06:23Z dglane001 $
#if !defined(CONJGRADOPTIMIZER_H)
#define CONJGRADOPTIMIZER_H

// base class includes
#include "Optimizer.h"

// subordinate brent optimizer
// #include "BrentOptimizer.h"

// the line function for the Brent optimizer
//#include "LineFunction.h"

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
	CConjGradOptimizer(DynamicCovarianceCostFunction/*CObjectiveFunction*/ *pFunc);

	DeclareMember(LineOptimizerTolerance, REAL);

	// optimize the objective function
	virtual const CVectorN<>& Optimize(const CVectorN<>& vInit);

	// used to set up the variance min / max calculation
	void SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax);

protected:
	void InitializeDynamicCovariance(int nDim);
	void UpdateDynamicCovariance();

private:
	// "statics" for the optimization routine
	vnl_vector<REAL> m_vGrad;
	vnl_vector<REAL> m_vGradPrev;
	vnl_vector<REAL> m_vDir;
	vnl_vector<REAL> m_vLambdaScaled;

	// flag to indicate adaptive variance calculation
	bool m_bCalcVar;

	// AV min and max
	REAL m_varMin;
	REAL m_varMax;

	// stores orthogonal basis for searched directions (used to calculate adaptive variance)
	vnl_matrix<REAL> m_mOrthoBasis;
	vnl_matrix<REAL> m_mSearchedDir;

	// stores the calculated AV
	CVectorN<> m_vAdaptVariance;

};	// class CConjGradOptimizer


#endif
