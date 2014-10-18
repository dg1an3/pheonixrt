// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: KLDivTerm.cpp 647 2009-11-05 21:52:59Z dglane001 $
#include "stdafx.h"

#include "KLDivTerm.h"

namespace dH
{

const REAL EPS = (REAL) 1e-5;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
KLDivTerm::KLDivTerm(Structure *pStructure, REAL weight)
	: VOITerm(pStructure, weight)
		, m_bRecompTarget(true)
		, m_bReconvolve(true)
		, m_bTargetCrossEntropy(false)
{
	// default interval
	SetInterval(0.0, 0.01, 1.0, FALSE);

	// initialize flag
	int nTargetCrossEntropy = 
		::AfxGetApp()->GetProfileInt(_T("KLDivTerm"), _T("TargetCrossEntropy"), 0);
	m_bTargetCrossEntropy = (nTargetCrossEntropy != 0);

	// store value back to registry
	::AfxGetApp()->WriteProfileInt(_T("KLDivTerm"), _T("TargetCrossEntropy"), nTargetCrossEntropy);
		
	// set up to receive binning change events
	//GetHistogram()->GetBinningChangeEvent().AddObserver(this, 
	//	(ListenerFunction) &KLDivTerm::OnHistogramBinningChange);

}	// KLDivTerm::KLDivTerm


///////////////////////////////////////////////////////////////////////////////
KLDivTerm::~KLDivTerm()
{
}	// KLDivTerm::~KLDivTerm

///////////////////////////////////////////////////////////////////////////////
void 
	KLDivTerm::UpdateFrom(const VOITerm *otherTerm)
{
	VOITerm::UpdateFrom(otherTerm);
	ASSERT(otherTerm->GetVOI() == this->GetVOI());

	const KLDivTerm * otherKLDT = static_cast<const KLDivTerm *>(otherTerm);
	SetDVPs(otherKLDT->GetDVPs());
}

//////////////////////////////////////////////////////////////////////
const CMatrixNxM<>& 
	KLDivTerm::GetDVPs() const
	// gets dose-volume points for the target histogram;
	// mDVPs   =	|x x x x ... |
	//				|y y y y     |
{
	return m_mDVPs;
}

//////////////////////////////////////////////////////////////////////
void 
	KLDivTerm::SetDVPs(const CMatrixNxM<>& mDVPs)
	// sets dose-volume points for the target histogram;
	// mDVPs   =	|x x x x ... |
	//				|y y y y     |
{
	// store the points
	// NOTE: don't use true parameter, because of assign-to-self from Serialize
	m_mDVPs.Reshape(mDVPs.GetCols(), mDVPs.GetRows());
	m_mDVPs = mDVPs;

	// check that cumulative DVPs start at 100%
	ASSERT(IsApproxEqual(m_mDVPs[0][1], R(1.0)));
	ASSERT(IsApproxEqual(m_mDVPs[mDVPs.GetCols()-1][1], R(0.0)));

	// trigger recomp of target bins
	m_bRecompTarget = true;

	// triggers recalc of GBins
	m_bReconvolve = true;

	// notify of change
	// GetChangeEvent().Fire();
	this->Modified();
	// this->DataHasBeenGenerated();

}	// KLDivTerm::SetDVPs

//////////////////////////////////////////////////////////////////////
void 
	KLDivTerm::SetInterval(REAL low, REAL high, REAL fraction, BOOL bMid)
	// sets the target histgram to an interval
{
	CMatrixNxM<> mDVPs;
	mDVPs.Reshape(bMid ? 4 : 2, 2);
	mDVPs[0][0] = low;
	mDVPs[0][1] = 1.0;
	if (bMid)
	{
		mDVPs[1][0] = low - (low - high) / 3.0;
		mDVPs[1][1] = 2.0 / 3.0;
		mDVPs[2][0] = low - 2.0 * (low - high) / 3.0;
		mDVPs[2][1] = 1.0 / 3.0;
		mDVPs[3][0] = high;
		mDVPs[3][1] = 0.0;
	}
	else
	{
		mDVPs[1][0] = high;
		mDVPs[1][1] = 0.0;
	}

	// set up the DVPs
	SetDVPs(mDVPs);

}	// KLDivTerm::SetInterval

///////////////////////////////////////////////////////////////////////////////
REAL 
	KLDivTerm::GetMinDose(void) const
	// returns minimum target dose to the structure
{
	REAL doseValue = m_mDVPs[0][0];
	return doseValue; 

}	// KLDivTerm::GetMinDose

///////////////////////////////////////////////////////////////////////////////
REAL 
	KLDivTerm::GetMaxDose(void) const
	// returns maximum target dose to the structure
{
	REAL doseValue = m_mDVPs[m_mDVPs.GetCols()-1][0];
	return doseValue;

}	// KLDivTerm::GetMaxDose

///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	KLDivTerm::GetTargetBins() const
	// accessor for target bins (after calling SetDVPs)
{ 
	if (m_bRecompTarget)
	{
		// get the main volume
		REAL high = m_mDVPs[m_mDVPs.GetCols()-1][0];
		m_vTargetBins.SetDim(GetHistogram()->GetBinForValue(high) + 1);
		m_vTargetBins.SetZero();

		// now for each interval
		for (int nAtInterval = 0; nAtInterval < m_mDVPs.GetCols()-1; nAtInterval++)
		{
			REAL interMin = m_mDVPs[nAtInterval][0];
			REAL interMax = m_mDVPs[nAtInterval+1][0];

			// compute slope (= per-bin value within this interval)
			REAL interSlope = (m_mDVPs[nAtInterval][1] - m_mDVPs[nAtInterval+1][1])
									/ (interMax - interMin); 
			interSlope /= (REAL) GetHistogram()->GetBinWidth();

			// now set bin values in the interval
			for (int nAtBin = GetHistogram()->GetBinForValue(interMin); 
				nAtBin <= GetHistogram()->GetBinForValue(interMax); nAtBin++)
			{
				m_vTargetBins[nAtBin] = interSlope;
			}
		}

		// ok done
		m_bRecompTarget = false;
	}

	return m_vTargetBins; 

}	// KLDivTerm::GetTargetBins

///////////////////////////////////////////////////////////////////////////////
const CVectorN<>& 
	KLDivTerm::GetTargetGBins() const 
	// returns convolved target bins
{ 
	if (m_bReconvolve)
	{
		// now convolve GBins
		m_vTargetGBins.SetDim(GetTargetBins().GetDim() 
				+ (int) (GBINS_BUFFER * sqrt(GetHistogram()->GetGBinVarMax()) / GetHistogram()->GetBinWidth()));

		CVectorN<> vTargetGBinsVarMin;
		vTargetGBinsVarMin.SetDim(m_vTargetGBins.GetDim());

		// perform convolve
		GetHistogram()->ConvGauss(GetTargetBins(), GetHistogram()->GetKernelVarMin(), vTargetGBinsVarMin);
		GetHistogram()->ConvGauss(GetTargetBins(), GetHistogram()->GetKernelVarMax(), m_vTargetGBins);
		vTargetGBinsVarMin *= 0.5;
		m_vTargetGBins *= 0.5;
		m_vTargetGBins += vTargetGBinsVarMin;

		// normalize
		REAL sum = 0.0;
		ITERATE_VECTOR(m_vTargetGBins, nAt, sum += m_vTargetGBins[nAt]);
		m_vTargetGBins *= R(1.0) / sum;

		m_bReconvolve = false;
	}

	return m_vTargetGBins; 

}	// KLDivTerm::GetTargetGBins

///////////////////////////////////////////////////////////////////////////////
REAL 
	KLDivTerm::Eval(CVectorN<> *pvGrad, const CArray<BOOL, BOOL>& arrInclude)
	// evaluates the term (and optionally gradient)
{
	// trigger update of targets
	OnHistogramBinningChange(); // NULL, NULL);

	REAL sum = 0.0;

	// get the calculated histogram bins
	const CVectorN<>& calcGPDF = GetHistogram()->GetGBins();

	// get the target bins
	const CVectorN<>& targetGPDF = GetTargetGBins();

	if (!m_bTargetCrossEntropy)
	{	
		for (int nAtBin = 0; nAtBin < calcGPDF.GetDim(); nAtBin++)
		{
			if (nAtBin < targetGPDF.GetDim())
			{
				// ASSERT(targetGPDF[nAtBin] >= 0.0);
				sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS);
			}
			else
			{
				sum += calcGPDF[nAtBin] * log(calcGPDF[nAtBin] / (EPS) + EPS);	
				ASSERT(_finite(sum));
			}
		}
	}
	else // if (m_bTargetCrossEntropy)
	{
//#define IPP_CALC_KLDIV
#ifdef IPP_CALC_KLDIV
		if (m_vCalc_EPS.GetDim() < calcGPDF.GetDim())
		{
			m_vCalc_EPS.SetDim(calcGPDF.GetDim());
		}
		m_vCalc_EPS.SetZero();
		SumValues<REAL>(&m_vCalc_EPS[0], &calcGPDF[0], EPS, calcGPDF.GetDim());

		if (m_vTarget_div_Calc.GetDim() < targetGPDF.GetDim())
		{
			m_vTarget_div_Calc.SetDim(targetGPDF.GetDim());
		}
		m_vTarget_div_Calc.SetZero();

		// CAREFUL about the order of operands for DivValues
		DivValues<REAL>(&m_vTarget_div_Calc[0], &m_vCalc_EPS[0], 
			&targetGPDF[0], __min(calcGPDF.GetDim(), targetGPDF.GetDim()));
		if (calcGPDF.GetDim() < targetGPDF.GetDim())
		{
			DivValues<REAL>(&m_vTarget_div_Calc[calcGPDF.GetDim()], 
				&targetGPDF[calcGPDF.GetDim()], EPS, targetGPDF.GetDim() - calcGPDF.GetDim());
		}

		if (m_vTarget_div_Calc_EPS.GetDim() < targetGPDF.GetDim())
		{
			m_vTarget_div_Calc_EPS.SetDim(targetGPDF.GetDim());
		}
		m_vTarget_div_Calc_EPS.SetZero();
		SumValues<REAL>(&m_vTarget_div_Calc_EPS[0], &m_vTarget_div_Calc[0], EPS, targetGPDF.GetDim());

		if (m_vLogTarget_div_Calc.GetDim() < targetGPDF.GetDim())
		{
			m_vLogTarget_div_Calc.SetDim(targetGPDF.GetDim());
		}
		m_vLogTarget_div_Calc.SetZero();
		// ::ippsLn_64f(&m_vTarget_div_Calc_EPS[0], &m_vLogTarget_div_Calc[0], targetGPDF.GetDim());
		for (int nN = 0; nN < targetGPDF.GetDim(); nN++)
			m_vLogTarget_div_Calc[nN] = log(m_vTarget_div_Calc_EPS[nN]);
		MultValues<REAL>(&m_vLogTarget_div_Calc[0], &targetGPDF[0], targetGPDF.GetDim());

		// now sum all values
		// ::ippsSum_64f(&m_vLogTarget_div_Calc[0], targetGPDF.GetDim(), &sum); 
		sum = 0.0;
		for (int nN = 0; nN < targetGPDF.GetDim(); nN++)
			sum += m_vLogTarget_div_Calc[nN];
#else
		for (int nAtBin = 0; nAtBin < targetGPDF.GetDim(); nAtBin++)
		{
			if (nAtBin < calcGPDF.GetDim())
			{
				sum += targetGPDF[nAtBin] 
					* log(targetGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS) + EPS);
			}
			else
			{
				sum += targetGPDF[nAtBin] 
					* log(targetGPDF[nAtBin] / (EPS) + EPS);
			}
		}
#endif
	}

	// if a gradient is needed
	if (pvGrad)
	{
#ifdef IPP_CALC_KLDIV

		// if cross entropy, prepare some values
		if (m_bTargetCrossEntropy)
		{
			if (m_v_dx_Target_div_Calc.GetDim() < targetGPDF.GetDim())
			{
				m_v_dx_Target_div_Calc.SetDim(targetGPDF.GetDim());
			}
			m_v_dx_Target_div_Calc.SetZero();

			// divide by derivative of log term (target / calc)
			DivValues<REAL>(&m_v_dx_Target_div_Calc[0], 
				&m_vTarget_div_Calc_EPS[0], &m_vTarget_div_Calc[0], targetGPDF.GetDim());

			// multiply to form target / (calc * calc) (chain rule) term
			DivValues<REAL>(&m_v_dx_Target_div_Calc[0], 
				&m_vCalc_EPS[0], &m_v_dx_Target_div_Calc[0], __min(calcGPDF.GetDim(), targetGPDF.GetDim()));
			if (calcGPDF.GetDim() < targetGPDF.GetDim())
			{
				DivValues<REAL>(&m_v_dx_Target_div_Calc[calcGPDF.GetDim()], 
					&m_v_dx_Target_div_Calc[calcGPDF.GetDim()], EPS, targetGPDF.GetDim() - calcGPDF.GetDim());
			}

			// multiply (scale) by target GPDF
			MultValues<REAL>(&m_v_dx_Target_div_Calc[0], 
				&targetGPDF[0], &m_v_dx_Target_div_Calc[0], targetGPDF.GetDim());
		}
#endif
		const int n_dVolCount = GetHistogram()->Get_dVolumeCount();
		pvGrad->SetDim(n_dVolCount);
		pvGrad->SetZero();

		// iterate over the dVolumes
		for (int nAt_dVol = 0; nAt_dVol < n_dVolCount; nAt_dVol++)
		{
			if (!arrInclude[nAt_dVol])
			{
				continue;
			}

			// get the dGPDF distribution
			const CVectorN<>& arrCalc_dGPDF = GetHistogram()->Get_dGBins(nAt_dVol);

			ASSERT(arrCalc_dGPDF.GetDim() >= calcGPDF.GetDim());
			if (!m_bTargetCrossEntropy)
			{
				for (int nAtBin = 0; nAtBin < __max(targetGPDF.GetDim(), arrCalc_dGPDF.GetDim()); nAtBin++)
				{
					if (nAtBin < calcGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						// u * v'
						(*pvGrad)[nAt_dVol] += 
							calcGPDF[nAtBin] 
								* R(1.0) / (calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS) 
									* arrCalc_dGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS);

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(calcGPDF[nAtBin] / (targetGPDF[nAtBin] + EPS) + EPS);
					}
					else if (nAtBin < calcGPDF.GetDim())
					{
						// u * v'
						(*pvGrad)[nAt_dVol] += 
							calcGPDF[nAtBin] 
								* R(1.0) / (calcGPDF[nAtBin] / (EPS) + EPS) 
									* arrCalc_dGPDF[nAtBin] / (EPS);

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(calcGPDF[nAtBin] / (EPS) + EPS);

					}
					else if (nAtBin < arrCalc_dGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						// u * v' = 0

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(targetGPDF[nAtBin] + EPS);
					}
					else if (nAtBin < arrCalc_dGPDF.GetDim())
					{
						// u * v' = 0

						// + u' * v
						(*pvGrad)[nAt_dVol] += 
							arrCalc_dGPDF[nAtBin]
								* log(EPS);
					} 
				}
			}
			else // if (m_bTargetCrossEntropy)
			{
#ifdef IPP_CALC_KLDIV
			
				int nFinalSize = __min(targetGPDF.GetDim(), arrCalc_dGPDF.GetDim());
				if (m_v_dVol_Target.GetDim() < nFinalSize)
				{
					m_v_dVol_Target.SetDim(nFinalSize);
				}
				m_v_dVol_Target.SetZero();
				MultValues<REAL>(&m_v_dVol_Target[0], 
					&arrCalc_dGPDF[0], &m_v_dx_Target_div_Calc[0], nFinalSize);
				::ippsSum_64f(&m_v_dVol_Target[0], nFinalSize, &(*pvGrad)[nAt_dVol]);

#else
				for (int nAtBin = 0; nAtBin < __max(targetGPDF.GetDim(), arrCalc_dGPDF.GetDim()); nAtBin++)
				{
					if (nAtBin < calcGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						(*pvGrad)[nAt_dVol] += 
							targetGPDF[nAtBin] 
							* arrCalc_dGPDF[nAtBin]	// don't forget negate
							* (targetGPDF[nAtBin] / ((calcGPDF[nAtBin] + EPS) * (calcGPDF[nAtBin] + EPS)))
							/ (targetGPDF[nAtBin] / (calcGPDF[nAtBin] + EPS) + EPS);
					}
					else if (nAtBin < arrCalc_dGPDF.GetDim() && nAtBin < targetGPDF.GetDim())
					{
						(*pvGrad)[nAt_dVol] += 
							targetGPDF[nAtBin] 
							* arrCalc_dGPDF[nAtBin]	// don't forget negate
							* (targetGPDF[nAtBin] / ((EPS) * (EPS)))
							/ (targetGPDF[nAtBin] / (EPS) + EPS);
					} 
				}
#endif
			}
		}

		// now adjust or integral 
		(*pvGrad) *= GetHistogram()->GetBinWidth();

		// and weight
		(*pvGrad) *= GetWeight();
	}

	// now adjust or integral 
	sum *= GetHistogram()->GetBinWidth();

	// and weight
	sum *= GetWeight();

	return sum;

}	// KLDivTerm::Eval

///////////////////////////////////////////////////////////////////////////////
VOITerm *
	KLDivTerm::Clone() 
{
	KLDivTerm *pCopy = new KLDivTerm(this->GetVOI(), GetWeight());
	pCopy->UpdateFrom(this);
	return pCopy;

}	// KLDivTerm::Clone

///////////////////////////////////////////////////////////////////////////////
void 
	KLDivTerm::OnHistogramBinningChange() // CObservableEvent *pSource, void *pValue)
	// flags recalc when histogram changes
{
	// recompute target
	m_bRecompTarget = true;

	// and reconvolve as well
	m_bReconvolve = true;

}	// KLDivTerm::OnHistogramChange

}	// namespace dH