// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: TargetDVHSeries.cpp 608 2008-09-14 18:32:44Z dglane001 $
#include "StdAfx.h"
#include ".\targetdvhseries.h"

#include <Graph.h>

CTargetDVHSeries::CTargetDVHSeries(dH::KLDivTerm *pKLDT)
#ifdef USE_RTOPT
// : m_pKLDivTerm(pKLDT)
#endif
{
#ifdef USE_RTOPT
	SetForKLDivTerm(pKLDT);
	//m_pKLDivTerm->GetChangeEvent().AddObserver(this, 
	//	(dH::ListenerFunction) &CTargetDVHSeries::OnKLDTChanged);
#endif

	if (GetForKLDivTerm().IsNotNull())
		OnKLDTChanged(); // NULL, NULL);
}

CTargetDVHSeries::~CTargetDVHSeries(void)
{
}

const CMatrixNxM<>& 
	CTargetDVHSeries::GetDataMatrix()
{
	OnKLDTChanged(); // NULL, NULL);
	return CDataSeries::GetDataMatrix();
}

void CTargetDVHSeries::SetDataMatrix(const CMatrixNxM<>& mData)
{
	CDataSeries::SetDataMatrix(mData);

	CMatrixNxM<REAL> mTemp(mData.GetCols(), mData.GetRows());
	mTemp = mData;
	for (int nAt = 0; nAt < mData.GetCols(); nAt++)
	{
		mTemp[nAt][0] /= R(100.0);
		mTemp[nAt][1] /= R(100.0);
	}
#ifdef USE_RTOPT
	GetForKLDivTerm()->SetDVPs(mTemp);
#endif
}

void CTargetDVHSeries::OnKLDTChanged() // CObservableEvent * pEv, void * pVoid)
{
#ifdef USE_RTOPT
		// m_mData.Reshape(0, 2);

	if (GetForKLDivTerm().IsNull())
	{
		throw new exception("Didn't initialize");
	}

	// now draw the histogram
	const CMatrixNxM<>& mDVPs = GetForKLDivTerm()->GetDVPs();
	m_mData.Reshape(mDVPs.GetCols(), mDVPs.GetRows());
	m_mData = mDVPs;

//	m_mData.Reshape(arrBins.GetDim(), 2);
//	REAL sum = m_pHisto->GetRegion()->GetSum();
	// int nStart = -floor(m_pHisto->GetBinMinValue() / m_pHisto->GetBinWidth());
//	REAL binValue = m_pHisto->GetBinMinValue();
	for (int nAt = 0; /* nStart; */ nAt < m_mData.GetCols(); nAt++)
	{
		m_mData[nAt][0] *= R(100.0);
		m_mData[nAt][1] *= R(100.0);
		// AddDataPoint(C VectorD<2>(100.0 * binValue, 100.0 * arrBins[nAt] / sum));
		// AddDataPoint(C VectorD<2>
		//	(1000 * (nAt - nStart) / 256, arrBins[nAt] / sum * 4100.0));
//		binValue += m_pHisto->GetBinWidth();
	}	

	//if (m_pGraph)
	//{
	//	m_pGraph->AutoScale();
	//	m_pGraph->SetAxesMin(MakeVector<2>(0.0, 0.0));
	//	m_pGraph->Invalidate(TRUE);
	//}
#endif
}
