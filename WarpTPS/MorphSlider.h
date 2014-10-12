#pragma once
#include <TPSTransform.h>
#include <Dib.h>
#include <DibView.h>
#include "afxcmn.h"
#include "afxwin.h"

// MorphSlider dialog

class MorphSlider : public CDialog
{
	DECLARE_DYNAMIC(MorphSlider)

public:
	MorphSlider(CWnd* pParent = NULL);   // standard constructor
	virtual ~MorphSlider();

	CTPSTransform *m_pTransform;
	CTPSTransform *m_pInverseTransform;

	// various and sundry images
	CDib *m_pSourceImage;
	CDib *m_pDestinationImage;
	CDib *m_pWarpedSourceImage;
	CDib *m_pWarpedDestinationImage;
	CDib *m_pBlendedImage;

	CDibView *m_pWarpedView;

	float m_morphPercent;
	float m_blendPercent;

	// Dialog Data
	enum { IDD = IDD_MORPHSLIDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedCheckLock();

	// child controls
	CSliderCtrl m_morphSlider;
	CSliderCtrl m_blendSlider;
	CSliderCtrl m_rSlider;
	CButton m_checkLock;
};
