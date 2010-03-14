#include "StdAfx.h"
#include "WindowLevelTracker.h"

#include "PlanarView.h"


//////////////////////////////////////////////////////////////////////////////
void 
	WindowLevelTracker::OnButtonDown(UINT nFlags, CPoint point)
{
	const VolumeReal *pVolumeIn = GetView()->GetVolumeResampler0()->GetInput();
	m_windowMax = GetMax<VOXEL_REAL>(pVolumeIn);
	m_windowStart = GetView()->GetWindowLevelFilter0()->GetWindow();
	m_levelStart = GetView()->GetWindowLevelFilter0()->GetLevel();
	m_ptWLStart = point;
}

//////////////////////////////////////////////////////////////////////////////
void 
	WindowLevelTracker::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rectClient;
	GetView()->GetClientRect(&rectClient);

	CPoint ptDelta = point - m_ptWLStart;

	REAL fracWindow = (REAL) ptDelta.y / (REAL) rectClient.Height();
	REAL newWindow =  m_windowStart - m_windowMax * fracWindow;
	newWindow = __max(newWindow, 0.001);

	REAL fracLevel = (REAL) ptDelta.x / (REAL) rectClient.Width();
	REAL newLevel = m_levelStart - m_windowMax * fracLevel;

	GetView()->GetWindowLevelFilter0()->SetWindowLevel(newWindow, newLevel);
	GetView()->GetWindowLevelFilter0()->Modified();
}
