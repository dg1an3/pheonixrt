// DibView.cpp : implementation file
//

#include "stdafx.h"
#include "DibView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDibView

CDibView::CDibView()
	: m_pDoc(NULL),
		m_pDib(NULL),
		m_pTransform(NULL),
		m_pInverseTransform(NULL),
		m_nDataSet(-1),
		m_nDraggingLandmark(-1),
		m_bDrawMarks(TRUE)
{
}

CDibView::~CDibView()
{
}

// sets the document that is being displayed by this CDibView
void CDibView::SetDocument(CDocument *pDoc)
{
	m_pDoc = pDoc;
}

// sets the DIB to be displayed
void CDibView::SetDib(CDib *pDib)
{
	m_pDib = pDib;
}

// sets the transform to be managed
void CDibView::SetTransform(CTPSTransform *pForwardTransform, CTPSTransform *pInverseTransform, int nDataSet)
{
	m_pTransform = pForwardTransform;
	m_pInverseTransform = pInverseTransform;
	m_nDataSet = nDataSet;
}

BEGIN_MESSAGE_MAP(CDibView, CWnd)
	//{{AFX_MSG_MAP(CDibView)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDibView message handlers

static COLORREF g_arrColors[] = 
{
	RGB(255, 128, 255),
	RGB(255,   0, 255),
	RGB(255, 255,   0),
	RGB(  0, 255, 255),
	RGB(255,   0,   0),
	RGB(  0, 255,   0),
	RGB(  0,   0, 255),
	RGB(128, 255, 128),
};

void CDibView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (m_pDib)
	{
		CRect rectSrc(0, 0, m_pDib->GetSize().cx, m_pDib->GetSize().cy);
		CRect rectDst = GetDstRect();
		
		// draw the image
		m_pDib->Draw(dc, &rectDst, &rectSrc,
		    FALSE, /*bUseDrawDib*/
			NULL, 
			FALSE /*bForeground*/
			);

		// draw the landmarks
		if (m_nDataSet >= 0 && m_bDrawMarks)
		{
			CBrush *pOldBrush = (CBrush *) dc.SelectStockObject(HOLLOW_BRUSH);

			for (int nAt = 0; nAt < m_pTransform->GetLandmarkCount(); nAt++)
			{	
				int nColorCount = sizeof(g_arrColors) / sizeof(COLORREF);
				CPen pen(PS_SOLID, 1, g_arrColors[nAt % nColorCount]);
				CPen *pOldPen = (CPen *) dc.SelectObject(&pen);

				CPoint pt = 
					Image2Client(m_pTransform->GetLandmark(m_nDataSet, nAt));

				// dc.Ellipse(pt.x - 5, pt.y - 5, pt.x + 6, pt.y + 6);

				dc.MoveTo(pt.x - 5, pt.y);
				dc.LineTo(pt.x + 6, pt.y);

				dc.MoveTo(pt.x, pt.y - 5);
				dc.LineTo(pt.x, pt.y + 6);

				dc.SelectObject(pOldPen);
			}

			dc.SelectObject(pOldBrush);
		}
	}
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CDibView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (NULL != m_pTransform)
	{
		// find the landmark that we are dragging
		for (int nAt = 0; nAt < m_pTransform->GetLandmarkCount(); nAt++)
		{
			CSize size = point 
				- Image2Client(m_pTransform->GetLandmark(m_nDataSet, nAt));

			if (-5 < size.cx && size.cx < 5
				&& -5 < size.cy && size.cy < 5)
			{
				m_nDraggingLandmark = nAt;
			}
		}

		// create a new point
		if (-1 == m_nDraggingLandmark)
		{
			int nOldSize = m_pTransform->GetLandmarkCount();
			m_nDraggingLandmark = m_pTransform->AddLandmark(Client2Image(point));
			int nNewSize = m_pTransform->GetLandmarkCount();
			ASSERT(nOldSize+1 == nNewSize);

			// add the inverse landmark
			int inverseLandmark = m_pInverseTransform->AddLandmark(Client2Image(point));

			// check that the inverse landmark is at the same position
			ASSERT(m_nDraggingLandmark == inverseLandmark);

			// redraw the segment the warping
			Invalidate(FALSE);
		}

		// store the mouse point
		m_ptPrev = point;
	}
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CDibView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (-1 != m_nDraggingLandmark)
	{
		m_pTransform->SetLandmark(m_nDataSet, m_nDraggingLandmark, 
			m_pTransform->GetLandmark(m_nDataSet, m_nDraggingLandmark) 
				+ Client2Image(point) - Client2Image(m_ptPrev));

		// TODO: shouldn't this be reversed
		m_pInverseTransform->SetLandmark(1-m_nDataSet, m_nDraggingLandmark, 
			m_pInverseTransform->GetLandmark(1-m_nDataSet, m_nDraggingLandmark) 
				+ Client2Image(point) - Client2Image(m_ptPrev));

		Invalidate(FALSE);

		// store the mouse point
		m_ptPrev = point;
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

void CDibView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_nDraggingLandmark = -1;

	// Check if both inverse and forward are consistent
	ASSERT(m_pTransform->CheckInverse(m_pInverseTransform));
	ASSERT(m_pInverseTransform->CheckInverse(m_pTransform));

	if (m_pDoc)
	{
		m_pDoc->UpdateAllViews(NULL);
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

CVectorD<3> CDibView::Client2Image(const CPoint &ptClient)
{
	// source (DIB) image size
	CRect rectSrc(0, 0, m_pDib->GetSize().cx, m_pDib->GetSize().cy);

	// destination (window) image size
	CRect rectDst = GetDstRect();

	// ratio of source / destination heights
	REAL ratio = (REAL) rectSrc.Height() / (REAL) rectDst.Height();

	// offset to center and scale
	CVectorD<3> vImage(ptClient);
	vImage -= CVectorD<3>(rectDst.CenterPoint());
	vImage *= ratio;
	vImage += CVectorD<3>(rectSrc.CenterPoint());

	return vImage;
}

CPoint CDibView::Image2Client(const CVectorD<3>&vImage)
{
	// source (DIB) image size
	CRect rectSrc(0, 0, m_pDib->GetSize().cx, m_pDib->GetSize().cy);

	// destination (window) image size
	CRect rectDst = GetDstRect();

	// ratio of destination / source heights
	REAL ratio = (REAL) rectDst.Height() / (REAL) rectSrc.Height();

	// offset to center and scale
	CPoint ptClient(vImage - CVectorD<3>(rectSrc.CenterPoint()));
	ptClient.x = (int)(ratio * (REAL) ptClient.x); 
	ptClient.y = (int)(ratio * (REAL) ptClient.y);
	ptClient += rectDst.CenterPoint();

	return ptClient;
}

CRect CDibView::GetDstRect()
{
	CRect rectDst;
	GetClientRect(&rectDst);

	CRect rectSrc(0, 0, m_pDib->GetSize().cx, m_pDib->GetSize().cy);

	// set the client rectangle to the same aspect ratio
	int nClientWidth = rectDst.Height() * rectSrc.Width() 
		/ rectSrc.Height();
	CPoint ptCenter = rectDst.CenterPoint();
	rectDst.left = ptCenter.x - nClientWidth / 2;
	rectDst.right = ptCenter.x + nClientWidth / 2;

	return rectDst;
}
