// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ConjGradOptimizer.cpp 644 2009-11-05 17:46:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "ConjGradOptimizer.h"

#include <vnl/algo/vnl_brent_minimizer.h>

#include <vector>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
class LineProjectionFunction : public vnl_cost_function
{
public:
	LineProjectionFunction(vnl_cost_function& projectedFunction)
		: m_projectedFunction(projectedFunction) { }

	DeclareMember(Point, vnl_vector<REAL>);
	DeclareMember(Direction, vnl_vector<REAL>);

	virtual double f(vnl_vector<double> const& x)
	{
		m_vEvalPoint = GetDirection();
		m_vEvalPoint *= x[0];
		m_vEvalPoint += GetPoint();

		// return evaluate of projected function
		return m_projectedFunction.f(m_vEvalPoint);
	}

private:
	// the function to project
	vnl_cost_function& m_projectedFunction;

	// temporary store of evaluation point
	mutable vnl_vector<REAL> m_vEvalPoint;
};


///////////////////////////////////////////////////////////////////////////////
// constants used to optimize
///////////////////////////////////////////////////////////////////////////////

// maximum iterations
const int ITER_MAX = 500;		

// z-epsilon -- small number to protect against fractional accuracy for 
//		a minimum that happens to be exactly zero.
const REAL ZEPS = (REAL) 1.0e-10;	

///////////////////////////////////////////////////////////////////////////////
DynamicCovarianceOptimizer::DynamicCovarianceOptimizer(DynamicCovarianceCostFunction *pFunc)
	: // COptimizer(pFunc)
	m_pCostFunction(pFunc)
	, m_bCalcVar(false)
{
}	// CConjGradOptimizer::CConjGradOptimizer

///////////////////////////////////////////////////////////////////////////////
// bool 
//	CConjGradOptimizer::minimize(vnl_vector<REAL>& v)
// const CVectorN<>& 
vnl_nonlinear_minimizer::ReturnCodes 
	DynamicCovarianceOptimizer::minimize(vnl_vector<REAL>& vInit)
{
	LineProjectionFunction m_lineFunction(*m_pCostFunction);
	vnl_brent_minimizer m_optimizeBrent(m_lineFunction);
	m_optimizeBrent.set_x_tolerance(GetLineOptimizerTolerance());

	// initialize, if we are calculating adaptive variance?
	InitializeDynamicCovariance(vInit.size());

	// store the initial parameter vector
	// m_vFinalParam.SetDim(vInit.GetDim());
	m_FinalParameter = vInit; // const_cast<CVectorN<>&>(vInit).GetVnlVector();

	// set the dimension of the current direction
	m_vGrad.set_size(vInit.size());		// TODO: is this needed (check logic of compute)

	// evaluate the function at the initial point, storing
	//		the gradient as the current direction
	m_pCostFunction->compute(m_FinalParameter, &m_FinalValue, &m_vGrad);
	m_vGrad *= R(-1.0);

	// if we are too short,
	if (m_vGrad.magnitude() < 1e-8)
	{
		Log(_T("Gradient too small -- adding length"));
		RandomVector(get_x_tolerance(), &m_vGrad[0], m_vGrad.size());
	}

	// set the initial (steepest descent) direction
	m_vDir = m_vGrad;

	BOOL bConvergence = FALSE;
	ReturnCodes retCode = FAILED_TOO_MANY_ITERATIONS;
	for (num_iterations_ = 0; (num_iterations_ < ITER_MAX) && !bConvergence; num_iterations_++)
	{
		///////////////////////////////////////////////////////////////////////////////
		// line minimization

		// set up the direction for the line minimization
		m_lineFunction.SetPoint(m_FinalParameter);
		m_lineFunction.SetDirection(m_vDir);

		// now launch a line optimization
		REAL lambda = m_optimizeBrent.minimize(0);

		// update the final parameter value
		m_vLambdaScaled = m_lineFunction.GetDirection();
		m_vLambdaScaled *= lambda;
		m_FinalParameter += m_vLambdaScaled;

		// store the final value from the line optimizer
		REAL new_fv = m_optimizeBrent.f_at_last_minimum();

		// test for convergence on line minimalization
		bConvergence = (2.0 * fabs(m_FinalValue - new_fv) 
			<= get_x_tolerance() * (fabs(m_FinalValue) + fabs(new_fv) + ZEPS));

		// store the previous lambda value
		m_FinalValue = new_fv;

		// need to call-back?
		if (m_pCallbackFunc)
		{
			if (!(*m_pCallbackFunc)(this, m_pCallbackParam)) 
			{
				// request to terminate
				// num_iterations_ = -1;
				//vInit = m_vFinalParam;
				retCode = FAILED_USER_REQUEST;
				break;
				//return retCode; // m_vFinalParam;
			}
		}

		// are we calculating adaptive variance?
		UpdateDynamicCovariance();

		///////////////////////////////////////////////////////////////////////////////
		// Update Direction
		
		// compute denominator for gamma
		REAL gg = dot_product(m_vGrad, m_vGrad);
		bConvergence = bConvergence || (gg == 0.0);

		// must have performed at least one full iteration
		if (!bConvergence)
		{
			// store gradient for 
			m_vGradPrev = m_vGrad;

			// compute the gradient at the current parameter value
			m_pCostFunction->gradf(m_FinalParameter, m_vGrad);
			m_vGrad *= -1.0;

			// compute numerator for gamma (Polak-Ribiera formula)
			REAL dgg = dot_product(m_vGrad, m_vGrad) - dot_product(m_vGradPrev, m_vGrad);

			// otherwise, update the direction
			m_vDir *= dgg / gg;
			m_vDir += m_vGrad;
		}
		else
		{
			retCode = CONVERGED_XTOL;
		}
	}

	//if (!bConvergence)
	//{
	//	retCode = FAILED_TOO_MANY_ITERATIONS;
	//}

	vInit = m_FinalParameter;

	// return the last parameter vector
	//m_vFinalParamTemp.SetDim(m_vFinalParam.size());
	//m_vFinalParamTemp.GetVnlVector() = m_vFinalParam;
	return retCode; // m_vFinalParamTemp;

}	// CConjGradOptimizer::Optimize


//////////////////////////////////////////////////////////////////////////////
void 
	DynamicCovarianceOptimizer::SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax)
	// used to set up the variance min / max calculation
{
	// set the flag
	m_bCalcVar = bCalcVar;

	// store min / max
	m_varMin = varMin;
	m_varMax = varMax;

	// set up for the objective function
	m_pCostFunction->SetAdaptiveVariance(&m_vAdaptVariance, m_varMin, m_varMax);

}	// CConjGradOptimizer::SetAdaptiveVariance


//////////////////////////////////////////////////////////////////////////////
void 
	DynamicCovarianceOptimizer::InitializeDynamicCovariance(int nDim)
{
	if (!m_bCalcVar)
		return;

	// initialize orthogonal basis matrix
	m_mOrthoBasis.set_size(nDim, nDim); 
	m_mOrthoBasis.set_identity();
	m_mSearchedDir.set_size(nDim, nDim);
	m_mSearchedDir.set_identity();

	m_vAdaptVariance.SetDim(nDim);
	for (int nN = 0; nN < m_vAdaptVariance.GetDim(); nN++)
	{
		m_vAdaptVariance[nN] = m_varMax;
	}

	m_pCostFunction->SetAdaptiveVariance(&m_vAdaptVariance, m_varMin, m_varMax);
}

//////////////////////////////////////////////////////////////////////////////
void 
	DynamicCovarianceOptimizer::UpdateDynamicCovariance()
{
	if (!m_bCalcVar)
		return;

	// add direction to orthogonal basis
	vnl_vector<REAL> vDirNorm = m_vDir;
	vDirNorm.normalize();
	m_mSearchedDir.set_column(num_iterations_, vDirNorm);
	m_mOrthoBasis.set_column(num_iterations_, m_mSearchedDir.get_column(num_iterations_));

	// stores the projection vector
	vnl_vector<REAL> vProj;
	vProj.set_size(m_mOrthoBasis.rows());

	// now use GSO to make sure basis is orthogonal to already searched directions
	for (int nDir = num_iterations_-1; nDir >= 0; nDir--)
	{
		vnl_vector<REAL> vOrtho;
		vOrtho.set_size(m_mSearchedDir.rows());
		vOrtho = m_mSearchedDir.get_column(nDir);
		for (int nDirOrtho = nDir+1; nDirOrtho < num_iterations_; nDirOrtho++)
		{					
			REAL projScale = dot_product(vOrtho, m_mOrthoBasis.get_column(nDirOrtho));
			vOrtho -= projScale * m_mOrthoBasis.get_column(nDirOrtho);
		}
		vOrtho.normalize(); 
		m_mOrthoBasis.set_column(nDir, vOrtho);
	}

	// now use GSO to make sure basis is orthogonal to already searched directions
	for (int nDir = num_iterations_+1; nDir < m_mOrthoBasis.columns(); nDir++)
	{
		vnl_vector<REAL> vOrtho;
		vOrtho.set_size(m_mSearchedDir.rows());
		vOrtho = m_mSearchedDir.get_column(nDir);
		for (int nDirOrtho = nDir-1; nDirOrtho >= 0; nDirOrtho--)
		{
			REAL projScale = dot_product(vOrtho, m_mOrthoBasis.get_column(nDirOrtho));
			vOrtho -= projScale * m_mOrthoBasis.get_column(nDirOrtho);
		}
		vOrtho.normalize();
		m_mOrthoBasis.set_column(nDir, vOrtho);
	}

	vnl_matrix<REAL> mScaling;
	mScaling.set_size(m_mOrthoBasis.rows(), m_mOrthoBasis.columns());
	mScaling.fill(0.0);
	for (int nScale = 0; nScale < m_vDir.size(); nScale++)
	{
		REAL scale = 1.0;
		if (nScale < num_iterations_)
			scale = pow(4.0, nScale) / pow(4.0, (double) num_iterations_);

		mScaling(nScale, nScale) = 1.0 / (scale * (m_varMax - m_varMin) + m_varMin);
	}

	vnl_matrix<REAL> mOrthoBasisT = m_mOrthoBasis.transpose();
	vnl_matrix<REAL> covar = mOrthoBasisT;
	covar *= mScaling;
	covar *= m_mOrthoBasis;
	for (int nDim = 0; nDim < m_vDir.size(); nDim++)
	{
		double sum = 0.0;
		for (int nRow = 0; nRow < covar.rows(); nRow++)
			sum += covar(nRow, nDim);
		m_vAdaptVariance[nDim] = 1.0 / sum;
	}

	// now reset the final value, using the new AV vector
	m_FinalValue = m_pCostFunction->f(m_FinalParameter);
}
