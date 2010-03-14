// Copyright (C) 2005-2008 DGLane
// $Id$
#include "stdafx.h"

#include "PlanarView.h"
#include <ContourOverlay.h>

#include <Series.h>

namespace dH {

////////////////////////////////////////////////////////////////////////////////////////////
PlanarView::PlanarView()
	: m_pSeries(NULL)
	, m_pTrackerWithFocus(NULL)
{
	SetVolumeResampler0(dH::InPlaneResampleImageFilter::New()); 
	SetVolumeResampler1(dH::InPlaneResampleImageFilter::New());

	SetWindowLevelFilter0(WindowLevelFilterType::New());
	GetWindowLevelFilter0()->SetInput(GetVolumeResampler0()->GetOutput());
	GetWindowLevelFilter0()->SetWindowLevel(400.0, 30.0);

	SetWindowLevelFilter1(WindowLevelFilterType::New());
	GetWindowLevelFilter1()->SetInput(GetVolumeResampler1()->GetOutput());
	GetWindowLevelFilter1()->SetWindowLevel(1.0, 0.5);

	SetBlender(dH::DisplayBlendFilter::New());
	GetBlender()->SetInput(0, GetWindowLevelFilter0()->GetOutput());
	GetBlender()->SetInput(1, GetWindowLevelFilter1()->GetOutput());
	GetBlender()->SetAlpha((REAL) 1.0);

	SetZoomPanner(ZoomPanTracker::New());
	GetZoomPanner()->SetView(this);

	SetWindowLeveler(WindowLevelTracker::New());
	GetWindowLeveler()->SetView(this);

	const VolumeReal *pOutput = GetVolumeResampler0()->GetOutput();

	SetContourDrawer(ContourOverlay::New());
	GetContourDrawer()->SetOriginSpacing(pOutput->GetOrigin(), 
		pOutput->GetSpacing());

	SetContourEditor(ContourEditTracker::New());
	GetContourEditor()->SetOverlay(GetContourDrawer());

	SetIsocurveDrawer(IsocurveOverlay::New());
	GetIsocurveDrawer()->SetOriginSpacing(pOutput->GetOrigin(), 
		pOutput->GetSpacing());
}

////////////////////////////////////////////////////////////////////////////////////////////
PlanarView::~PlanarView()
{
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::SetSeries(dH::Series *pSeries)
{
	m_pSeries = pSeries;
	GetContourDrawer()->SetSeries(pSeries);
	SetVolume(GetSeries()->GetDensity(), 0);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::SetVolume(VolumeReal *pVolume, int nVolumeAt)
{
	switch (nVolumeAt)
	{
	case 0:
		GetVolumeResampler0()->SetInput(pVolume);
		break;

	case 1:
		GetVolumeResampler1()->SetInput(pVolume);
		GetIsocurveDrawer()->SetDoseVolume(pVolume);
		break;
	}

	if (nVolumeAt == 0)
	{
		// set up initial zoom
		InitZoomCenter();
	}

	// if no secondary, show only primary from blender
	if (GetVolumeResampler1()->GetInput() == NULL)
	{
		GetBlender()->SetAlpha(1.0);
	}
	else
	{
		GetBlender()->SetAlpha(0.5);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::InitZoomCenter(void)
	// sets up initial zoom / pan state
{
	// reset zoom
	m_Zoom = 1.0;
	m_Center = Vector<REAL>(0.0);

	const VolumeReal *pVolumeIn = GetVolumeResampler0()->GetInput();
	if (pVolumeIn)
	{
		m_Center[0] = pVolumeIn->GetOrigin()[0] 
			+ pVolumeIn->GetSpacing()[0] * 0.5 * (REAL) pVolumeIn->GetBufferedRegion().GetSize()[0];
		m_Center[1] = pVolumeIn->GetOrigin()[1] 
			+ pVolumeIn->GetSpacing()[1] * 0.5 * (REAL) pVolumeIn->GetBufferedRegion().GetSize()[1];
		m_Center[2] = pVolumeIn->GetOrigin()[2];
	}

	// set the zoom (to set the basis)
	SetBasis();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::SetCenter(const Vector<REAL>& vCenter)
{
	m_Center = vCenter;
	SetBasis();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::SetZoom(const REAL& zoom)
{
	m_Zoom = zoom;
	SetBasis();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::SetBasis(void)
{
	const VolumeReal *pVolumeIn = GetVolumeResampler0()->GetInput();
	if (pVolumeIn)
	{
		Vector<REAL> spacing = pVolumeIn->GetSpacing();
		spacing /= m_Zoom;

		Point<REAL> origin;
		origin[0] = m_Center[0] - spacing[0] * 0.5 * (REAL) GetVolumeResampler0()->GetSize()[0];
		origin[1] = m_Center[1] - spacing[1] * 0.5 * (REAL) GetVolumeResampler0()->GetSize()[1];  
		origin[2] = m_Center[2];

		GetVolumeResampler0()->SetOutputOrigin(origin); 
		GetVolumeResampler0()->SetOutputSpacing(spacing); 

		GetVolumeResampler1()->SetOutputOrigin(origin); 
		GetVolumeResampler1()->SetOutputSpacing(spacing); 
	}
}

BEGIN_MESSAGE_MAP(PlanarView, CWnd)
	//{{AFX_MSG_MAP(PlanarView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);

	// update the blender - use UpdateLargestPossibleRegion so that it will automatically expand
	//		to include the upstream image sizes
	GetBlender()->UpdateLargestPossibleRegion();

	// update the DIB
	m_dib.SetBitmapBits(rect.Width() * rect.Height() * sizeof(COLORREF),
		(void *) GetBlender()->GetOutput()->GetBufferPointer());

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	// selects image objects in to context
	dcMem.SelectObject(m_dib);

	// draw isocurves
	GetIsocurveDrawer()->Draw(&dcMem);

	// draw contours
	GetContourDrawer()->Draw(&dcMem);

	// draw Z position
	CString strPosition;
	strPosition.Format(_T("Z = %.1lf"), GetVolumeResampler0()->GetOutputOrigin()[2]);
	dcMem.SetTextColor(RGB(255, 255, 255));
	dcMem.SetBkMode(TRANSPARENT);
	dcMem.TextOut(rect.Width() / 2 - 50, rect.Height() - 40, strPosition);

	// and blt
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

	// Do not call CWnd::OnPaint() for painting messages
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	// only set the size on the resamplers (beginning of viewing pipeline); the call
	//		to UpdateLargestPossibleRegion will automatically set the downstream
	//		process object sizes
	GetVolumeResampler0()->SetSize(MakeSize(cx, cy, 1));
	GetVolumeResampler1()->SetSize(MakeSize(cx, cy, 1));

	if (m_dib.GetSize().cx != cx
		|| m_dib.GetSize().cy != cy)
	{
		m_dib.DeleteObject();
		HBITMAP bm = CreateCompatibleBitmap(*GetDC(), cx, cy);
		m_dib.Attach(bm);
	}

	// recalc zoom / pan
	SetBasis();
}

////////////////////////////////////////////////////////////////////////////////////////////
void	
	PlanarView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_pTrackerWithFocus = GetWindowLeveler();
	m_pTrackerWithFocus->OnButtonDown(nFlags, point);
	SetCapture();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	if (m_pTrackerWithFocus)
		m_pTrackerWithFocus->OnButtonUp(nFlags, point);
	m_pTrackerWithFocus = NULL;
	ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnLButtonDown(UINT nFlags, CPoint point)
{
	//m_pTrackerWithFocus = GetContourEditor();
	m_pTrackerWithFocus = GetWindowLeveler();
	m_pTrackerWithFocus->OnButtonDown(nFlags, point);
	SetCapture();

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_pTrackerWithFocus)
		m_pTrackerWithFocus->OnButtonUp(nFlags, point);
	m_pTrackerWithFocus = NULL;
	ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	GetContourEditor()->OnButtonDblClk(nFlags, point);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_pTrackerWithFocus = GetZoomPanner();
	m_pTrackerWithFocus->OnButtonDown(nFlags, point);
	SetCapture();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_pTrackerWithFocus)
		m_pTrackerWithFocus->OnButtonUp(nFlags, point);
	m_pTrackerWithFocus = NULL;
	ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	PlanarView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_pTrackerWithFocus)
	{
		m_pTrackerWithFocus->OnMouseMove(nFlags, point);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

}