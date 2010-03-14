// Copyright (C) DGLane - U. S. Patent 0,000,000
// $Id PlanarView.h,v0.0 2008-MM-DD Derek Lane $
#pragma once

#include <Dib.h>

#include <itkIntensityWindowingImageFilter.h>
using namespace itk;

#include <Structure.h>
#include <InPlaneResampleImageFilter.h>
#include <DisplayBlendFilter.h>
#include <ZoomPanTracker.h>
#include <WindowLevelTracker.h>
#include <ContourOverlay.h>
#include <ContourEditTracker.h>
#include <IsocurveOverlay.h>

namespace dH {

/////////////////////////////////////////////////////////////////////////////
class PlanarView 
	: public CWnd
{
public:
	PlanarView();
	virtual ~PlanarView();

// Attributes

	// points to the currently displayed series
	DeclareMemberPtrGI(Series, dH::Series);
	// void SetSeries(dH::Series *pSeries);

	// displayed volumes
	void SetVolume(VolumeReal *pVolume, int nVolumeAt = 0);

	// sets up specifics of zoom / pan
	void InitZoomCenter(void);
	DeclareMemberGI(Center, Vector<REAL>);
    //void SetCenter(const Vector<Real>& vCenter);
	DeclareMemberGI(Zoom, REAL);
	//void SetZoom(const Real& value);

	// controller for zoom / pan
	DeclareMemberSPtr(ZoomPanner, ZoomPanTracker);

	// controller for zoom / pan
	DeclareMemberSPtr(WindowLeveler, WindowLevelTracker);

	// displays the contour overlays
	DeclareMemberSPtr(ContourDrawer, ContourOverlay);
	DeclareMemberSPtr(ContourEditor, ContourEditTracker);

	// displays the isocurves
	DeclareMemberSPtr(IsocurveDrawer, IsocurveOverlay);

	// image processing pipeline

	// resample filters
	DeclareMemberSPtr(VolumeResampler0, dH::InPlaneResampleImageFilter);
	DeclareMemberSPtr(VolumeResampler1, dH::InPlaneResampleImageFilter);

	// window/level filters
	typedef IntensityWindowingImageFilter<VolumeReal, VolumeChar> WindowLevelFilterType;
	DeclareMemberSPtr(WindowLevelFilter0, WindowLevelFilterType);
	DeclareMemberSPtr(WindowLevelFilter1, WindowLevelFilterType);

	// blending filter
	DeclareMemberSPtr(Blender, dH::DisplayBlendFilter);

// Operations
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PlanarView)
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetBasis(void);

	// drawing bitmap
	CDib m_dib;

	// temp pointer to the tracker that currently has focus
	PlanarTracker *m_pTrackerWithFocus;

	// Generated message map functions
protected:
	//{{AFX_MSG(PlanarView)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};	// class PlanarView 


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

}