//////////////////////////////////////////////////////////////////////
// DibView.h: interface for the CDibView class.
//
// Copyright (C) 2002
// $Id: DibView.h,v 1.3 2003/11/01 01:40:01 default Exp $
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBVIEW_H__82A024A5_F229_4D54_AC0C_53BDCEFEF58D__INCLUDED_)
#define AFX_DIBVIEW_H__82A024A5_F229_4D54_AC0C_53BDCEFEF58D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VectorD.h>
#include <Dib.h>

#include "TPSTransform.h"

//////////////////////////////////////////////////////////////////////
// class CDibView
// 
// window that displays a particular DIB and allows the user to place
//		landmarks that are added to a CTPSTransform.
//////////////////////////////////////////////////////////////////////
class CDibView : public CWnd
{
// Construction
public:
	CDibView();

// Attributes
public:

	// sets the document that is being displayed by this CDibView
	void SetDocument(CDocument *pDoc);

	// sets the DIB to be displayed
	CDib* GetDib() { return m_pDib; }
	void SetDib(CDib *pDib);

	// sets the transform to be managed
	void SetTransform(CTPSTransform *pForwardTransform, CTPSTransform *pInverseTransform, int nDataSet);

// Operations
public:

	// returns the images destination rectangle
	CRect GetDstRect();

	// conversion image <-> client coordinates
	CPoint Image2Client(const CVectorD<3>& vImage);
	CVectorD<3> Client2Image(const CPoint& ptClient);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDibView)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL m_bDrawMarks;
	virtual ~CDibView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDibView)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	// the document (for calling Update)
	CDocument * m_pDoc;

	// pointer to the image to be displayed
	CDib * m_pDib;

	// the TPS that is being managed by the view
	CTPSTransform * m_pTransform;
	CTPSTransform * m_pInverseTransform;
	int m_nDataSet;

	// viewing transform
	CVectorD<2> m_vOffset;
	double m_scale;

	// handles landmark dragging
	int m_nDraggingLandmark;
	CPoint m_ptPrev;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBVIEW_H__82A024A5_F229_4D54_AC0C_53BDCEFEF58D__INCLUDED_)
