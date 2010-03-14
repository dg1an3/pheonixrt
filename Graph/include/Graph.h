// Copyright (C) 2008 DGLane
// $Id$
#pragma once

#include <Dib.h>

#include <itkColorTable.h>
using namespace itk;

#include <DataSeries.h>

namespace dH {

/////////////////////////////////////////////////////////////////////////////
class Graph : 
		public CWnd
{
// Construction
public:
	Graph();
	virtual ~Graph();

// Attributes
public:

	// accessors for data series
	int GetDataSeriesCount();
	dH::DataSeries *GetDataSeriesAt(int nAt);
	void AddDataSeries(dH::DataSeries *pSeries);
	void RemoveDataSeries(int nAt, bool bDelete = false);
	void RemoveAllDataSeries(bool bDelete = false);

// Operations
public:

	// sets up graph margins
	void SetMargins(int nLeft, int nTop, int nRight, int nBottom);

	// sets the axes ranges and tick marks
	typedef dH::DataSeries::CurveType::VertexType GraphCoord;

	// axes tick marks

	DeclareMember(AxesMin, GraphCoord);
	DeclareMember(AxesMax, GraphCoord);

	DeclareMember(AxesMajor, GraphCoord);
	DeclareMember(AxesMinor, GraphCoord);

	// computes the min and max values for the graph
	void AutoScale();

	// sets tick marks for min / max
	void ScaleTickMarks();

	// flag to indicate that zero tail be "chopped off"
	DeclareMember(TruncateZeroTail, bool);

	// sets up the legend (legend only displayed after this is called)
	void SetLegendColormap(ColorTable<unsigned char> *pColormap, 
				REAL window, REAL level);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraph)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CGraph)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Implementation
public:

	// draws the minor ticks and grids
	void DrawMinorAxes(CDC * pDC, const CRect& rect);

	// draw major ticks and grids
	void DrawMajorAxes(CDC * pDC, const CRect& rect);

	// draw one series (or all?)
	void DrawSeries(CDC * pDC, const CRect& rect);

	// draw the legend
	void DrawLegend(CDC * pDC, const CRect& rect);

	// converts to coordinates on the plot
	CPoint ToPlotCoord(const GraphCoord& vCoord);
	GraphCoord FromPlotCoord(const CPoint& vCoord);

private:
	// the array of data series
	std::vector< dH::DataSeries::Pointer > m_arrDataSeries;

	// graph plot area
	int m_arrMargins[4];

	// dragging logic
	dH::DataSeries *m_pDragSeries;
	int m_nDragPoint;
	CPoint m_ptDragOffset;
	BOOL m_bDragging;

	// legend variables
	ColorTable<unsigned char>::Pointer m_pLegendColormap;
	REAL m_window;
	REAL m_level;
	bool m_bShowLegend;

	// graph buffer
	CDib m_dib;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

}