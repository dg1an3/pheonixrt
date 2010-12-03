// Copyright (C) 2nd Messenger Systems
// $Id: PlanarView.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"
#include "brimstone.h"
#include "PlanarView.h"

#ifdef USE_IPP
#include <ippi.h>
#endif

#include <Series.h>

/////////////////////////////////////////////////////////////////////////////
// CPlanarView

////////////////////////////////////////////////////////////////////////////////////////////
CPlanarView::CPlanarView()
	: m_bWindowLeveling(false)
	, m_bZooming(false)
	, m_bPanning(false)
	, m_bPanningZ(false)
	, m_bLockToSlice(true)
	, m_bEditContourMode(true)
	, m_bAddContourMode(false)
	, m_pSeries(NULL)
	, m_pSelectedStructure(NULL)
	// , m_pNewContour(NULL)
	//, m_pSelectedContour(NULL)
	, m_SelectedVertex(-1)
{
	m_pVolume[0] = NULL;
	m_pVolume[1] = NULL;

	m_volumeResamp[0] = VolumeReal::New(); 
	m_volumeResamp[1] = VolumeReal::New(); 

	m_window[0] = (REAL) 400.0;
	m_window[1] = (REAL) 400.0; // (REAL) 1.0 / 0.8;

	m_level[0] = (REAL) 30.0;
	m_level[1] = (REAL) 30.0; // 0.4;

	m_alpha = (REAL) 1.0;
}

////////////////////////////////////////////////////////////////////////////////////////////
CPlanarView::~CPlanarView()
{
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetSeries(dH::Series *pSeries)
{
	m_pSeries = pSeries;
	SetVolume(GetSeries()->GetDensity(), 0);
}


////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetVolume(VolumeReal *pVolume, int nVolumeAt)
{
	if (nVolumeAt == 1 && m_pVolume[1] == NULL)
	{
		m_alpha = 0.75;
	}

	m_pVolume[nVolumeAt] = pVolume;

	if (nVolumeAt == 0)
	{
		// set up initial zoom
		InitZoomCenter();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetBasis(const Point<REAL>& origin, const Vector<REAL>& spacing)
{
	m_volumeResamp[0]->SetOrigin(origin); 
	m_volumeResamp[0]->SetSpacing(spacing); 
	m_volumeResamp[1]->SetOrigin(origin); 
	m_volumeResamp[1]->SetSpacing(spacing); 
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetWindowLevel(REAL win, REAL cen, int nVolumeAt)
{
	m_window[nVolumeAt] = win;
	m_level[nVolumeAt] = cen;
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetAlpha(REAL alpha)
{
	m_alpha = alpha;
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetLUT(CArray<COLORREF, COLORREF>& arrLUT, int nVolumeAt)
{
	m_arrLUT[nVolumeAt].Copy(arrLUT);
}

/////////////////////////////////////////////////////////////////////////////
inline void Resample(const VolumeReal *pOrig, 
										 VolumeReal *pNew, 
										 BOOL bBilinear = FALSE,
										 int nSlice = 0)
{
	itk::Matrix<REAL, 4, 4> mBasisOrig;
	CalcBasis<3>(pOrig, mBasisOrig);

	itk::Matrix<REAL, 4, 4> mBasisNew;
	CalcBasis<3>(pNew, mBasisNew);

	itk::Matrix<REAL, 4, 4> mXform = mBasisOrig.GetInverse();
	mXform *= mBasisNew;

	// calculate plane
	REAL planeZ = (pNew->GetOrigin()[2] - pOrig->GetOrigin()[2]) / pOrig->GetSpacing()[2];

	// calculate original voxel starting position
	const VOXEL_REAL *pOrigVoxel = pOrig->GetBufferPointer();
	VolumeReal::IndexType idx;
	idx[0] = 0;
	idx[1] = 0;
	idx[2] = // nSlice; // 
		::Round<int>(planeZ);
	if (!pOrig->GetBufferedRegion().IsInside(idx))
	{
		return;
	}
	pOrigVoxel += pOrig->ComputeOffset(idx);

	double coeffs[2][3];
	coeffs[0][0] = mXform(0, 0);
	coeffs[0][1] = mXform(0, 1);
	coeffs[0][2] = mXform(0, 3);
	coeffs[1][0] = mXform(1, 0);
	coeffs[1][1] = mXform(1, 1);
	coeffs[1][2] = mXform(1, 3);

	IppStatus stat = ippiWarpAffineBack_32f_C1R(
		pOrigVoxel, 
		MakeIppiSize(pOrig->GetBufferedRegion()),
		pOrig->GetBufferedRegion().GetSize()[0] * sizeof(VOXEL_REAL), 
		MakeIppiRect(pOrig->GetBufferedRegion()),
		pNew->GetBufferPointer(), 
		pNew->GetBufferedRegion().GetSize()[0] * sizeof(VOXEL_REAL), 
		MakeIppiRect(pNew->GetBufferedRegion()),
		coeffs, IPPI_INTER_LINEAR);

}	// Resample


////////////////////////////////////////////////////////////////////////////////////////////
void CPlanarView::DrawImages(CDC *pDC)
{
	BOOL bDraw = FALSE;

	CRect rect;
	GetClientRect(&rect);

	VOXEL_REAL *pVoxels0 = NULL;
	if (m_pVolume[0] && m_pVolume[0]->GetBufferedRegion().GetSize()[0] != 0)
	{
		if (m_volumeResamp[0]->GetBufferedRegion().GetSize()[0] != rect.Width())
		{
			m_volumeResamp[0]->SetRegions(MakeSize(rect.Width(), rect.Height(), 1));
			m_volumeResamp[0]->Allocate();

			// FIX for actual width
			rect.right = rect.left + (int) m_volumeResamp[0]->GetBufferedRegion().GetSize()[0];
			ASSERT(rect.Width() == m_volumeResamp[0]->GetBufferedRegion().GetSize()[0]);

			// recalc zoom / pan
			SetCenter(m_vCenter);
		}

		m_volumeResamp[0]->FillBuffer(0.0); 
		Resample(m_pVolume[0], m_volumeResamp[0], TRUE);

		pVoxels0 = m_volumeResamp[0]->GetBufferPointer(); 

		bDraw = TRUE;
	}

	VOXEL_REAL *pVoxels1 = NULL; 
	if (m_pVolume[1] && m_pVolume[1]->GetBufferedRegion().GetSize()[1] != 0)
	{
		ConformTo<VOXEL_REAL,3>(m_volumeResamp[0], m_volumeResamp[1]);
		m_volumeResamp[1]->FillBuffer(0.0); 
		Resample(m_pVolume[1], m_volumeResamp[1], TRUE);

		pVoxels1 = m_volumeResamp[1]->GetBufferPointer(); 
		bDraw = TRUE;
	}

	if (!bDraw)
		return;
	
	m_arrPixels.SetSize(rect.Width() * rect.Height());

	VOXEL_REAL alpha1 = (VOXEL_REAL) (1.0 - m_alpha);
	for (int nAt = 0; nAt < rect.Width() * rect.Height(); nAt++)
	{
		VOXEL_REAL pix_value0 = (VOXEL_REAL) 
			(128.0 / m_window[0] * (pVoxels0[nAt] - m_level[0]) + 128.0);

		// scale to 0..255
		pix_value0 = (VOXEL_REAL) __min(pix_value0, 255.0);
		pix_value0 = (VOXEL_REAL) __max(pix_value0, 0.0);

		if (!pVoxels1)
		{
			m_arrPixels[nAt] = 
				RGB(pix_value0, pix_value0, pix_value0);
		}
		else
		{
			// are we using LUT???
			if (m_arrLUT[1].GetSize() > 0)
			{
				VOXEL_REAL max_value = (VOXEL_REAL) m_arrLUT[1].GetSize();
				VOXEL_REAL pix_value1 = (VOXEL_REAL) 
					(max_value * m_window[1] * (pVoxels1[nAt] - m_level[1]) + max_value / 2.0);

				// scale to 0..255
				pix_value1 = (VOXEL_REAL) __min(pix_value1, max_value - 1.0);
				pix_value1 = (VOXEL_REAL) __max(pix_value1, 0.0);

				//  colorIndex = (int) pix_value1 // * (VOXEL_REAL) (max_value - 1.0));
				int colorIndex = Round<int>(__min(pix_value1, max_value - 1.0));
				COLORREF color1 = m_arrLUT[1][colorIndex];

				if (colorIndex > 5)
				{
					m_arrPixels[nAt] = 
						RGB(
							m_alpha * pix_value0 + alpha1 * GetBValue(color1), 
							m_alpha * pix_value0 + alpha1 * GetGValue(color1),
							m_alpha * pix_value0 + alpha1 * GetRValue(color1)
							);
				}
				else
				{
					m_arrPixels[nAt] = 
						RGB(pix_value0, pix_value0, pix_value0);
				}
			}
			else
			{
				VOXEL_REAL pix_value1 = (VOXEL_REAL) 
					(128.0 / m_window[1] * (pVoxels1[nAt] - m_level[1]) + 128.0);

				// scale to 0..255
				pix_value1 = (VOXEL_REAL) __min(pix_value1, 255.0);
				pix_value1 = (VOXEL_REAL) __max(pix_value1, 0.0);

				m_arrPixels[nAt] = 
					RGB(
						m_alpha * pix_value0 + alpha1 * pix_value1, 
						m_alpha * pix_value0 + alpha1 * pix_value1,
						m_alpha * pix_value0 + alpha1 * pix_value1
						);
			}
		}
	}

	
	if (m_dib.GetSize().cx != rect.Width())
	{
		m_dib.DeleteObject();
		HBITMAP bm = ::CreateCompatibleBitmap(*pDC, rect.Width(), rect.Height());
		m_dib.Attach(bm);
	}
	DWORD dwCount = m_dib.SetBitmapBits(rect.Width() * rect.Height() * sizeof(COLORREF),
		(void *) m_arrPixels.GetData());

	// PLDrawBitmap(*pDC, &m_dib, NULL, NULL, SRCCOPY);
}

////////////////////////////////////////////////////////////////////////////////////////////
CPoint 
	ToDC(const itk::Point<REAL,2>& vVert, const itk::Point<REAL>& origin, const itk::Vector<REAL>& spacing)
{
	CPoint pt;
	pt.x = Round<LONG>((vVert[0] - origin[0]) / spacing[0]);
	pt.y = Round<LONG>((vVert[1] - origin[1]) / spacing[1]);

	return pt;
}

////////////////////////////////////////////////////////////////////////////////////////////
CPoint 
	ToDC(REAL vX, REAL vY, const itk::Point<REAL>& origin, const itk::Vector<REAL>& spacing)
{
	CPoint pt;
	pt.x = Round<LONG>((vX - origin[0]) / spacing[0]);
	pt.y = Round<LONG>((vY - origin[1]) / spacing[1]);

	return pt;
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::DrawContours(CDC *pDC)
{
	if (!m_pSeries)
		return;

	// get geom parameters for display
	const Point<REAL>& origin = m_volumeResamp[0]->GetOrigin();
	const Vector<REAL>& spacing = m_volumeResamp[0]->GetSpacing();

	for (int nAtStruct = 0; nAtStruct < m_pSeries->GetStructureCount(); nAtStruct++)
	{
		dH::Structure *pStruct = m_pSeries->GetStructureAt(nAtStruct);

		if (!pStruct->GetVisible())
		{
			continue;
		}

		CPen pen(PS_SOLID, 2, pStruct->GetColor());
		CPen *pOldPen = pDC->SelectObject(&pen);
		pDC->SelectStockObject(HOLLOW_BRUSH);

		for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
		{
			REAL zPos = pStruct->GetContourRefDist(nAtContour);
			if (IsApproxEqual(zPos, origin[2]/*slicePos*/, spacing[2])) // 1.0))	// TODO: use actual slice spacing for volume
			{
				dH::Structure::PolygonType *pPoly = pStruct->GetContour(nAtContour);

				pDC->MoveTo(ToDC(pPoly->GetPoint(0)->GetPosition(), origin, spacing));
				for (int nAtVert = 1; nAtVert < pPoly->GetNumberOfPoints(); nAtVert++)
				{
					pDC->LineTo(ToDC(pPoly->GetPoint(nAtVert)->GetPosition(), origin, spacing));
				}
				pDC->LineTo(ToDC(pPoly->GetPoint(0)->GetPosition(), origin, spacing));
			}
		}

		// do we need to display the points?
		if (GetSelectedStructure() == pStruct)
		{
			/// this only applies to a new contour
			//if (GetSelectedContour() != NULL)
			//{
			//	pDC->MoveTo(ToDC(GetSelectedContour()->GetVertexAt(0), origin, spacing));
			//	for (int nAtVert = 1; nAtVert < GetSelectedContour()->GetVertexCount(); nAtVert++)
			//	{
			//		pDC->LineTo(ToDC(GetSelectedContour()->GetVertexAt(nAtVert), origin, spacing));
			//	}

			//	// TODO: do we close it?

			//	for (int nAtVert = 0; nAtVert < GetSelectedContour()->GetVertexCount(); nAtVert++)
			//	{
			//		CRect rect(ToDC(GetSelectedContour()->GetVertexAt(nAtVert), origin, spacing), CSize(5, 5));
			//		rect -= CPoint(2, 2);
			//		pDC->Rectangle(rect);
			//	}
			// }

			// TODO: fix this (i.e. save the selected contours zPos)
			for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
			{
				if (GetSelectedContour() == pStruct->GetContour(nAtContour))
				{
					REAL zPos = pStruct->GetContourRefDist(nAtContour);
					if (IsApproxEqual(zPos, origin[2]/*slicePos*/, spacing[2])) // 1.0))	// TODO: use actual slice spacing for volume
					{
						CPen penHandle(PS_SOLID, 1, RGB(224, 224, 224));
						pDC->SelectObject(&penHandle);
						dH::Structure::PolygonType *pContour = GetSelectedContour();
						for (int nAtVert = 0; nAtVert < pContour->GetNumberOfPoints(); nAtVert++)
						{
							CRect rect(ToDC(pContour->GetPoint(nAtVert)->GetPosition(), origin, spacing), CSize(5, 5));
							rect -= CPoint(2, 2);
							pDC->Rectangle(rect);
						}
					}
				}
			}
		}

		pDC->SelectObject(pOldPen);
	}

}

bool 
CPlanarView::ContourHitTest(CPoint& point, dH::Structure::PolygonType *pContour, int *pnVertex) // Vector<REAL, 2>*& pVertex)
{	
	// get geom parameters for display
	const Point<REAL>& origin = m_volumeResamp[0]->GetOrigin();
	const Vector<REAL>& spacing = m_volumeResamp[0]->GetSpacing();

	// see if we hit a handle
	for ((*pnVertex) = 0; (*pnVertex) < pContour->GetNumberOfPoints(); (*pnVertex)++)
	{
		// pVertex = // const_cast< Vector<REAL, 2> * >(&pContour->GetVertexAt(nVertex));

		CRect rectHandle(ToDC(pContour->GetPoint(*pnVertex)->GetPosition(), origin, spacing), CSize(4, 4));
		rectHandle -= CPoint(2, 2);
		rectHandle.InflateRect(5, 5);
		if (rectHandle.PtInRect(point))
		{
			return true;
		}
	}

	// no handle, so there is no selected vertex
	(*pnVertex) = -1;

	// see if we hit the region
	CRgn *pRgn = GetRgnForContour(pContour);
	bool ptInRegion = pRgn->PtInRegion(point) ? true : false;
	pRgn->DeleteObject();
	delete pRgn;

	return ptInRegion;
}

CRgn *
	CPlanarView::GetRgnForContour(dH::Structure::PolygonType *pContour)
{
	// generate a DC to draw the contour (conforms with current window)
	CDC *pCommonDC = GetDC();
	CDC dc;
	dc.CreateCompatibleDC(pCommonDC);
	ReleaseDC(pCommonDC);

	// get geom parameters for display
	const Point<REAL>& origin = m_volumeResamp[0]->GetOrigin();
	const Vector<REAL>& spacing = m_volumeResamp[0]->GetSpacing();

	// draw the path
	dc.BeginPath();

	dc.MoveTo(ToDC(pContour->GetPoint(0)->GetPosition(), origin, spacing));
	for (int nAtVert = 1; nAtVert < pContour->GetNumberOfPoints(); nAtVert++)
	{
		dc.LineTo(ToDC(pContour->GetPoint(nAtVert)->GetPosition(), origin, spacing));
	}
	dc.LineTo(ToDC(pContour->GetPoint(0)->GetPosition(), origin, spacing));

	dc.EndPath();

	// create the region
	CRgn *pRgn = new CRgn();
	pRgn->CreateFromPath(&dc);

	return pRgn;
}
////////////////////////////////////////////////////////////////////////////////////////////
double calcPoint(double bottom, double top, double c, double tempstep)
{             
	return ((c-bottom)/(top-bottom))*tempstep; 
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::DrawIsocurves(VolumeReal *pVolume, REAL c, CDC *pDC)
{
	const itk::Point<REAL>& srcOrigin = pVolume->GetOrigin();
	const itk::Vector<REAL>& srcSpacing = pVolume->GetSpacing();

	const itk::Point<REAL>& dstOrigin = m_volumeResamp[0]->GetOrigin();
	const itk::Vector<REAL>& dstSpacing = m_volumeResamp[0]->GetSpacing();

	double topleft = 0;
	double topright = 0;
	double bottomleft = 0;
	double bottomright = 0;

	for(int nY = 0; nY < ((int)pVolume->GetBufferedRegion().GetSize()[1])-1; nY++)
	{          
		for(int nX = 0; nX < ((int)pVolume->GetBufferedRegion().GetSize()[0])-1; nX++)
		{  
			int nStrideZ = Round<int>((dstOrigin[2] - srcOrigin[2])/srcSpacing[2]);
			if (nStrideZ < 0 || nStrideZ >= pVolume->GetBufferedRegion().GetSize()[2])
				continue;
			nStrideZ *= pVolume->GetBufferedRegion().GetSize()[0] * pVolume->GetBufferedRegion().GetSize()[1];
			int nStrideY = pVolume->GetBufferedRegion().GetSize()[0];
			if(nX == 0)
			{
				topleft = pVolume->GetBufferPointer()[nStrideZ + (nY+1)*nStrideY + nX]; 
				topright = pVolume->GetBufferPointer()[nStrideZ + (nY+1)*nStrideY + (nX+1)]; 
				bottomleft = pVolume->GetBufferPointer()[nStrideZ + (nY)*nStrideY + (nX)]; 
				bottomright = pVolume->GetBufferPointer()[nStrideZ + (nY)*nStrideY + (nX+1)]; 
			}
			else
			{
				topleft = topright;
				topright = pVolume->GetBufferPointer()[nStrideZ + (nY+1)*nStrideY + (nX+1)]; 
				bottomleft = bottomright;
				bottomright = pVolume->GetBufferPointer()[nStrideZ + (nY)*nStrideY + (nX+1)]; 
			}         
			
			//case 1.1
			if((topleft<c && topright<c && bottomleft>c && bottomright>c) 
				|| (topleft>c && topright>c && bottomleft<c && bottomright<c))
			{
				// points are divided down some line in the middle
				double left = calcPoint(bottomleft, topleft, c, srcSpacing[1]);
				double right = calcPoint(bottomright, topright, c, srcSpacing[1]);
					
				pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX, 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY + left, dstOrigin, dstSpacing));
				pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) (nX+1), 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY + right, dstOrigin, dstSpacing));
			} 

			//case 1.2 
			else if((topleft<c && topright>c && bottomleft<c && bottomright>c)
				|| (topleft>c && topright<c && bottomleft>c && bottomright<c))
			{
				// points are divided down some line in the middle
				double bottom = calcPoint(bottomleft, bottomright, c, srcSpacing[0]);    
				double top = calcPoint(topleft, topright, c, srcSpacing[0]);
				
				pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + bottom, 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY, dstOrigin, dstSpacing));
				pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + top, 
					srcOrigin[1] + srcSpacing[1] * (REAL) (nY + 1), dstOrigin, dstSpacing));
			} 
			
			//case 2              
			else if((topleft<c && topright<c && bottomleft<c && bottomright<c)
				|| (topleft>c && topright>c && bottomleft>c && bottomright>c)) 
			{
				// all points below or above the line, so dont do anything
			}  
			//case 3.1              
			else if((topleft>c && topright>c && bottomleft>c && bottomright<c)
				||(topleft<c && topright<c && bottomleft<c && bottomright>c))
			{
				// three points above or below the line and the other the opposite
				double right = calcPoint(bottomright, topright, c, srcSpacing[1]);
				double bottom = calcPoint(bottomleft, bottomright, c, srcSpacing[0]);
				
				pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + bottom, 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY, dstOrigin, dstSpacing));
				pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) (nX+1), 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY + right, dstOrigin, dstSpacing));
			}  
			//case 3.2              
			else if((topleft>c && topright>c && bottomleft<c && bottomright>c)
				||(topleft<c && topright<c && bottomleft>c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double left = calcPoint(bottomleft, topleft, c, srcSpacing[1]);
				double bottom = calcPoint(bottomleft, bottomright, c, srcSpacing[0]);
				
				pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + bottom, 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY, dstOrigin, dstSpacing));
				pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX, 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY + left, dstOrigin, dstSpacing));
			}  
			//case 3.3             
			else if((topleft<c && topright>c && bottomleft>c && bottomright>c)
				||(topleft>c && topright<c && bottomleft<c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double left = calcPoint(bottomleft, topleft, c, srcSpacing[1]);
				double top = calcPoint(topleft, topright, c, srcSpacing[0]);
				
				pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX, 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY + left, dstOrigin, dstSpacing));
				pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + top, 
					srcOrigin[1] + srcSpacing[1] * (REAL) (nY+1), dstOrigin, dstSpacing));
			}  
			//case 3.4              
			else if((topleft>c && topright<c && bottomleft>c && bottomright>c)
				||(topleft<c && topright>c && bottomleft<c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double top = calcPoint(topleft, topright, c, srcSpacing[0]);
				double right = calcPoint(bottomright, topright, c, srcSpacing[1]);
				
				pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + top, 
					srcOrigin[1] + srcSpacing[1] * (REAL) (nY+1), dstOrigin, dstSpacing));
				pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) (nX+1), 
					srcOrigin[1] + srcSpacing[1] * (REAL) nY + right, dstOrigin, dstSpacing));
			}  
			
			//case 4.1              
			else if(topleft>c && topright<c && bottomleft<c && bottomright>c)
			{
				//Ambigous case where the corners are diametrically opposed
				//Test in the middle and decide which way the lines go..
				double middle = 0.0;
				
				double top = calcPoint(topleft, topright, c, srcSpacing[0]);
				double bottom = calcPoint(bottomleft, bottomright, c, srcSpacing[0]);
				double left = calcPoint(bottomleft, topleft, c, srcSpacing[1]);
				double right = calcPoint(bottomright, topright, c, srcSpacing[1]);
				
				if(middle < c)
				{
					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + left, dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + top, 
						srcOrigin[1] + srcSpacing[1] * (REAL) (nY+1), dstOrigin, dstSpacing));
					
					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + bottom, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY, dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) (nX+1), 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + right, dstOrigin, dstSpacing));
				}
				else if(middle > c)
				{
					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + left, dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + bottom, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY, dstOrigin, dstSpacing));

					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + top, 
						srcOrigin[1] + srcSpacing[1] * (REAL) (nY+1), dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) (nX+1), 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + right, dstOrigin, dstSpacing));
				}
				
			}
			//case 4.2
			else if(topleft<c && topright>c && bottomleft>c && bottomright<c)
			{
				//Ambigous case where the corners are diametrically opposed
				//Test in the middle and decide which way the lines go..
				double middle = 0.0;
				double top = calcPoint(topleft, topright, c, srcSpacing[0]);
				double bottom = calcPoint(bottomleft, bottomright, c, srcSpacing[0]);
				double left = calcPoint(bottomleft, topleft, c, srcSpacing[1]);
				double right = calcPoint(bottomright, topright, c, srcSpacing[1]);
				
				if(middle<c)
				{
					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + left, dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + bottom, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY, dstOrigin, dstSpacing));
					
					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + top, 
						srcOrigin[1] + srcSpacing[1] * (REAL) (nY+1), dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) (nX+1), 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + right, dstOrigin, dstSpacing));
				}
				else if(middle>c)
				{
					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + left, dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + top, 
						srcOrigin[1] + srcSpacing[1] * (REAL) (nY+1), dstOrigin, dstSpacing));

					pDC->MoveTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) nX + bottom, 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY, dstOrigin, dstSpacing));
					pDC->LineTo(ToDC(srcOrigin[0] + srcSpacing[0] * (REAL) (nX+1), 
						srcOrigin[1] + srcSpacing[1] * (REAL) nY + right, dstOrigin, dstSpacing)); 
				} 
				
			}
			
			// nX++; 
    }//close i-loop
	
	// i = imin;
	// nY++;
	
 }//close j-loop
 
 // i = imin;
 // j = jmin;
 // myzcounter++;
 
} // close k-loop


////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::InitZoomCenter(void)
	// sets up initial zoom / pan state
{
	// reset zoom
	m_zoom = 1.0;

	// calculate new center
	m_vCenter[0] = m_pVolume[0]->GetOrigin()[0] 
		+ m_pVolume[0]->GetSpacing()[0] * 0.5 * (REAL) m_pVolume[0]->GetBufferedRegion().GetSize()[0];
	m_vCenter[1] = m_pVolume[0]->GetOrigin()[1] 
		+ m_pVolume[0]->GetSpacing()[1] * 0.5 * (REAL) m_pVolume[0]->GetBufferedRegion().GetSize()[1];
	m_vCenter[2] = m_pVolume[0]->GetOrigin()[2];

	// set the zoom (to set the basis)
	SetZoom(m_zoom);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetCenter(const itk::Vector<REAL>& vCenter)
{
	m_vCenter = vCenter;

	// set the zoom (to set the basis)
	SetZoom(m_zoom);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::SetZoom(REAL zoom /* = 1.0 */)
{
	m_zoom = zoom;

	Vector<REAL> vSpacing = m_pVolume[0]->GetSpacing();
	vSpacing /= m_zoom;

	Point<REAL> vOrigin = m_pVolume[0]->GetOrigin();
	vOrigin[0] = m_vCenter[0] - vSpacing[0] * 0.5 * (REAL) m_volumeResamp[0]->GetBufferedRegion().GetSize()[0];
	vOrigin[1] = m_vCenter[1] - vSpacing[1] * 0.5 * (REAL) m_volumeResamp[0]->GetBufferedRegion().GetSize()[1];  
	vOrigin[2] = m_vCenter[2];
	SetBasis(vOrigin, vSpacing);
}


BEGIN_MESSAGE_MAP(CPlanarView, CWnd)
	//{{AFX_MSG_MAP(CPlanarView)
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


/////////////////////////////////////////////////////////////////////////////
// CPlanarView message handlers


////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);

	DrawImages(&dc);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	// selects image objects in to context
	dcMem.SelectObject(m_dib);

	dcMem.SelectStockObject(HOLLOW_BRUSH);
	dcMem.Rectangle(rect);

	// draw isocurves
	if (m_pVolume[1] != NULL 
		&& m_pVolume[1]->GetBufferedRegion().GetSize()[1] > 0
		&& m_arrLUT[1].GetSize() > 0)
	{
		VOXEL_REAL c = (VOXEL_REAL) 0.3;
		while (c < 1.0)
		{
			VOXEL_REAL max_value = (VOXEL_REAL) m_arrLUT[1].GetSize();
			VOXEL_REAL pix_value1 = (VOXEL_REAL)
				(max_value * m_window[1] * (c - m_level[1]) + max_value / 2.0);

			// scale to 0..255
			pix_value1 = (VOXEL_REAL) __min(pix_value1, max_value - 1.0);
			pix_value1 = (VOXEL_REAL) __max(pix_value1, 0.0);

			int colorIndex = Round<int>(__min(pix_value1, max_value - 1.0));

			CPen pen(PS_SOLID, 1, m_arrLUT[1][colorIndex]);
			dcMem.SelectObject(&pen);
			DrawIsocurves(m_pVolume[1], c, &dcMem);

			c += (VOXEL_REAL) 0.05;
		}
	}

	// draw contours
	DrawContours(&dcMem);

	// draw Z position
	CString strPosition;
	strPosition.Format(_T("Z = %lf"), m_volumeResamp[0]->GetOrigin()[2]);
	dcMem.TextOut(10, 10, strPosition);

	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

	// Do not call CWnd::OnPaint() for painting messages
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	if (NULL != m_pVolume[0])
	{
		// Q: shouldn't the resampleVolume be resized here? No, auto-resized in DrawImages
		// set the zoom (to set the basis)
		// SetCenter(m_vCenter);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void CPlanarView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_bWindowLeveling = true;
	m_windowStart = m_window[0];
	m_levelStart = m_level[0];
	
	m_ptOpStart = point;

	SetCapture();

	CWnd::OnMButtonDown(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////////////////
void CPlanarView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	m_bWindowLeveling = false;
	::ReleaseCapture();

	CWnd::OnMButtonUp(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////////////////
void CPlanarView::OnLButtonDown(UINT nFlags, CPoint point)
{
	const Point<REAL>& sliceOrigin = m_volumeResamp[0]->GetOrigin();
	const Vector<REAL>& sliceSpacing = m_volumeResamp[0]->GetSpacing();
	const Point<REAL>& volumeOrigin = m_pVolume[0]->GetOrigin();
	const Vector<REAL>& volumeSpacing = m_pVolume[0]->GetSpacing();

	if (m_bAddContourMode
		&& GetSelectedStructure() != NULL)
	{
		// make sure we are on the nearest plane
		/// TODO: move this to set mode
		Vector<REAL> vCenter = m_vCenter;
		int nOriginalSliceNumber = Round<int>((sliceOrigin[2] - volumeOrigin[2]) / volumeSpacing[2]);
		vCenter[2] = volumeOrigin[2] + (REAL) nOriginalSliceNumber * volumeSpacing[2]; 
		SetCenter(vCenter);

		if (GetSelectedContour().IsNull())
		{
			// start a new polygon
			SetSelectedContour(dH::Structure::PolygonType::New());
		}

		dH::Structure::PolygonType::PointType vVert;
		vVert[0] = point.x * sliceSpacing[0] + sliceOrigin[0];
		vVert[1] = point.y * sliceSpacing[1] + sliceOrigin[1];
		GetSelectedContour()->AddPoint(vVert);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return;
	}
	else if (m_bEditContourMode
		&& GetSelectedStructure() != NULL)
	{
		dH::Structure *pStruct = GetSelectedStructure();
		SetSelectedContour(NULL);
		SetSelectedVertex(NULL);

		// see if we hit one of the selected structures contours
		for (int nAtContour = 0; nAtContour < pStruct->GetContourCount(); nAtContour++)
		{
			REAL zPos = pStruct->GetContourRefDist(nAtContour);
			if (IsApproxEqual(zPos, sliceOrigin[2]/*slicePos*/, sliceSpacing[2])) // 1.0))	// TODO: use actual slice spacing for volume
			{
				dH::Structure::PolygonType *pContour = pStruct->GetContour(nAtContour);
				int nVertex = -1;
				if (ContourHitTest(point, pContour, &nVertex))
				{
					SetSelectedContour(pContour);
					if (nVertex != -1)
					{
						SetSelectedVertex(nVertex);
						m_ptOpStart = point;
						m_vVertexStart = pContour->GetPoint(nVertex)->GetPosition();
					}
				}
			}
		}

		// update display
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
void CPlanarView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bEditContourMode
		&& GetSelectedVertex() != -1)
	{
		SetSelectedVertex(-1);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_bAddContourMode
		&& GetSelectedContour().IsNotNull())
	{
		const Point<REAL>& origin = m_volumeResamp[0]->GetOrigin();
		GetSelectedStructure()->AddContour(GetSelectedContour(), origin[2]);
		SetSelectedContour(NULL);

		m_bAddContourMode = false;
		m_bEditContourMode = true;

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// form panning region
	CRect rectClient;
	GetClientRect(&rectClient);
	rectClient.DeflateRect(rectClient.Width() / 3, rectClient.Height() / 3,
		rectClient.Width() / 3, rectClient.Height() / 3);

	// check for panning
	if (rectClient.PtInRect(point))
	{
		m_bPanning = true;

		// store basis at start
		CalcBasis<3>(m_volumeResamp[0], m_mBasisStart);

		// calc starting point
		MultHG(m_mBasisStart, MakeVector<3>(point), m_vPtStart);

		// calc center at start
		m_vCenterStart = m_vCenter;
	}
	else if (point.x < rectClient.left || point.x > rectClient.right)
	{
		m_bZooming = true;
		m_zoomStart = m_zoom;
	}
	else
	{
		m_bPanningZ = true;

		// store basis at start
		CalcBasis<3>(m_volumeResamp[0], m_mBasisStart);

		// calc starting point
		MultHG(m_mBasisStart, MakeVector<3>(point), m_vPtStart);

		// calc center at start
		m_vCenterStart = m_vCenter;
	}

	m_ptOpStart = point;

	SetCapture();

	CWnd::OnRButtonDown(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bZooming = false;
	m_bPanning = false;
	m_bPanningZ = false;
	ReleaseCapture();

	CWnd::OnRButtonUp(nFlags, point);
}

////////////////////////////////////////////////////////////////////////////////////////////
void 
	CPlanarView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rectClient;
	GetClientRect(&rectClient);

	CPoint ptDelta = point - m_ptOpStart;

	if (m_bWindowLeveling)
	{
		VOXEL_REAL voxelMax = GetMax<VOXEL_REAL>(m_pVolume[0]);

		m_window[0] = m_windowStart - voxelMax * (VOXEL_REAL) ptDelta.y / (VOXEL_REAL) rectClient.Height();
		m_window[0] = __max(m_window[0], 0.001);

		m_level[0] = m_levelStart - voxelMax * (VOXEL_REAL) ptDelta.x / (VOXEL_REAL) rectClient.Width();

		// update display
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (m_bZooming)
	{
		REAL newZoom = m_zoomStart * exp( - 0.75 * (REAL) ptDelta.y / (REAL) rectClient.Height());

		SetZoom(newZoom);

		// update display
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (m_bPanning)
	{
		Vector<REAL> vPtNext;
		MultHG(m_mBasisStart, MakeVector<3>(point), vPtNext);

		// convert m_vCenter to space
		Vector<REAL> vNewCenter = m_vCenterStart - (vPtNext - m_vPtStart);
		SetCenter(vNewCenter);

		// update display
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (m_bPanningZ)
	{
		Vector<REAL> vPtNext;
		MultHG(m_mBasisStart, MakeVector<3>(point), vPtNext);

		// convert m_vCenter to space
		Vector<REAL> vNewCenter = m_vCenterStart;
		vNewCenter[2] +=  (vPtNext[0] - m_vPtStart[0]) / 1.0;

		if (m_bLockToSlice)
		{
			const Point<REAL>& origin = m_pVolume[0]->GetOrigin();
			const Vector<REAL>& spacing = m_pVolume[0]->GetSpacing();
			vNewCenter[2] = Round<int>((vNewCenter[2] - origin[2]) / spacing[2]) * spacing[2] + origin[2];
		}
		SetCenter(vNewCenter);

		// update display
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (m_bEditContourMode 
		&& GetSelectedVertex() != -1)
	{
		// get geom parameters for display
		const Point<REAL>& origin = m_volumeResamp[0]->GetOrigin();
		const Vector<REAL>& spacing = m_volumeResamp[0]->GetSpacing();
	
		// compute shift of point
		dH::Structure::PolygonType::PointType vShift = GetSelectedContour()->GetPoint(GetSelectedVertex())->GetPosition();
		vShift[0] = spacing[0] * ptDelta.x; // + origin[0];
		vShift[1] = spacing[1] * ptDelta.y; // + origin[1];

		GetSelectedContour()->ReplacePoint(GetSelectedContour()->GetPoint(GetSelectedVertex())->GetPosition(),
			vShift);

		// update display
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	CWnd::OnMouseMove(nFlags, point);
}
