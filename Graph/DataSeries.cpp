//
//
#include "StdAfx.h"

#include <DataSeries.h>
#include <Graph.h>

namespace dH {

////////////////////////////////////////////////////////////////////////////
DataSeries::DataSeries()
		: m_pGraph(NULL)
		, m_pObject(NULL)
		, m_Color(RGB(255, 0, 0))
		, m_HasHandles(false)
		, m_PenStyle(PS_SOLID)
{ 
}

////////////////////////////////////////////////////////////////////////////
DataSeries::~DataSeries()
{
}

////////////////////////////////////////////////////////////////////////////
int 
	DataSeries::GetHandleHit(const CPoint& point, int nSize, CPoint *pOfs)
	// returns index of handle, if point is at a hit
{
	if (GetHasHandles())
	{
		//const CMatrixNxM<>& mData = GetDataMatrix();
		//for (int nDataPoint = 0; nDataPoint < mData.GetCols(); 
		//		nDataPoint++)
		//{
		//	CPoint ptOfs = m_pGraph->ToPlotCoord(MakeVector<2>(mData[nDataPoint][0], mData[nDataPoint][1]));
		//	ptOfs -= point;
		//	if (abs(ptOfs.x) < nSize 
		//		&& abs(ptOfs.y) < nSize)
		//	{
		//		// store offset, if need be
		//		if (pOfs != NULL)
		//		{
		//			(*pOfs) = ptOfs;
		//		}

		//		// and return the hit point
		//		return nDataPoint;
		//	}
		//}
	}

	return -1;
}

}