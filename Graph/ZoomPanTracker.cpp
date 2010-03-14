#include "StdAfx.h"
#include <ZoomPanTracker.h>

#include <PlanarView.h>

namespace dH {

//////////////////////////////////////////////////////////////////////////////
ZoomPanTracker::ZoomPanTracker(void)
	: m_pView(NULL)
	, m_bZooming(false)
	, m_bPanning(false)
	, m_bPanningZ(false)
	, m_LockToSlice(true)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
	ZoomPanTracker::OnButtonDown(UINT nFlags, CPoint point)
{
	// form panning region
	CRect rectClient;
	GetView()->GetClientRect(&rectClient);
	rectClient.DeflateRect(rectClient.Width() / 3, rectClient.Height() / 3,
		rectClient.Width() / 3, rectClient.Height() / 3);

	// check for panning
	if (rectClient.PtInRect(point))
	{
		m_bPanning = true;
		m_bPanningZ = false;
	}
	else if (point.x < rectClient.left || point.x > rectClient.right)
	{
		m_bZooming = true;
		m_zoomStart = GetView()->GetZoom();
		m_ptZoomStart = point;
	}
	else
	{
		m_bPanning = true;
		m_bPanningZ = true;
	}

	if (m_bPanning)
	{
		// store basis at start
		CalcBasis<3>(GetView()->GetVolumeResampler0()->GetOutput(), m_mBasisStart);
		MultHG(m_mBasisStart, MakeVector<3>(point), m_vPanStart);

		// calc center at start
		m_vCenterStart = GetView()->GetCenter();
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	ZoomPanTracker::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bZooming)
	{
		CRect rectClient;
		GetView()->GetClientRect(&rectClient);

		int yDelta = point.y - m_ptZoomStart.y;
		REAL newZoom = m_zoomStart * exp( - 0.75 * (REAL) yDelta / (REAL) rectClient.Height());
		GetView()->SetZoom(newZoom);
	}
	else if (m_bPanning)
	{
		Vector<REAL> vPtNext;
		MultHG(m_mBasisStart, MakeVector<3>(point), vPtNext);

		// convert m_vCenter to space
		Vector<REAL> vNewCenter = m_vCenterStart;
		if (m_bPanningZ)
		{
			vNewCenter[2] -= (vPtNext[0] - m_vPanStart[0]) / 1.0;
			if (GetLockToSlice())
			{
				const VolumeReal *pVolumeIn = GetView()->GetVolumeResampler0()->GetInput();
				const Point<REAL>& origin = pVolumeIn->GetOrigin();
				const Vector<REAL>& spacing = pVolumeIn->GetSpacing();
				vNewCenter[2] = Round<int>((vNewCenter[2] - origin[2]) / spacing[2]) * spacing[2] + origin[2];
			}
		}
		else	// normal pan
		{
			vNewCenter -= (vPtNext - m_vPanStart);
		}

		// set the new center position
		GetView()->SetCenter(vNewCenter);
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	ZoomPanTracker::OnButtonUp(UINT nFlags, CPoint point)
{
	m_bZooming = false;
	m_bPanning = false;
	m_bPanningZ = false;
}

}