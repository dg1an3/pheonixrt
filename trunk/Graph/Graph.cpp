// Copyright (C) 2nd Messenger Systems - U. S. Patent 7,369,645
// $Id: Graph.cpp 608 2008-09-14 18:32:44Z dglane001 $
#include "stdafx.h"

#include <float.h>

#include "Graph.h"

#include <ItkUtils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int DEFAULT_MARGIN = 50;
const COLORREF AXIS_COLOR = RGB(255, 255, 255);

/////////////////////////////////////////////////////////////////////////////
// CGraph::CGraph
/////////////////////////////////////////////////////////////////////////////
CGraph::CGraph()
	: m_pDragSeries(NULL)
		, m_bDragging(FALSE)
		, m_bShowLegend(false)
		, m_TruncateZeroTail(true)
{
	SetMargins(DEFAULT_MARGIN, DEFAULT_MARGIN / 2, DEFAULT_MARGIN / 2, 
		DEFAULT_MARGIN);

}	// CGraph::CGraph

/////////////////////////////////////////////////////////////////////////////
// CGraph::~CGraph
//
// accessors for data series
/////////////////////////////////////////////////////////////////////////////
CGraph::~CGraph()
{
	RemoveAllDataSeries(true);	// be sure and delete them as well

}	// CGraph::~CGraph

/////////////////////////////////////////////////////////////////////////////
// CGraph::GetDataSeriesCount
//
// accessors for data series
/////////////////////////////////////////////////////////////////////////////
int CGraph::GetDataSeriesCount()
{
	return (int) m_arrDataSeries.GetSize();

}	// CGraph::GetDataSeriesCount

/////////////////////////////////////////////////////////////////////////////
// CGraph::GetDataSeriesAt
/////////////////////////////////////////////////////////////////////////////
CDataSeries *CGraph::GetDataSeriesAt(int nAt)
{
	return (CDataSeries *) m_arrDataSeries.GetAt(nAt);
}

/////////////////////////////////////////////////////////////////////////////
// CGraph::AddDataSeries
/////////////////////////////////////////////////////////////////////////////
void CGraph::AddDataSeries(CDataSeries::Pointer pSeries)
{
	pSeries->m_pGraph = this;
	// AddObserver(&pSeries->GetChangeEvent(), this, &CGraph::OnDataSeriesChanged);

	m_arrDataSeriesPtr.push_back(pSeries);
	m_arrDataSeries.Add(pSeries);
	OnDataSeriesChanged(); // NULL, NULL);

}	// CGraph::AddDataSeries


/////////////////////////////////////////////////////////////////////////////
// CGraph::RemoveDataSeries
/////////////////////////////////////////////////////////////////////////////
void CGraph::RemoveDataSeries(int nAt, bool bDelete)
{
	//if (bDelete)
	//{
	//	delete m_arrDataSeries[nAt];
	//}

	m_arrDataSeries.RemoveAt(nAt);

}	// CGraph::RemoveDataSeries

/////////////////////////////////////////////////////////////////////////////
// CGraph::RemoveAllDataSeries
/////////////////////////////////////////////////////////////////////////////
void CGraph::RemoveAllDataSeries(bool bDelete)
{
	if (bDelete)
	{
		m_arrDataSeriesPtr.clear();
		//for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
		//{
		//	delete m_arrDataSeries[nAt];
		//}
	}

	m_arrDataSeries.RemoveAll();

}	// CGraph::RemoveAllDataSeries

/////////////////////////////////////////////////////////////////////////////
// CGraph::SetMargins
//
// sets the graph margins
/////////////////////////////////////////////////////////////////////////////
void 
	CGraph::SetMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_arrMargins[0] = nLeft;
	m_arrMargins[1] = nTop;
	m_arrMargins[2] = nRight;
	m_arrMargins[3] = nBottom;

}	// CGraph::SetMargins

/////////////////////////////////////////////////////////////////////////////
// CGraph::AutoScale
//
// determines graph range and tick marks
/////////////////////////////////////////////////////////////////////////////
void 
	CGraph::AutoScale()
{
	// accumulate mins and maxes
	REAL minInit[2] = {FLT_MAX, FLT_MAX};
	SetAxesMin(GraphCoord(minInit)); // C VectorD<2>(FLT_MAX, FLT_MAX));
	REAL maxInit[2] = {-FLT_MAX, -FLT_MAX};
	SetAxesMax(GraphCoord(maxInit)); // C VectorD<2>(-FLT_MAX, -FLT_MAX));

	// find the min / max for mantissa and abcsisca
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		if (!m_arrDataSeries[nAt]->UseForAutoScale)
			continue;

		const CMatrixNxM<>& mData = m_arrDataSeries[nAt]->GetDataMatrix();
		for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
		{
			SetAxesMin(MakeVector<2>(
				__min(GetAxesMin()[0], mData[nAtPoint][0]),
				__min(GetAxesMin()[1], mData[nAtPoint][1])));

/*(			if (!GetTruncateZeroTail() 
				|| mData[nAtPoint][1] > 0.0) */
			{
				SetAxesMax(MakeVector<2>(
					__max(GetAxesMax()[0], mData[nAtPoint][0]*1.2),
					__max(GetAxesMax()[1], mData[nAtPoint][1]*1.2)));
			} 
/*			else 
			{
				SetAxesMax(C VectorD<2>(
					GetAxesMax()[0],
					__max(GetAxesMax()[1], mData[nAtPoint][1])));
			} */
		} 
	}

	// set major tick marks for X & Y
	for (int nD = 0; nD < 2; nD++)
	{
		// compute tick marks
		m_AxesMajor[nD] = 10.0;		// default to 10.0
		REAL absMax = __max(fabs(m_AxesMin[nD]), fabs(m_AxesMax[nD]));
		if (absMax > R(0.0))
		{
			m_AxesMajor[nD] = pow(R(10.0), floor(log10(absMax + DEFAULT_EPSILON) + R(0.1)) - R(1.0));
		}
	}

	// set max / min for X & Y
	for (int nD = 0; nD < 2; nD++)
	{
		m_AxesMinor[nD] = m_AxesMajor[nD] / R(2.0);
		m_AxesMax[nD] = m_AxesMajor[nD] * (ceil(m_AxesMax[nD] / m_AxesMajor[nD]));
		m_AxesMin[nD] = m_AxesMajor[nD] * (floor(m_AxesMin[nD] / m_AxesMajor[nD]));
	}

	// now adjust for plot coordinates
	CPoint ptOrigin = ToPlotCoord(MakeVector<2>(0.0, 0.0));
	CPoint ptStep = ToPlotCoord(m_AxesMajor) - ptOrigin;
	while (ptStep.x != 0 && abs(ptStep.x) < 12)
	{
		m_AxesMajor[0] *= 2.0;
		ptStep = ToPlotCoord(m_AxesMajor) - ptOrigin;
	}
	while (ptStep.y != 0 && abs(ptStep.y) < 12)
	{
		m_AxesMajor[1] *= 2.0;
		ptStep = ToPlotCoord(m_AxesMajor) - ptOrigin;
	}

	// reset max / min for X & Y
	for (int nD = 0; nD < 2; nD++)
	{
		m_AxesMinor[nD] = m_AxesMajor[nD] / R(2.0);
		m_AxesMax[nD] = m_AxesMajor[nD] * (ceil(m_AxesMax[nD] / m_AxesMajor[nD]));
		m_AxesMin[nD] = m_AxesMajor[nD] * (floor(m_AxesMin[nD] / m_AxesMajor[nD]));
	}

}	// CGraph::AutoScale

/////////////////////////////////////////////////////////////////////////////
// CGraph::SetLegendLUT
/////////////////////////////////////////////////////////////////////////////
void CGraph::SetLegendLUT(CArray<COLORREF, COLORREF>&  arrLUT, REAL win, REAL level)
{
	m_arrLegendLUT.Copy(arrLUT);
	m_window = win;
	m_level = level;

	m_bShowLegend = true;

}	// CGraph::SetLegendLUT



BEGIN_MESSAGE_MAP(CGraph, CWnd)
	//{{AFX_MSG_MAP(CGraph)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGraph message handlers

////////////////////////////////////////////////////////////////////////////
void 
	CGraph::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CRect rect;
	GetClientRect(&rect);

	if (m_dib.GetSize().cx != rect.Width())
	{
		m_dib.DeleteObject();
		HBITMAP bm = ::CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
		m_dib.Attach(bm);
	}
	dcMem.SelectObject(m_dib);

	// draw the axes
	COLORREF bkColor = RGB(16, 16, 16);
	CBrush brushBack(bkColor);
	dcMem.SelectObject(&brushBack);
	dcMem.Rectangle(rect);

	dcMem.SetBkColor(bkColor);

	if (m_arrDataSeries.GetSize() != 0)
	{
		rect.DeflateRect(m_arrMargins[0], m_arrMargins[1], 
			m_arrMargins[2], m_arrMargins[3]);

		// draw minor ticks and grids
		DrawMinorAxes(&dcMem, rect);

		// draw major ticks, grids, and labels
		DrawMajorAxes(&dcMem, rect);

		// draw curves
		DrawSeries(&dcMem, rect);

		// draw legend
		if (m_bShowLegend)
		{
			DrawLegend(&dcMem, rect);
		}

		// draw rectangle
		CPen pen(PS_SOLID, 2, RGB(0, 0, 0));
		dcMem.SelectObject(&pen);
		dcMem.SelectStockObject(HOLLOW_BRUSH);
		dcMem.Rectangle(rect);
	}

	GetClientRect(&rect);
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

	// Do not call CWnd::OnPaint() for painting messages
}

/////////////////////////////////////////////////////////////////////////////
void 
	CGraph::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bDragging)
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));

		CMatrixNxM<> mData = m_pDragSeries->GetDataMatrix();
		mData[m_nDragPoint][0] = FromPlotCoord(point - m_ptDragOffset)[0];
		mData[m_nDragPoint][1] = FromPlotCoord(point - m_ptDragOffset)[1];

		if (m_nDragPoint > 0)
		{
			mData[m_nDragPoint][0] = __max(mData[m_nDragPoint][0], 
				mData[m_nDragPoint-1][0]);
			mData[m_nDragPoint][1] = __min(mData[m_nDragPoint][1], 
				mData[m_nDragPoint-1][1]);
		}

		if (m_nDragPoint < mData.GetCols()-1)
		{
			mData[m_nDragPoint][0] = __min(mData[m_nDragPoint][0], 
				mData[m_nDragPoint+1][0]);
			mData[m_nDragPoint][1] =__max(mData[m_nDragPoint][1], 
					mData[m_nDragPoint+1][1]);
		}


		if (m_nDragPoint == 0)
			// && m_pDragSeries->GetMonotonicDirection() == -1)
		{
			mData[m_nDragPoint][1] = 100.0;
		}

		if (m_nDragPoint == mData.GetCols()-1)
		{
			mData[m_nDragPoint][1] = 0.0;
		}
		m_pDragSeries->SetDataMatrix(mData);

		Invalidate();
		return;
	}

	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		if (m_arrDataSeries[nAt]->GetHandleHit(point, 10) != -1)
		{
			::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
			return;
		}
	}

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

/////////////////////////////////////////////////////////////////////////////
void 
	CGraph::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bDragging)
	{
 		for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
		{
			m_nDragPoint = m_arrDataSeries[nAt]->GetHandleHit(point, 10, &m_ptDragOffset);
			if (m_nDragPoint != -1)
			{
				m_bDragging = TRUE;
				m_pDragSeries = m_arrDataSeries[nAt];
				::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
				return;
			}
		}
	}
	m_pDragSeries = NULL;

	// CWnd::OnLButtonDown(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
void 
	CGraph::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDragging = FALSE;
	m_pDragSeries = NULL;

	// if we have dragged, then re-scale
	// aAutoScale();

	// CWnd::OnLButtonUp(nFlags, point);
}


////////////////////////////////////////////////////////////////////////////
// drawing helpers


////////////////////////////////////////////////////////////////////////////
void 
	CGraph::DrawMinorAxes(CDC * pDC, const CRect& rect)
	// draws the minor ticks and grids
{
	CPen penMinorTicks(PS_SOLID, 1, AXIS_COLOR); // RGB(0, 0, 0));
	CPen penMinorGrids(PS_DOT, 1, RGB(176, 176, 176));

	itk::Vector<REAL,2> vAtTick;
	vAtTick[1] = GetAxesMin()[1];
	for (vAtTick[0] = GetAxesMin()[0]; vAtTick[0] < GetAxesMax()[0]; vAtTick[0] += GetAxesMinor()[0])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMinorTicks);
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.bottom + 5);

		pDC->SelectObject(&penMinorGrids);	
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.top);
	}

	vAtTick[0] = GetAxesMin()[0];
	for (vAtTick[1] = GetAxesMin()[1]; vAtTick[1] < GetAxesMax()[1]; vAtTick[1] += GetAxesMinor()[1])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMinorTicks);
		pDC->MoveTo(ptTick); 
		pDC->LineTo(rect.left - 5, ptTick.y);

		pDC->SelectObject(&penMinorGrids);
		pDC->MoveTo(ptTick); 
		pDC->LineTo(rect.right, ptTick.y);
	}
}

////////////////////////////////////////////////////////////////////////////
void 
	CGraph::DrawMajorAxes(CDC * pDC, const CRect& rect)
	// draw major ticks and grids
{
	CPen penMajorTicks(PS_SOLID, 1, AXIS_COLOR); // RGB(0, 0, 0));
	CPen penMajorGrids(PS_DOT, 1, RGB(160, 160, 160));
	pDC->SetTextColor(AXIS_COLOR);

	// stores size of text (for vert centering)
	CSize sz = pDC->GetTextExtent(_T("Test"));
	pDC->SetTextAlign(TA_CENTER);

	itk::Vector<REAL,2> vAtTick;
	vAtTick[1] = GetAxesMin()[1];
	for (vAtTick[0] = GetAxesMin()[0]; vAtTick[0] < GetAxesMax()[0]; vAtTick[0] += GetAxesMajor()[0])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMajorTicks);
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.bottom + 10);

		CString strLabel;
		strLabel.Format((GetAxesMajor()[0] >= 1.0) ? _T("%0.0lf") : _T("%6.3lf"), vAtTick[0]);
		pDC->TextOut(ptTick.x, rect.bottom + 15, strLabel);

		pDC->SelectObject(&penMajorGrids);
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.top);
	}

	pDC->SetTextAlign(TA_RIGHT);

	int nPrevTop = INT_MAX;
	vAtTick[0] = GetAxesMin()[0];
	for (vAtTick[1] = GetAxesMin()[1]; vAtTick[1] < GetAxesMax()[1]; vAtTick[1] += GetAxesMajor()[1])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMajorTicks);
		pDC->MoveTo(ptTick);
		pDC->LineTo(rect.left - 10, ptTick.y);

		if (ptTick.y + sz.cy / 2 < nPrevTop)
		{
			CString strLabel;
			strLabel.Format((GetAxesMajor()[1] >= 1.0) ? _T("%0.0lf") : _T("%6.3lf"), vAtTick[1]);
			pDC->TextOut(rect.left - 15, ptTick.y - sz.cy / 2, strLabel);
			nPrevTop = ptTick.y - sz.cy / 2;
		}

		pDC->SelectObject(&penMajorGrids);
		pDC->MoveTo(ptTick);
		pDC->LineTo(rect.right, ptTick.y);
	}
}

////////////////////////////////////////////////////////////////////////////
void 
	CGraph::DrawSeries(CDC * pDC, const CRect& rect)
{
	for (int nAt = 0; nAt < m_arrDataSeries.GetSize(); nAt++)
	{
		CDataSeries *pSeries = (CDataSeries *)m_arrDataSeries[nAt];

		CPen pen(pSeries->GetPenStyle() /* PS_SOLID */, 1 /* 2 */, pSeries->GetColor());
		// CPen *pOldPen = 
		pDC->SelectObject(&pen);

		const CMatrixNxM<>& mData = pSeries->GetDataMatrix();
		if (mData.GetCols() == 0)
		{
			continue;
		}
		pDC->MoveTo(ToPlotCoord(MakeVector<2>(mData[0][0], mData[0][1])));
		for (int nAtPoint = 1; nAtPoint < mData.GetCols(); nAtPoint++)
		{
			if (mData[nAtPoint][0] < GetAxesMin()[0]
				|| mData[nAtPoint][0] > GetAxesMax()[0]
				|| mData[nAtPoint][1] < GetAxesMin()[1]
				|| mData[nAtPoint][1] > GetAxesMax()[1])
			{
				// continue;
			}


			if (mData[nAtPoint][0] >= 0.0)
			{
				pDC->LineTo(ToPlotCoord(MakeVector<2>(mData[nAtPoint][0], mData[nAtPoint][1])));
			}
			else
			{
				pDC->MoveTo(ToPlotCoord(MakeVector<2>(mData[nAtPoint][0], mData[nAtPoint][1])));
			}
		}

		CPen penHandle(PS_SOLID, 1, AXIS_COLOR);
		pDC->SelectObject(&penHandle);
		if (pSeries->GetHasHandles())
		{
			CRect rectHandle(0, 0, 5, 5);
			for (int nAtPoint = 0; nAtPoint < mData.GetCols(); nAtPoint++)
			{
				rectHandle.OffsetRect(ToPlotCoord(MakeVector<2>(mData[nAtPoint][0], mData[nAtPoint][1]))
					- rectHandle.CenterPoint());
				pDC->Rectangle(rectHandle);
			}
		}

		// pDC->SelectObject(pOldPen);
	}
}


////////////////////////////////////////////////////////////////////////////
void 
	CGraph::DrawLegend(CDC * pDC, const CRect& rect)
{
	CRect rectLegend(rect);
	rectLegend.bottom = rectLegend.top - 2;
	rectLegend.top -= 12;

	REAL max_legend_value = R(m_arrLegendLUT.GetSize());

	for (int nAtX = rectLegend.left; nAtX < rectLegend.right; nAtX++)
	{
		// compute x coord
		double x = (double) (nAtX - rectLegend.left) / (double) rectLegend.Width() 
			* (GetAxesMax()[0] - GetAxesMin()[0]);
		x /= 100.0;

		REAL pix_value = max_legend_value * m_window * (R(x) - m_level) + max_legend_value / R(2.0);

		// scale to 0..255
		pix_value = __min(pix_value, max_legend_value - 1.0f);
		pix_value = __max(pix_value, 0.0f);

		int colorIndex = (int) __min(pix_value, max_legend_value - 1.0f);
		CPen penLegend(PS_SOLID, 1, m_arrLegendLUT[colorIndex]);
		pDC->SelectObject(&penLegend);

		pDC->MoveTo(nAtX, rectLegend.bottom - 1);
		pDC->LineTo(nAtX, rectLegend.top);
	}

	CPen pen(PS_SOLID, 2, RGB(0, 0, 0));
	pDC->SelectObject(&pen);
	pDC->SelectStockObject(HOLLOW_BRUSH);
	pDC->Rectangle(rectLegend);
}

////////////////////////////////////////////////////////////////////////////
CPoint 
	CGraph::ToPlotCoord(const GraphCoord& vCoord)
{
	CRect rect;
	GetClientRect(&rect);

	rect.DeflateRect(m_arrMargins[0], m_arrMargins[1], 
		m_arrMargins[2], m_arrMargins[3]);
		
	CPoint pt;
	pt.x = rect.left + (int)((REAL) rect.Width() * (vCoord[0] - GetAxesMin()[0]) 
		/ (GetAxesMax()[0] - GetAxesMin()[0]));
	pt.y = rect.Height() + rect.top - (int)((REAL) rect.Height() * (vCoord[1] - GetAxesMin()[1]) 
		/ (GetAxesMax()[1] - GetAxesMin()[1]));

	return pt;
}

////////////////////////////////////////////////////////////////////////////
CGraph::GraphCoord 
	CGraph::FromPlotCoord(const CPoint& pt)
{
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(m_arrMargins[0], m_arrMargins[1], 
		m_arrMargins[2], m_arrMargins[3]);

	GraphCoord vCoord;
	vCoord[0] = (pt.x - rect.left) * (GetAxesMax()[0] - GetAxesMin()[0]) 
		/ rect.Width() + GetAxesMin()[0];
	vCoord[1] = -(pt.y - rect.Height() - rect.top) * (GetAxesMax()[1] - GetAxesMin()[1]) 
		/ rect.Height() + GetAxesMin()[1];  

	return vCoord;
}

////////////////////////////////////////////////////////////////////////////
void 
	CGraph::OnDataSeriesChanged() // CObservableEvent * pOE, void * pParam)
	// called when one of my data series changes
{
	AutoScale();
	SetAxesMin(MakeVector<2>(0.0, 0.0));
	Invalidate(TRUE);
}
