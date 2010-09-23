// Copyright (C) 2nd Messenger Systems
// $Id: PlanarView.h 640 2009-06-13 05:06:50Z dglane001 $
#include <Dib.h>
#include <itkUtils.h>
#include <Structure.h>

using namespace itk;

#pragma once

/////////////////////////////////////////////////////////////////////////////
// class CPlanarView 
//
// represents a single MPR
/////////////////////////////////////////////////////////////////////////////
class CPlanarView : public CWnd
{
public:
// Construction
	CPlanarView();
	virtual ~CPlanarView();

// Attributes
	// points to the currently displayed series
	DECLARE_ATTRIBUTE_PTR_GI(Series, dH::Series);

	// displayed volumes
	void SetVolume(VolumeReal *pVolume, int nVolumeAt = 0);

	// sets contour mode for the selected structure
	DECLARE_ATTRIBUTE_PTR(SelectedStructure, dH::Structure);
	DECLARE_ATTRIBUTE(SelectedContour, dH::Structure::PolygonType::Pointer);

	// typedef itk::Vector<REAL,2> Vertex;
	DECLARE_ATTRIBUTE(SelectedVertex, int);

	// display state
	void SetWindowLevel(REAL win, REAL cen, int nVolumeAt = 0);
	void SetLUT(CArray<COLORREF, COLORREF>& arrLUT, int nVolumeAt = 0);
	void SetAlpha(REAL alpha);

	// sets up specifics of zoom / pan
	void InitZoomCenter(void);
	void SetCenter(const Vector<REAL>& vCenter);
	void SetZoom(REAL zoom = 1.0);
	void SetBasis(const Point<REAL>& origin, const Vector<REAL>& spacing );

// Operations

	// drawing helpers
	void DrawContours(CDC *pDC);
	void DrawImages(CDC *pDC);
	void DrawIsocurves(VolumeReal *pVolume, REAL c, CDC *pDC);

	bool ContourHitTest(CPoint& point, dH::Structure::PolygonType *pContour, int *pnVertex);
	CRgn *GetRgnForContour(dH::Structure::PolygonType *pContour);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlanarView)
	//}}AFX_VIRTUAL

// Implementation
public:
	// structures
	//CSeries * m_pSeries;

	// displayed volumes
	VolumeReal * m_pVolume[2];

	// drawing temporaries
	VolumeReal::Pointer m_volumeResamp[2];
	CArray<COLORREF, COLORREF> m_arrPixels;
	CDib m_dib;

	// image display parameters
	REAL m_window[2];
	REAL m_level[2];
	REAL m_alpha;
	CArray<COLORREF, COLORREF> m_arrLUT[2];

	// window / level mode 
	bool m_bWindowLeveling;
	REAL m_windowStart;
	REAL m_levelStart;

	// stores zoom / pan 
	Vector<REAL> m_vCenter;
	REAL m_zoom;

	// zoom / pan mode
	bool m_bZooming;
	REAL m_zoomStart;

	bool m_bPanning;
	bool m_bPanningZ;
	Vector<REAL> m_vPtStart;
	Vector<REAL> m_vCenterStart;
	Matrix<REAL, 4, 4> m_mBasisStart;

	// helper to start starting point of mouse operation
	CPoint m_ptOpStart;

	bool m_bEditContourMode;
	bool m_bAddContourMode;
	bool m_bLockToSlice;

	dH::Structure::PolygonType::PointType m_vVertexStart;

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlanarView)
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

};	// class CPlanarView 


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
