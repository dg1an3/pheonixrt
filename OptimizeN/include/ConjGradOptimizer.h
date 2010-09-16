// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ConjGradOptimizer.h 605 2008-09-14 18:06:23Z dglane001 $
#if !defined(CONJGRADOPTIMIZER_H)
#define CONJGRADOPTIMIZER_H

// base class includes
//#include "Optimizer.h"
#include <vnl/vnl_nonlinear_minimizer.h>
#include "ObjectiveFunction.h"

// subordinate brent optimizer
// #include "BrentOptimizer.h"

// the line function for the Brent optimizer
//#include "LineFunction.h"

//#include "MatrixNxM.h"


class DynamicCovarianceOptimizer;

typedef BOOL OptimizerCallback(DynamicCovarianceOptimizer *pOpt, void *pParam);


//////////////////////////////////////////////////////////////////////
class DynamicCovarianceOptimizer : public vnl_nonlinear_minimizer
{
public:
	// construct a gradient optimizer for an objective function
	DynamicCovarianceOptimizer(DynamicCovarianceCostFunction *pFunc);

	DeclareMember(LineOptimizerTolerance, REAL);

	// optimize the objective function
	// virtual const CVectorN<>& 
	vnl_nonlinear_minimizer::ReturnCodes minimize(vnl_vector<REAL>& vInit);

	// used to set up the variance min / max calculation
	void SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax);

	// holds the final value of the optimization
	DeclareMember(FinalValue, REAL);

	// holds the final value of the parameters for the minimum f
	DeclareMember(FinalParameter, vnl_vector<REAL>);

	// sets the callback function
	void SetCallback(OptimizerCallback *pCallback, void *pParam = NULL)
	{
		m_pCallbackFunc = pCallback;
		m_pCallbackParam = pParam;
	}

protected:
	void InitializeDynamicCovariance(int nDim);
	void UpdateDynamicCovariance();

private:
	// the objective function over which optimization is to occur
	DynamicCovarianceCostFunction *m_pCostFunction;

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

	// stores the callback info
	OptimizerCallback *m_pCallbackFunc;
	void *m_pCallbackParam;

};	// class DynamicCovarianceOptimizer


#endif
