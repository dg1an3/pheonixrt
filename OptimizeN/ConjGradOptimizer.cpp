// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ConjGradOptimizer.cpp 644 2009-11-05 17:46:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "ConjGradOptimizer.h"

#include <vector>
#include <algorithm>

class LineProjection_1dfun : public vnl_cost_function
{
public:
	vnl_cost_function* f_;
	unsigned int n_;
	vnl_vector<REAL> x0_;
	vnl_vector<REAL> dx_;
	vnl_vector<REAL> tmpx_;

	LineProjection_1dfun(int n, vnl_cost_function* f)
		: vnl_cost_function(1)
		, f_(f)
		, n_(n)
		, x0_(n)
		, dx_(n)
		, tmpx_(n) {}

	void init(vnl_vector<double> const& x0, vnl_vector<double> const& dx)
	{
		x0_ = x0;
		dx_ = dx;
		//assert(x0.size() == n_);
		//assert(dx.size() == n_);
	}

	double f(const vnl_vector<double>& x)
	{
		uninit(x[0], tmpx_);
		double e = f_->f(tmpx_);
		return e;
	}

	void uninit(double lambda, vnl_vector<double>& out)
	{
		for (unsigned int i = 0; i < n_; ++i)
			out[i] = x0_[i] + lambda * dx_[i];
	}
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
// CConjGradOptimizer::CConjGradOptimizer
// 
// construct a gradient optimizer for an objective function
///////////////////////////////////////////////////////////////////////////////
CConjGradOptimizer::CConjGradOptimizer(CObjectiveFunction *pFunc)
	: COptimizer(pFunc),
		m_lineFunction(pFunc),
		// m_pLineOptimizer(&m_optimizeBrent),
		m_optimizeBrent(m_lineFunction),
		//m_LineToleranceEqual(true),
		m_bCalcVar(false)
{
	// set the Brent optimizer to use the gradient information
	// m_optimizeBrent.SetUseGradientInfo(FALSE);

}	// CConjGradOptimizer::CConjGradOptimizer


///////////////////////////////////////////////////////////////////////////////
// CConjGradOptimizer::GetBrentOptimizer
// 
// returns the embedded line optimizer
///////////////////////////////////////////////////////////////////////////////
vnl_brent_minimizer& CConjGradOptimizer::GetBrentOptimizer()
{
	return m_optimizeBrent;

}	// CConjGradOptimizer::GetBrentOptimizer


///////////////////////////////////////////////////////////////////////////////
// CConjGradOptimizer::Optimize
// 
// performs the optimization given the initial value vector
// is IMPAC really just a bunch of cancer profiteers?
///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& CConjGradOptimizer::Optimize(const CVectorN<>& vInit)
{
	USES_CONVERSION;

	BEGIN_LOG_SECTION(CConjGradOptimizer::Optimize);
	LOG_EXPR_EXT(vInit);

//	// set the tolerance for the line optimizer
//	if (GetLineToleranceEqual())
//	{
////		m_pLineOptimizer->SetTolerance(GetTolerance());
//	}
//	LOG_EXPR(GetTolerance());

	// are we calculating adaptive variance?
	if (m_bCalcVar)
	{
		// initialize orthogonal basis matrix
		m_mOrthoBasis.Reshape(vInit.GetDim() * 2, vInit.GetDim());
		m_mSearchedDir.Reshape(vInit.GetDim() * 2, vInit.GetDim());

		m_vAdaptVariance.SetDim(vInit.GetDim());
		for (int nN = 0; nN < m_vAdaptVariance.GetDim(); nN++)
		{
			m_vAdaptVariance[nN] = m_varMax;
		}

		m_pFunc->SetAdaptiveVariance(&m_vAdaptVariance, m_varMin, m_varMax);
	}

	// store the initial parameter vector
	m_vFinalParam.SetDim(vInit.GetDim());
	m_vFinalParam = vInit;

	// set the dimension of the current direction
	m_vGrad.SetDim(vInit.GetDim());

	// evaluate the function at the initial point, storing
	//		the gradient as the current direction
	m_finalValue = (*m_pFunc)(m_vFinalParam, &m_vGrad);
	m_vGrad *= R(-1.0);
	LOG_EXPR(m_finalValue);
	LOG_EXPR_EXT(m_vGrad);

	// if we are too short,
	if (m_vGrad.GetLength() < 1e-8) // GetTolerance())
	{
		LOG(_T("Gradient too small -- adding length"));
		RandomVector(GetTolerance(), &m_vGrad[0], m_vGrad.GetDim());
	}

	// set the initial (steepest descent) direction
	m_vDirPrev.SetDim(m_vGrad.GetDim());
	m_vDir.SetDim(m_vGrad.GetDim());
	m_vDirPrev = m_vDir = m_vGrad;

	BOOL bConvergence = FALSE;
	for (num_iterations_ = 0; (num_iterations_ < ITER_MAX) && !bConvergence; num_iterations_++)
	{
		// LOG(_T("Iteration %i"), m_nIteration);

		///////////////////////////////////////////////////////////////////////////////
		// line minimization

		BEGIN_LOG_SECTION(CConjGradOptimizer::Optimize!Line_Minimization);

		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, m_vDir); // m_vGrad);
		// of1d.init(m_vFinalParam.GetVnlVector(), m_vDir.GetVnlVector());

		// now launch a line optimization
		REAL lambda = // m_optimizeBrent.Optimize(CBrentOptimizer::GetInitZero())[0];
			m_optimizeBrent.minimize(0);
		LOG_EXPR(lambda);

		// update the final parameter value
		m_vLambdaScaled.SetDim(m_lineFunction.GetDirection().GetDim());
		m_vLambdaScaled = m_lineFunction.GetDirection();
		m_vLambdaScaled *= lambda;
		m_vFinalParam += m_vLambdaScaled;
		LOG_EXPR_EXT(m_vFinalParam);

		// store the final value from the line optimizer
		REAL new_fv = m_optimizeBrent.f_at_last_minimum(); // ->GetFinalValue();

		// test for convergence on line minimalization
		bConvergence = (2.0 * fabs(m_finalValue - new_fv) 
			<= GetTolerance() * (fabs(m_finalValue) + fabs(new_fv) + ZEPS));

		// store the previous lambda value
		m_finalValue = new_fv;
		LOG_EXPR(m_finalValue);

		END_LOG_SECTION();	// Line Minimization

		// need to call-back?
		if (m_pCallbackFunc)
		{
			if (!(*m_pCallbackFunc)(this, m_pCallbackParam)) 
			{
				// request to terminate
				num_iterations_ = -1;
				return m_vFinalParam;
			}
		}

		// are we calculating adaptive variance?
		if (m_bCalcVar)
		{
			// add direction to orthogonal basis
			m_mSearchedDir[num_iterations_] = m_vDir;
			m_mSearchedDir[num_iterations_].Normalize();
			m_mOrthoBasis[num_iterations_] = m_mSearchedDir[num_iterations_];

			// stores the projection vector
			// static 
			CVectorN<> vProj;
			vProj.SetDim(m_mOrthoBasis[num_iterations_].GetDim());

			// now use GSO to make sure basis is orthogonal to already searched directions
			for (int nDir = num_iterations_-1; nDir >= 0; nDir--)
			{
				// static 
				CVectorN<> vOrtho;
				vOrtho.SetDim(m_mSearchedDir[nDir].GetDim());
				vOrtho = m_mSearchedDir[nDir];
				for (int nDirOrtho = nDir+1; nDirOrtho < num_iterations_; nDirOrtho++)
				{
					REAL projScale = vOrtho * m_mOrthoBasis[nDirOrtho];
					vProj = projScale * m_mOrthoBasis[nDirOrtho];
					vOrtho -= vProj;
				}
				m_mOrthoBasis[nDir] = vOrtho;
				m_mOrthoBasis[nDir].Normalize();
				ASSERT(IsApproxEqual(m_mOrthoBasis[num_iterations_] * m_mOrthoBasis[nDir], 0.0));
			}

			// now use GSO to make sure basis is orthogonal to already searched directions
			for (int nDir = num_iterations_+1; nDir < m_mOrthoBasis.GetCols(); nDir++)
			{
				// static 
				CVectorN<> vOrtho;
				vOrtho.SetDim(m_mSearchedDir[nDir].GetDim());
				vOrtho = m_mSearchedDir[nDir];
				for (int nDirOrtho = nDir-1; nDirOrtho >= 0; nDirOrtho--)
				{
					REAL projScale = vOrtho * m_mOrthoBasis[nDirOrtho];
					vProj = projScale * m_mOrthoBasis[nDirOrtho];
					vOrtho -= vProj;
				}
				m_mOrthoBasis[nDir] = vOrtho;
				m_mOrthoBasis[nDir].Normalize();
				ASSERT(IsApproxEqual(m_mOrthoBasis[num_iterations_] * m_mOrthoBasis[nDir], 0.0));
			}

			CMatrixNxM<REAL> mScaling = m_mOrthoBasis;
			mScaling.SetIdentity();
			for (int nScale = 0; nScale < m_vDir.GetDim(); nScale++)
			{
				REAL scale = 1.0;
				if (nScale < num_iterations_)
					scale = pow(1.5, nScale) / pow(1.5, (double) num_iterations_);

				mScaling[nScale][nScale] = 1.0 / (scale * (m_varMax - m_varMin) + m_varMin);
			}

			CMatrixNxM<REAL> mOrthoBasisT = m_mOrthoBasis;
			mOrthoBasisT.Transpose();
			CMatrixNxM<REAL> covar = mOrthoBasisT * mScaling * m_mOrthoBasis;
			for (int nDim = 0; nDim < m_vDir.GetDim(); nDim++)
			{
				double sum = 0.0;
				for (int nRow = 0; nRow < covar.GetRows(); nRow++)
					sum += covar[nDim][nRow];
				m_vAdaptVariance[nDim] = 1.0 / sum;
			}

			// now reset the final value, using the new AV vector
			m_finalValue = (*m_pFunc)(m_vFinalParam);
		}

		///////////////////////////////////////////////////////////////////////////////
		// Update Direction

		BEGIN_LOG_SECTION(CConjGradOptimizer::Optimize!Update_Direction);
		
		// compute denominator for gamma
		REAL gg = m_vGrad * m_vGrad;
		bConvergence = bConvergence || (gg == 0.0);

		// must have performed at least one full iteration
		if (!bConvergence)
		{
			// store gradient for 
			m_vGradPrev.SetDim(m_vGrad.GetDim());
			m_vGradPrev = m_vGrad;

			// compute the gradient at the current parameter value
			(*m_pFunc)(m_vFinalParam, &m_vGrad);
			m_vGrad *= -1.0;
			LOG_EXPR_EXT(m_vGrad);

			// compute numerator for gamma (Polak-Ribiera formula)
			REAL dgg = m_vGrad * m_vGrad - m_vGradPrev * m_vGrad;

			// otherwise, update the direction
			m_vDir *= dgg / gg;
			m_vDir += m_vGrad;
			LOG_EXPR_EXT(m_vDir);

			BEGIN_LOG_ONLY(Test Direction Conjugacy);
			
			// output orthogonality conditions
			LOG_EXPR(m_vGrad * m_vGradPrev);
			LOG_EXPR(m_vGrad * m_vDirPrev);

			// output vector lengths for comparison
			LOG_EXPR(m_vDir.GetLength());
			LOG_EXPR(m_vGrad.GetLength());

			// store direction for next iteration
  			m_vDirPrev = m_vDir;

			END_LOG_SECTION();	// Test Conjugacy
		}
		else
		{
			// TRACE("Convergence in %i iterations\n", m_nIteration);
		}

		END_LOG_SECTION();	// Update Direction
	}

	if (!bConvergence)
	{
		// Too many iterations
		LOG(_T("Too many iterations"));

		num_iterations_ = -1;
	}

	END_LOG_SECTION();	// CConjGradOptimizer

	// return the last parameter vector
	return m_vFinalParam;

}	// CConjGradOptimizer::Optimize


//////////////////////////////////////////////////////////////////////////////
void 
	CConjGradOptimizer::SetAdaptiveVariance(bool bCalcVar, REAL varMin, REAL varMax)
	// used to set up the variance min / max calculation
{
	// set the flag
	m_bCalcVar = bCalcVar;

	// store min / max
	m_varMin = varMin;
	m_varMax = varMax;

	// set up for the objective function
	m_pFunc->SetAdaptiveVariance(&m_vAdaptVariance, m_varMin, m_varMax);

}	// CConjGradOptimizer::SetAdaptiveVariance
