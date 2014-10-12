// MorphSlider.cpp : implementation file
//

#include "stdafx.h"
#include "WarpTPS.h"
#include "MorphSlider.h"
#include "afxdialogex.h"
#include <math.h>

// MorphSlider dialog

IMPLEMENT_DYNAMIC(MorphSlider, CDialog)

MorphSlider::MorphSlider(CWnd* pParent /*=NULL*/)
	: CDialog(MorphSlider::IDD, pParent)
	, m_pTransform(NULL)
	, m_pSourceImage(NULL)
	, m_pWarpedSourceImage(NULL)
	, m_pDestinationImage(NULL)
	, m_pWarpedDestinationImage(NULL)
	, m_pBlendedImage(NULL)
	, m_pWarpedView(NULL)
	, m_morphPercent(1.0f)
	, m_blendPercent(1.0f)
{
}

MorphSlider::~MorphSlider()
{
}

void MorphSlider::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_morphSlider);
	DDX_Control(pDX, IDC_SLIDER2, m_blendSlider);
	DDX_Control(pDX, IDC_CHECK_LOCK, m_checkLock);
	DDX_Control(pDX, IDC_SLIDER3, m_rSlider);
}


BEGIN_MESSAGE_MAP(MorphSlider, CDialog)
	ON_WM_HSCROLL()
	ON_WM_SIZE()
END_MESSAGE_MAP()


void MorphSlider::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_pTransform == NULL)
		return;

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	::GetScrollInfo(pScrollBar->GetSafeHwnd(), SB_HORZ, &si);

	if (nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
	{
		BOOL bMorphChanged = FALSE;
		BOOL bBlendChanged = FALSE;
		if (pScrollBar->GetSafeHwnd() == m_morphSlider.GetSafeHwnd())
		{
			bMorphChanged = TRUE;

			int minPos = 0, maxPos = 100;
			pScrollBar->GetScrollRange(&minPos, &maxPos);
			m_morphPercent = 1.0 - (float) nPos / (float) (maxPos - minPos);

			if (m_checkLock.GetCheck() == 1)
			{
				m_blendPercent = m_morphPercent;
				m_blendSlider.SetPos(nPos);
				bBlendChanged = TRUE;
			}
		}
		else if (pScrollBar->GetSafeHwnd() == m_blendSlider.GetSafeHwnd())
		{
			bBlendChanged = TRUE;

			int minPos = 0, maxPos = 100;
			pScrollBar->GetScrollRange(&minPos, &maxPos);
			m_blendPercent = 1.0 - (float) nPos / (float) (maxPos - minPos);

			if (m_checkLock.GetCheck() == 1)
			{
				m_morphPercent = m_blendPercent;
				m_morphSlider.SetPos(nPos);
				bMorphChanged = TRUE;
			}
		}
		else if (pScrollBar->GetSafeHwnd() == m_rSlider.GetSafeHwnd())
		{
			bMorphChanged = TRUE;

			int minPos = 0, maxPos = 100;
			pScrollBar->GetScrollRange(&minPos, &maxPos);
			float r_exp = ((float)(nPos+1));

			m_pTransform->SetK(r_exp);
			m_pInverseTransform->SetK(r_exp);
		}

		if (bMorphChanged)
		{
			TRACE("OnHScroll Resample nSBCode=%i pScrollBar=%x percent=%f\n", 
				nSBCode, pScrollBar, m_morphPercent);

			// now resample the center image
			m_pTransform->Resample(m_pSourceImage, 
				m_pWarpedSourceImage, m_morphPercent);

			// now resample the inverse center image
			m_pInverseTransform->Resample(m_pDestinationImage, 
				m_pWarpedDestinationImage, 1.0f-m_morphPercent);				
		}

		if (bBlendChanged || bMorphChanged)
		{
			m_pBlendedImage->CopyPixels(m_pWarpedSourceImage);
			m_pBlendedImage->BlendPixels(m_pWarpedDestinationImage, m_blendPercent);

			ASSERT(m_pWarpedView->GetDib() == m_pBlendedImage);
			m_pWarpedView->SetDib(m_pBlendedImage);
			m_pWarpedView->SetTransform(m_pTransform, m_pInverseTransform, 0);
			m_pWarpedView->RedrawWindow(NULL, NULL);
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void MorphSlider::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (::IsWindow(m_morphSlider.GetSafeHwnd()))
	{
		CRect rect;
		GetClientRect(&rect);
		rect.DeflateRect(10, 0, 10, 0);

		CRect rectSlider;
		m_morphSlider.GetWindowRect(&rectSlider);
		ScreenToClient(&rectSlider);
		rectSlider.left = rect.left;
		rectSlider.right = rect.right;
		m_morphSlider.MoveWindow(&rectSlider);

		CRect rectBlendSlider;
		m_blendSlider.GetWindowRect(&rectBlendSlider);
		ScreenToClient(&rectBlendSlider);
		rectBlendSlider.left = rect.left;
		rectBlendSlider.right = rect.right;
		m_blendSlider.MoveWindow(&rectBlendSlider);

		CWnd *pTxLabel = GetDlgItem(IDC_STATIC_TREATMENT);
		CRect rectTxLabel;
		pTxLabel->GetWindowRect(&rectTxLabel);
		ScreenToClient(&rectTxLabel);
		rectTxLabel.left = rect.right - rectTxLabel.Width();
		rectTxLabel.right = rect.right;
		pTxLabel->MoveWindow(&rectTxLabel);

		CRect rectCheckLock;
		m_checkLock.GetWindowRect(&rectCheckLock);
		ScreenToClient(&rectCheckLock);
		int width = rectCheckLock.Width();
		rectCheckLock.left = rect.CenterPoint().x - width/2;
		rectCheckLock.right = rect.CenterPoint().x + width/2;
		m_checkLock.MoveWindow(&rectCheckLock);

		CRect rectRSlider;
		m_rSlider.GetWindowRect(&rectRSlider);
		ScreenToClient(&rectRSlider);
		rectRSlider.left = rect.left;
		rectRSlider.right = rect.right;
		m_rSlider.MoveWindow(&rectRSlider);
	}
}
