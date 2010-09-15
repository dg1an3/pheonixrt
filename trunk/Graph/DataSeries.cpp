// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: DataSeries.cpp 608 2008-09-14 18:32:44Z dglane001 $
#include "StdAfx.h"
#include "DataSeries.h"
#include <Graph.h>
#include <ItkUtils.h>	// for MakeVector function

////////////////////////////////////////////////////////////////////////////
	CDataSeries::CDataSeries(void)
		: m_pGraph(NULL)
		, m_pObject(NULL)
		, m_Color(RGB(255, 0, 0))
		, m_HasHandles(false)
		, m_PenStyle(PS_SOLID)
		, UseForAutoScale(true)
{ 
}	// CDataSeries::CDataSeries


////////////////////////////////////////////////////////////////////////////
	CDataSeries::~CDataSeries(void)
{
}	// CDataSeries::~CDataSeries


////////////////////////////////////////////////////////////////////////////
void 
	CDataSeries::SetColor(const COLORREF& color)
{
	m_Color = color;
	// GetChangeEvent().Fire();
	if (m_pGraph)
		m_pGraph->OnDataSeriesChanged(); // NULL, NULL);

}	// CDataSeries::SetColor

////////////////////////////////////////////////////////////////////////////
const CMatrixNxM<>& 
	CDataSeries::GetDataMatrix()
		// accessor for the data series data
{
	return m_mData;

}	// CDataSeries::GetDataMatrix()

////////////////////////////////////////////////////////////////////////////
void 
	CDataSeries::SetDataMatrix(const CMatrixNxM<>& mData)
{
	// copy the data
	m_mData.Reshape(mData.GetCols(), mData.GetRows());
	m_mData = mData;

	// notify
	// GetChangeEvent().Fire();
	if (m_pGraph)
		m_pGraph->OnDataSeriesChanged(); // NULL, NULL);

}	// CDataSeries::SetDataMatrix

////////////////////////////////////////////////////////////////////////////
void 
	CDataSeries::AddDataPoint(const itk::Vector<REAL,2>& vDataPt)
{
	// TODO: fix Reshape to correct this problem
	// ALSO TODO: CMatrixNxM serialization
	CMatrixNxM<> mTemp(m_mData.GetCols()+1, 2);
	for (int nAt = 0; nAt < m_mData.GetCols(); nAt++)
	{
		mTemp[nAt][0] = m_mData[nAt][0];
		mTemp[nAt][1] = m_mData[nAt][1];
	}

	// set the data point
	mTemp[mTemp.GetCols()-1][0] = vDataPt[0];
	mTemp[mTemp.GetCols()-1][1] = vDataPt[1];

	// call this, so that virtual over-rides can do their job
	SetDataMatrix(mTemp);

	// DON'T call change event, because SetDataMatrix calls it
	// GetChangeEvent().Fire();

}	// CDataSeries::AddDataPoint


////////////////////////////////////////////////////////////////////////////
int 
	CDataSeries::GetHandleHit(const CPoint& point, int nSize, CPoint *pOfs)
	// returns index of handle, if point is at a hit
{
	if (GetHasHandles())
	{
		const CMatrixNxM<>& mData = GetDataMatrix();
		for (int nDataPoint = 0; nDataPoint < mData.GetCols(); 
				nDataPoint++)
		{
			CPoint ptOfs = m_pGraph->ToPlotCoord(MakeVector<2>(mData[nDataPoint][0], mData[nDataPoint][1]));
			ptOfs -= point;
			if (abs(ptOfs.x) < nSize 
				&& abs(ptOfs.y) < nSize)
			{
				// store offset, if need be
				if (pOfs != NULL)
				{
					(*pOfs) = ptOfs;
				}

				// and return the hit point
				return nDataPoint;
			}
		}
	}

	return -1;

}	// CCastVectorD<2>(mData[nDataPoint])
