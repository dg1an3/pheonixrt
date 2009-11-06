// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: HistogramDataSeries.cpp 608 2008-09-14 18:32:44Z dglane001 $
#include "StdAfx.h"

#include <HistogramDataSeries.h>
#include <Graph.h>
#include ".\include\histogramdataseries.h"

////////////////////////////////////////////////////////////////////////////
	CHistogramDataSeries::CHistogramDataSeries(CHistogram *pHisto)
		: m_pHistogram(pHisto)
		, m_bRecalcCurve(true)
{
#ifdef USE_RTOPT
	AddObserver(&m_pHistogram->GetChangeEvent(), this, 
		&CHistogramDataSeries::OnHistogramChanged);
#endif

	// OnHistogramChanged(NULL, NULL);

}	// CHistogramDataSeries::CHistogramDataSeries

////////////////////////////////////////////////////////////////////////////
	CHistogramDataSeries::~CHistogramDataSeries(void)
{
}	// CHistogramDataSeries::~CHistogramDataSeries

////////////////////////////////////////////////////////////////////////////
const CMatrixNxM<>& 
	CHistogramDataSeries::GetDataMatrix() const
	// recalculates the data matrix based on current histogram, if flagged
{
#ifdef USE_RTOPT
	if (m_bRecalcCurve)
	{
		// now draw the histogram
		const CVectorN<>& arrBins = GetHistogram()->GetCumBins();

		m_mData.Reshape(arrBins.GetDim(), 2);
		REAL sum = GetSum<VOXEL_REAL>(GetHistogram()->GetRegion());
		REAL binValue = GetHistogram()->GetBinMinValue();
		for (int nAt = 0; nAt < m_mData.GetCols(); nAt++)
		{
			m_mData[nAt][0] = R(100.0) * binValue;
			m_mData[nAt][1] = R(100.0) * arrBins[nAt] / sum;
			binValue += GetHistogram()->GetBinWidth();
			if (IsApproxEqual(m_mData[nAt][1], 0.0))
			{
				m_mData.Reshape(nAt+1, 2);
			}
		}	

		m_bRecalcCurve = false;
	}
#endif
	return CDataSeries::GetDataMatrix();

}	// CHistogramDataSeries::GetDataMatrix

////////////////////////////////////////////////////////////////////////////
void 
	CHistogramDataSeries::OnHistogramChanged(CObservableEvent *, void *)
{
	// flag recalc
	m_bRecalcCurve = true;

	// propagate change
	GetChangeEvent().Fire();

}	// CHistogramDataSeries::OnHistogramChanged


/*
////////////////////////////////////////////////////////////////////////////
CHistogram * CHistogramDataSeries::GetHistogram(void)
{
	return m_pHisto;
}
*/