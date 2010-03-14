// Graph.cpp : implementation file
//
#include "stdafx.h"

#include <Graph.h>

BeginNamespace(dH)

const int DEFAULT_MARGIN = 50;
const COLORREF AXIS_COLOR = RGB(255, 255, 255);

/////////////////////////////////////////////////////////////////////////////
Graph::Graph()
: m_pDragSeries(NULL)
, m_bDragging(FALSE)
, m_bShowLegend(false)
, m_TruncateZeroTail(true)
{
}

/////////////////////////////////////////////////////////////////////////////
Graph::~Graph()
{
	RemoveAllDataSeries(true);	// be sure and delete them as well

}

/////////////////////////////////////////////////////////////////////////////
int 
Graph::GetDataSeriesCount()
{
	return m_arrDataSeries.size();
}

/////////////////////////////////////////////////////////////////////////////
dH::DataSeries *
Graph::GetDataSeriesAt(int nAt)
{
	return m_arrDataSeries[nAt];
}

/////////////////////////////////////////////////////////////////////////////
void 
Graph::AddDataSeries(dH::DataSeries *pSeries)
{
	pSeries->m_pGraph = this;
	m_arrDataSeries.push_back(pSeries);
	AutoScale();
}

/////////////////////////////////////////////////////////////////////////////
void 
Graph::RemoveDataSeries(int nAt, bool bDelete)
{
}	

/////////////////////////////////////////////////////////////////////////////
void 
Graph::RemoveAllDataSeries(bool bDelete)
{
	m_arrDataSeries.clear();
}	

/////////////////////////////////////////////////////////////////////////////
void 
Graph::SetMargins(int nLeft, int nTop, int nRight, int nBottom)
{
	m_arrMargins[0] = nLeft;
	m_arrMargins[1] = nTop;
	m_arrMargins[2] = nRight;
	m_arrMargins[3] = nBottom;
}

/////////////////////////////////////////////////////////////////////////////
void 
Graph::AutoScale()
{
	// are we determining new min / max?
	if (m_arrDataSeries.size() > 0)
	{
		// accumulate mins and maxes
		REAL minInit[2] = {0.0, 0.0};
		SetAxesMin(GraphCoord(minInit));
		REAL maxInit[2] = {100.0, 100.0};
		SetAxesMax(GraphCoord(maxInit));

		// find the min / max for mantissa and abcsisca
		std::vector< dH::DataSeries::Pointer >::iterator dataSeries =
			m_arrDataSeries.begin();
		for (; dataSeries != m_arrDataSeries.end(); ++dataSeries)
		{
			dH::DataSeries::CurveType *pCurve = (*dataSeries)->GetCurve();
			if (!pCurve)
				continue;

			// iterate over vertices
			dH::DataSeries::CurveType::VertexListType::const_iterator vertex = 
				pCurve->GetVertexList()->begin();
			for (; vertex != pCurve->GetVertexList()->end(); ++vertex)
			{
				SetAxesMin(MakeContinuousIndex<2>(
					__min(GetAxesMin()[0], (*vertex)[0]),
					__min(GetAxesMin()[1], (*vertex)[1])));

				SetAxesMax(MakeContinuousIndex<2>(
					__max(GetAxesMax()[0], (*vertex)[0]),
					__max(GetAxesMax()[1], (*vertex)[1])));
			} 
		}
	}

	ScaleTickMarks();
}	

/////////////////////////////////////////////////////////////////////////////
void 
Graph::ScaleTickMarks()
{
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

	// now adjust for plot coordinates
	CPoint ptOrigin = ToPlotCoord(MakeContinuousIndex<2>(0.0, 0.0));
	CPoint ptStep = ToPlotCoord(m_AxesMajor) - ptOrigin;
	while (ptStep.x != 0 && abs(ptStep.x) < 20)	// width of 20
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
	}
}	

/////////////////////////////////////////////////////////////////////////////
void 
Graph::SetLegendColormap(ColorTable<unsigned char> *pColormap, 
												 REAL window, REAL level)
{
	m_pLegendColormap = pColormap;
	m_window = window;
	m_level = level;

	m_bShowLegend = true;
}


BEGIN_MESSAGE_MAP(Graph, CWnd)
	//{{AFX_MSG_MAP(Graph)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
int 
Graph::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetMargins(DEFAULT_MARGIN, DEFAULT_MARGIN / 2, DEFAULT_MARGIN / 2, 
		DEFAULT_MARGIN);

	SetAxesMin(MakeContinuousIndex<2>(0.0, 0.0));
	SetAxesMax(MakeContinuousIndex<2>(100.0, 100.0));

	ScaleTickMarks();

	return 0;
}


////////////////////////////////////////////////////////////////////////////
void 
Graph::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CRect rect;
	GetClientRect(&rect);

	if (m_dib.GetSize().cx != rect.Width()
		|| m_dib.GetSize().cy != rect.Height())
	{
		m_dib.DeleteObject();
		HBITMAP bm = ::CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
		m_dib.Attach(bm);
	}
	dcMem.SelectObject(m_dib);

	// draw the axes
	COLORREF bkColor = RGB(16, 16, 16);
	CBrush brushBack(bkColor);
	CBrush *pOldBrush = dcMem.SelectObject(&brushBack);
	dcMem.Rectangle(rect);

	dcMem.SetBkColor(bkColor);

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

	GetClientRect(&rect);
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

	// Do not call CWnd::OnPaint() for painting messages
}

/////////////////////////////////////////////////////////////////////////////
void 
Graph::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bDragging)
	{
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));

		//CMatrixNxM<> mData = m_pDragSeries->GetDataMatrix();
		//mData[m_nDragPoint][0] = FromPlotCoord(point - m_ptDragOffset)[0];
		//mData[m_nDragPoint][1] = FromPlotCoord(point - m_ptDragOffset)[1];

		//if (m_nDragPoint > 0)
		//{
		//	mData[m_nDragPoint][0] = __max(mData[m_nDragPoint][0], 
		//		mData[m_nDragPoint-1][0]);
		//	mData[m_nDragPoint][1] = __min(mData[m_nDragPoint][1], 
		//		mData[m_nDragPoint-1][1]);
		//}

		//if (m_nDragPoint < mData.GetCols()-1)
		//{
		//	mData[m_nDragPoint][0] = __min(mData[m_nDragPoint][0], 
		//		mData[m_nDragPoint+1][0]);
		//	mData[m_nDragPoint][1] =__max(mData[m_nDragPoint][1], 
		//			mData[m_nDragPoint+1][1]);
		//}


		//if (m_nDragPoint == 0)
		//	// && m_pDragSeries->GetMonotonicDirection() == -1)
		//{
		//	mData[m_nDragPoint][1] = 100.0;
		//}

		//if (m_nDragPoint == mData.GetCols()-1)
		//{
		//	mData[m_nDragPoint][1] = 0.0;
		//}
		//m_pDragSeries->SetDataMatrix(mData);

		Invalidate();
		return;
	}

	std::vector< dH::DataSeries::Pointer >::iterator dataSeries =
		m_arrDataSeries.begin();
	for (; dataSeries != m_arrDataSeries.end(); ++dataSeries)
	{
		if ((*dataSeries)->GetHandleHit(point, 10) != -1)
		{
			::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
			return;
		}
	}

	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
}

/////////////////////////////////////////////////////////////////////////////
void 
Graph::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bDragging)
	{
		std::vector< dH::DataSeries::Pointer >::iterator dataSeries =
			m_arrDataSeries.begin();
		for (; dataSeries != m_arrDataSeries.end(); ++dataSeries)
		{
			m_nDragPoint = (*dataSeries)->GetHandleHit(point, 10, &m_ptDragOffset);
			if (m_nDragPoint != -1)
			{
				m_bDragging = TRUE;
				m_pDragSeries = (*dataSeries);
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
Graph::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDragging = FALSE;
	m_pDragSeries = NULL;

	// if we have dragged, then re-scale
	// aAutoScale();

	// CWnd::OnLButtonUp(nFlags, point);
}


// drawing helpers

////////////////////////////////////////////////////////////////////////////
void 
	Graph::DrawMinorAxes(CDC * pDC, const CRect& rect)
	// draws the minor ticks and grids
{
	CPen penMinorTicks(PS_SOLID, 1, AXIS_COLOR);
	CPen penMinorGrids(PS_SOLID, 1, RGB(64, 64, 64));

	ContinuousIndex<REAL,2> vAtTick;
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
	Graph::DrawMajorAxes(CDC * pDC, const CRect& rect)
	// draw major ticks and grids
{
	CPen penMajorTicks(PS_SOLID, 1, AXIS_COLOR);
	CPen penMajorGrids(PS_SOLID, 1, RGB(128, 128, 128));
	pDC->SetTextColor(AXIS_COLOR);

	// stores size of text (for vert centering)
	CSize sz = pDC->GetTextExtent(_T("Test"));
	pDC->SetTextAlign(TA_CENTER);

	GraphCoord vAtTick;
	vAtTick[1] = GetAxesMin()[1];
	for (vAtTick[0] = GetAxesMin()[0]; vAtTick[0] < GetAxesMax()[0]; vAtTick[0] += GetAxesMajor()[0])
	{
		CPoint ptTick = ToPlotCoord(vAtTick);

		pDC->SelectObject(&penMajorTicks);
		pDC->MoveTo(ptTick);
		pDC->LineTo(ptTick.x, rect.bottom + 10);

		CString strLabel;
		strLabel.Format((GetAxesMajor()[0] >= 1.0) ? _T("%0.0lf") : _T("%4.1lf"), vAtTick[0]);
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
			strLabel.Format((GetAxesMajor()[1] >= 1.0) ? _T("%0.0lf") : _T("%4.1lf"), vAtTick[1]);
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
	Graph::DrawSeries(CDC * pDC, const CRect& rect)
{
	std::vector< dH::DataSeries::Pointer >::iterator dataSeries =
		m_arrDataSeries.begin();
	for (; dataSeries != m_arrDataSeries.end(); dataSeries++)
	{
		CPen pen((*dataSeries)->GetPenStyle(), 1, (*dataSeries)->GetColor());
		CPen *pOldPen = pDC->SelectObject(&pen);

		// update the curve (if need be)
		(*dataSeries)->UpdateCurve();

		// check that there is a curve
		dH::DataSeries::CurveType *pCurve = (*dataSeries)->GetCurve();
		if (!pCurve || pCurve->GetVertexList()->size() == 0)
			continue;

		dH::DataSeries::CurveType::VertexListType::const_iterator vertex = 
			pCurve->GetVertexList()->begin();

		// move to first vertex
		pDC->MoveTo(ToPlotCoord(*vertex)); 
		++vertex;

		// iterate over remaining
		for (; vertex != pCurve->GetVertexList()->end(); ++vertex)
		{
			pDC->LineTo(ToPlotCoord(*vertex)); 
		}

		if ((*dataSeries)->GetHasHandles())
		{
			CPen penHandle(PS_SOLID, 1, AXIS_COLOR);
			pDC->SelectObject(&penHandle);
			CRect rectHandle(0, 0, 5, 5);

			// iterate over vertices
			dH::DataSeries::CurveType::VertexListType::const_iterator vertex = 
				pCurve->GetVertexList()->begin();
			for (; vertex != pCurve->GetVertexList()->end(); ++vertex)
			{
				rectHandle.OffsetRect(ToPlotCoord(*vertex)
					- rectHandle.CenterPoint());
				pDC->Rectangle(rectHandle);
			}
		}

		pDC->SelectObject(pOldPen);
	}
}

////////////////////////////////////////////////////////////////////////////
void 
	Graph::DrawLegend(CDC * pDC, const CRect& rect)
{
	CRect rectLegend(rect);
	rectLegend.bottom = rectLegend.top - 2;
	rectLegend.top -= 12;

	REAL max_legend_value = R(m_pLegendColormap->GetNumberOfColors());

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
		RGBPixel<unsigned char>* colorRgb = m_pLegendColormap->GetColor(colorIndex);
		CPen penLegend(PS_SOLID, 1, RGB(colorRgb->GetBlue(), colorRgb->GetGreen(), colorRgb->GetRed()));
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
	Graph::ToPlotCoord(const GraphCoord& vCoord)
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
Graph::GraphCoord 
	Graph::FromPlotCoord(const CPoint& pt)
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

EndNamespace(dH)
