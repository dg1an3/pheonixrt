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
	//m_pHistogram->GetChangeEvent().AddObserver(this, 
	//// AddObserver(&m_pHistogram->GetChangeEvent(), this, 
	//	(dH::ListenerFunction) &CHistogramDataSeries::OnHistogramChanged);
#endif

	UseForAutoScale = false;

	// OnHistogramChanged(NULL, NULL);

}	// CHistogramDataSeries::CHistogramDataSeries

////////////////////////////////////////////////////////////////////////////
	CHistogramDataSeries::~CHistogramDataSeries(void)
{
}	// CHistogramDataSeries::~CHistogramDataSeries

////////////////////////////////////////////////////////////////////////////
const CMatrixNxM<>& 
	CHistogramDataSeries::GetDataMatrix()
	// recalculates the data matrix based on current histogram, if flagged
{
	if (m_pHistogram->GetUpdateMTime() < GetUpdateMTime())
		return CDataSeries::GetDataMatrix();

#ifdef USE_RTOPT
	if (/*m_bRecalcCurve 
		|| */true)
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
		this->DataHasBeenGenerated(); // Modified();
	}
#endif
	return CDataSeries::GetDataMatrix();

}	// CHistogramDataSeries::GetDataMatrix

////////////////////////////////////////////////////////////////////////////
void 
	CHistogramDataSeries::OnHistogramChanged() // CObservableEvent *, void *)
{
	// flag recalc
	m_bRecalcCurve = true;

	// propagate change
	// GetChangeEvent().Fire();
	if (m_pGraph)
		m_pGraph->OnDataSeriesChanged(); // NULL, NULL);

}	// CHistogramDataSeries::OnHistogramChanged


/*
////////////////////////////////////////////////////////////////////////////
CHistogram * CHistogramDataSeries::GetHistogram(void)
{
	return m_pHisto;
}
*/