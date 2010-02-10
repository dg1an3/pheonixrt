// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: ConjGradOptimizer.cpp 644 2009-11-05 17:46:57Z dglane001 $
#include "stdafx.h"

// the class definition
#include "ConjGradOptimizer.h"

#include <vector>
#include <algorithm>

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
		m_pLineOptimizer(&m_optimizeBrent),
		m_optimizeBrent(&m_lineFunction),
		m_LineToleranceEqual(true),
		m_bCalcVar(false)
{
	// set the Brent optimizer to use the gradient information
	m_optimizeBrent.SetUseGradientInfo(FALSE);

}	// CConjGradOptimizer::CConjGradOptimizer


///////////////////////////////////////////////////////////////////////////////
// CConjGradOptimizer::GetBrentOptimizer
// 
// returns the embedded line optimizer
///////////////////////////////////////////////////////////////////////////////
CBrentOptimizer& CConjGradOptimizer::GetBrentOptimizer()
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

	// set the tolerance for the line optimizer
	if (GetLineToleranceEqual())
	{
		m_pLineOptimizer->SetTolerance(GetTolerance());
	}
	LOG_EXPR(GetTolerance());

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
	for (m_nIteration = 0; (m_nIteration < ITER_MAX) && !bConvergence; m_nIteration++)
	{
		LOG(_T("Iteration %i"), m_nIteration);

		///////////////////////////////////////////////////////////////////////////////
		// line minimization

		BEGIN_LOG_SECTION(CConjGradOptimizer::Optimize!Line_Minimization);

		// set up the direction for the line minimization
		m_lineFunction.SetLine(m_vFinalParam, m_vDir); // m_vGrad);

		// now launch a line optimization
		REAL lambda = m_pLineOptimizer->Optimize(CBrentOptimizer::GetInitZero())[0];
		LOG_EXPR(lambda);

		// update the final parameter value
		m_vLambdaScaled.SetDim(m_lineFunction.GetDirection().GetDim());
		m_vLambdaScaled = m_lineFunction.GetDirection();
		m_vLambdaScaled *= lambda;
		m_vFinalParam += m_vLambdaScaled;
		LOG_EXPR_EXT(m_vFinalParam);

		// store the final value from the line optimizer
		REAL new_fv = m_pLineOptimizer->GetFinalValue();

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
				m_nIteration = -1;
				return m_vFinalParam;
			}
		}

		// are we calculating adaptive variance?
		if (m_bCalcVar)
		{
			// add direction to orthogonal basis
			m_mSearchedDir[m_nIteration] = m_vDir;
			m_mSearchedDir[m_nIteration].Normalize();
			m_mOrthoBasis[m_nIteration] = m_mSearchedDir[m_nIteration];

#define ORTHOGONALIZE
#ifndef ORTHOGONALIZE
			// and normalize the new basis vector
			m_mOrthoBasis[m_nIteration].Normalize();
#else
			// stores the projection vector
			// static 
				CVectorN<> vProj;
			vProj.SetDim(m_mOrthoBasis[m_nIteration].GetDim());

// #define ORTHO_NEW
#ifdef ORTHO_NEW


			// PREVIOUS
			//// now use GSO to make sure basis is orthogonal to already searched directions
			for (int nDir = 0; nDir < m_nIteration-1; nDir++)
			{
				REAL projScale = m_mOrthoBasis[m_nIteration] * m_mOrthoBasis[nDir];
				vProj = projScale * m_mOrthoBasis[nDir];
				m_mOrthoBasis[m_nIteration] -= vProj;
				ASSERT(IsApproxEqual(m_mOrthoBasis[m_nIteration] * m_mOrthoBasis[nDir], 0.0));
			}

			//populate rest of matrix with first guess

			//// now use GSO to make sure basis is orthogonal to already searched directions
			//std::vector<double> arrSearchedRowLengths(m_mOrthoBasis.GetRows());
			//for (int nRow = 0; nRow < m_mOrthoBasis.GetRows(); nRow++)
			//{
			//	arrSearchedRowLengths[nRow] = 0.0;
			//	for (int nCol = 0; nCol < m_nIteration-1; nCol++)
			//	{
			//		arrSearchedRowLengths[nRow] += m_mOrthoBasis[nCol][nRow] * m_mOrthoBasis[nCol][nRow];
			//	}
			//}

			//for (int nCol = m_nIteration; nCol < vInit.GetDim(); nCol++)
			//{
			//	m_mOrthoBasis[nCol].SetZero();
			//	std::vector<double>::iterator iterMin = std::min_element(arrSearchedRowLengths.begin(), arrSearchedRowLengths.end());
			//	m_mOrthoBasis[nCol][(int)(iterMin-arrSearchedRowLengths.begin())] = 1.0;
			//	(*iterMin) = 1.0;	// no longer min;

			//	// now use GSO to make sure basis is orthogonal to already searched directions
			//	for (int nDir = 0; nDir < nCol-1; nDir++)
			//	{
			//		REAL projScale = m_mOrthoBasis[nCol] * m_mOrthoBasis[nDir];
			//		vProj = projScale * m_mOrthoBasis[nDir];
			//		m_mOrthoBasis[nCol] -= vProj;
			//		ASSERT(IsApproxEqual(m_mOrthoBasis[nCol] * m_mOrthoBasis[nDir], 0.0));
			//	}
			//}
#else
			// now use GSO to make sure basis is orthogonal to already searched directions
			for (int nDir = m_nIteration-1; nDir >= 0; nDir--)
			{
				// static 
				CVectorN<> vOrtho;
				vOrtho.SetDim(m_mSearchedDir[nDir].GetDim());
				vOrtho = m_mSearchedDir[nDir];
				for (int nDirOrtho = nDir+1; nDirOrtho < m_nIteration; nDirOrtho++)
				{
					REAL projScale = vOrtho * m_mOrthoBasis[nDirOrtho];
					vProj = projScale * m_mOrthoBasis[nDirOrtho];
					vOrtho -= vProj;
				}
				m_mOrthoBasis[nDir] = vOrtho;
				m_mOrthoBasis[nDir].Normalize();
				ASSERT(IsApproxEqual(m_mOrthoBasis[m_nIteration] * m_mOrthoBasis[nDir], 0.0));
			}

			// now use GSO to make sure basis is orthogonal to already searched directions
			for (int nDir = m_nIteration+1; nDir < m_mOrthoBasis.GetCols(); nDir++)
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
				ASSERT(IsApproxEqual(m_mOrthoBasis[m_nIteration] * m_mOrthoBasis[nDir], 0.0));
			}
#endif
#endif

			// and normalize the new basis vector
			// m_mOrthoBasis[m_nIteration].Normalize();


			CMatrixNxM<REAL> mScaling = m_mOrthoBasis;
			mScaling.SetIdentity();
			for (int nScale = 0; nScale < m_vDir.GetDim(); nScale++)
			{
				REAL scale = 1.0;
				if (nScale < m_nIteration)
					scale = pow(1.5, nScale) / pow(1.5, m_nIteration);

				mScaling[nScale][nScale] = 1.0 / (scale * (m_varMax - m_varMin) + m_varMin);
			}

			CMatrixNxM<REAL> mOrthoBasisT = m_mOrthoBasis;
			mOrthoBasisT.Transpose();
			CMatrixNxM<REAL> covar = mOrthoBasisT * mScaling * m_mOrthoBasis;// m_mOrthoBasis * mScaling * mOrthoBasisT;
			for (int nDim = 0; nDim < m_vDir.GetDim(); nDim++)
			{
				double sum = 0.0;
				for (int nRow = 0; nRow < covar.GetRows(); nRow++)
					sum += covar[nDim][nRow];
				m_vAdaptVariance[nDim] = 1.0 / sum;
			}

#ifdef MANUAL_SUM
			for (int nDim = 0; nDim < m_vDir.GetDim(); nDim++)
			{
				// now calculate the variance due to searched subspace
				REAL projSearchedSq = 0.0;
				REAL projSearchedSq_scaled = 0.0;
				REAL projSearchedSq_scaled_max = 0.0;
				for (int nOrtho = 0; nOrtho < m_vDir.GetDim()/*m_nIteration*/; nOrtho++)
				{
					projSearchedSq += mOrthoBasisT[nOrtho][nDim]/*m_mOrthoBasis[nOrtho][nDim] * m_mOrthoBasis[nOrtho][nDim] */;

					REAL scale = 1.0;
					if (nOrtho < m_nIteration)
						scale = pow(2.0, nOrtho) / pow(2.0, m_nIteration);
					REAL projSearchedSq_scaled_element = 
						m_mOrthoBasis[nOrtho][nDim] * m_mOrthoBasis[nOrtho][nDim]
							/ (scale * (m_varMax - m_varMin) + m_varMin);
						if (fabs(projSearchedSq_scaled_max) < fabs(projSearchedSq_scaled_element))
							projSearchedSq_scaled_max = projSearchedSq_scaled_element;
					projSearchedSq_scaled += 
						mOrthoBasisT[nOrtho][nDim]/* m_mOrthoBasis[nOrtho][nDim] * m_mOrthoBasis[nOrtho][nDim] */
							/ (scale * (m_varMax - m_varMin) + m_varMin);
				}
				projSearchedSq = __min(projSearchedSq, 1.0);

				// now calculate factor for variance due to non-searched subspace
				REAL projNSSq = 1.0 - projSearchedSq;

				// and set the corresponding element of the variance vector
				/// m_vAdaptVariance[nDim] = 1.0 / (projSearchedSq / m_varMin + projNSSq / m_varMax);
				m_vAdaptVariance[nDim] = // 1.0 / (projSearchedSq_scaled_max); /// + projNSSq / m_varMax);
					1.0 / (projSearchedSq_scaled + projNSSq / m_varMax);
				if (m_vAdaptVariance[nDim] > m_varMax || m_vAdaptVariance[nDim] < m_varMin)
				{
					TRACE("projSearchedSq = %lf\n", projSearchedSq);
					TRACE("m_varMax = %lf\n", m_varMax);
					TRACE("projNSSq = %lf\n", projNSSq);
					TRACE("m_varMin = %lf\n", m_varMin);
					TRACE("m_vAdaptVariance[nDim] = %lf\n", m_vAdaptVariance[nDim]);
					m_vAdaptVariance[nDim] = __max(m_vAdaptVariance[nDim], m_varMin);
					m_vAdaptVariance[nDim] = __min(m_vAdaptVariance[nDim], m_varMax);
				}				
			}
#endif

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

		m_nIteration = -1;
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
