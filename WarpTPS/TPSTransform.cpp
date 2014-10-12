//////////////////////////////////////////////////////////////////////
// TPSTransform.cpp: implementation of the CTPSTransform class.
//
// Copyright (C) 2002
// $Id: TPSTransform.cpp,v 1.3 2003/04/26 20:52:46 default Exp $
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <MatrixNxM.h>
#include <MatrixBase.inl>

#include "TPSTransform.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// distance_function
// 
// returns the log-distance squared betweeen two landmarks
//////////////////////////////////////////////////////////////////////
double distance_function(const CVectorD<3>& vL1, const CVectorD<3>& vL2, const REAL k = 1.0, REAL r_exp = 2.0)
{
	// compute the euclidean distance
	double r = (vL1 - vL2).GetLength();

#ifdef SQUARE_ONLY
	// pass through the log-squared
	return (r > 0.0) ? (k * r * r * log(r)) : 0.0;
#else
	return (r > 0.0) ? (k * pow(r, r_exp) * log(r)) : 0.0;
#endif
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CTPSTransform::CTPSTransform
// 
// constructs a CTPSTransform object with the given name
//////////////////////////////////////////////////////////////////////
CTPSTransform::CTPSTransform()
	: m_bRecalcMatrix(TRUE)
	, m_bRecalc(TRUE)
	, m_bRecalcPresample(TRUE)
	, m_presampledWidth(0)
	, m_presampledHeight(0)
	, m_k(1.0)
	, m_r_exp(2.0)
{
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::CTPSTransform
// 
// destroys the TPSTransform
//////////////////////////////////////////////////////////////////////
CTPSTransform::~CTPSTransform()
{
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::GetLandmarkCount
// 
// returns the number of defined landmarks for the TPS
//////////////////////////////////////////////////////////////////////
int CTPSTransform::GetLandmarkCount()
{
	return m_arrLandmarks[0].size();
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::GetLandmark
// 
// returns a landmark
//////////////////////////////////////////////////////////////////////
const CVectorD<3>& CTPSTransform::GetLandmark(int nDataSet, int nIndex)
{
	// return the landmark at the given index
	return m_arrLandmarks[nDataSet].at(nIndex);
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::SetLandmark
// 
// constructs a CTPSTransform object with the given name
//////////////////////////////////////////////////////////////////////
void CTPSTransform::SetLandmark(int nDataSet, int nIndex, const CVectorD<3>& vLandmark)
{
	// assign the landmark
	m_arrLandmarks[nDataSet].at(nIndex) = vLandmark;

	// only recalc matrix if it is dataset 0
	if (nDataSet == 0)
	{
		m_bRecalcMatrix = TRUE;
	}

	// set the flag to indicate recalculation is needed
	m_bRecalc = TRUE;
	m_bRecalcPresample = TRUE;

	// fire a change
	GetChangeEvent().Fire();
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::AddLandmark
// 
// adds a new landmark to the TPS
//////////////////////////////////////////////////////////////////////
int CTPSTransform::AddLandmark(const CVectorD<3>& vLandmark)
{
	// return the index of the new landmark
	return AddLandmark(vLandmark, vLandmark);
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::AddLandmark
// 
// adds a new landmark to the TPS
//////////////////////////////////////////////////////////////////////
int CTPSTransform::AddLandmark(const CVectorD<3>& vLandmark1, 
							   const CVectorD<3>& vLandmark2)
{
	// add the landmark to both data sets
	m_arrLandmarks[0].push_back(vLandmark1);
	m_arrLandmarks[1].push_back(vLandmark2);

	// set the flag to indicate recalculation is needed
	m_bRecalcMatrix = TRUE;
	m_bRecalc = TRUE;
	m_bRecalcPresample = TRUE;

	// fire a change
	GetChangeEvent().Fire();

	// return the index of the new landmark
	return GetLandmarkCount()-1;
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::RemoveAllLandmarks
// 
// removes all landmarks from the transform
//////////////////////////////////////////////////////////////////////
void CTPSTransform::RemoveAllLandmarks()
{
	m_arrLandmarks[0].clear();
	m_arrLandmarks[1].clear();

	m_bRecalcMatrix = TRUE;
	m_bRecalc = TRUE;
	m_bRecalcPresample = TRUE;
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::Eval
// 
// evaluates the vector field at a point
//////////////////////////////////////////////////////////////////////
void CTPSTransform::Eval(const CVectorD<3>& vPos, CVectorD<3>& vOffset, float percent)
{
	// start with transformed equal to input
	// vPosTrans = vPos;

	// don't compute without at least three landmarks
	if (GetLandmarkCount() >= 3)
	{
		// see if a recalc is needed
		if (m_bRecalc)
		{
			// recalculate the weight vectors
			RecalcWeights();
		}

		// add the weight vector displacements
		for (int nAt = 0; nAt < GetLandmarkCount(); nAt++)
		{
			// distance to the first landmark
			double d = distance_function(vPos, GetLandmark(0, nAt), m_k, m_r_exp);

			// add weight vector displacements
			vOffset[0] += d * m_vWx[nAt] * percent;
			vOffset[1] += d * m_vWy[nAt] * percent;
		}

		// add the affine displacements
		vOffset[0] += 
			(m_vWx[GetLandmarkCount() + 0] 
				+ m_vWx[GetLandmarkCount() + 1] * vPos[0] 
				+ m_vWx[GetLandmarkCount() + 2] * vPos[1])*percent;

		vOffset[1] += 
			(m_vWy[GetLandmarkCount() + 0] 
				+ m_vWy[GetLandmarkCount() + 1] * vPos[0] 
				+ m_vWy[GetLandmarkCount() + 2] * vPos[1])*percent;
	}
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::Resample
// 
// resamples a source image through the transform
//////////////////////////////////////////////////////////////////////
void CTPSTransform::Presample(int width, int height)
{
	if (width != m_presampledWidth
		|| height != m_presampledHeight)
	{
		m_presampledWidth = width;
		m_presampledHeight = height;
		m_presampledOffsets.resize(width * height);
		m_bRecalcPresample = TRUE;
	}

	CVectorD<3> vZero;
	for (int nAt = 0; nAt < m_presampledOffsets.size(); nAt++)
		m_presampledOffsets[nAt] = vZero;

	if (m_bRecalcPresample)
	{
		// position of the destination
		CVectorD<3> vDstPos; 

		// position of the source
		CVectorD<3> vSrcPos;

		// for each pixel in the image
		int dstAtY = 0;
		for (dstAtY = 0, vDstPos[1] = 0.0; vDstPos[1] < height; dstAtY++, vDstPos[1] += 1.0)
		{
			int dstAtX = 0;
			for (dstAtX = 0, vDstPos[0] = 0.0; vDstPos[0] < width; dstAtX++, vDstPos[0] += 1.0)
			{
				CVectorD<3>& vOffset = m_presampledOffsets[dstAtY * m_presampledWidth + dstAtX];
				Eval(vDstPos, vOffset, 1.0);

				// remove initial position to leave true offset
				// vSrcPos -= vDstPos;
			}
		}

		m_bRecalcPresample = FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::Resample
// 
// resamples a source image through the transform
//////////////////////////////////////////////////////////////////////
void CTPSTransform::Resample(CDib *pImageSrc, CDib *pImageDst, float percent)
{
	// get the number of bytes-per-pixel for each
	BITMAP srcBitmap;
	pImageSrc->GetBitmap(&srcBitmap);
	int nSrcBytesPerPixel = (srcBitmap.bmBitsPixel+7)/8;

	// ensure the scan line width is on a LONG boundary
	int nSrcWidthBytes = 4 * ((srcBitmap.bmWidth * nSrcBytesPerPixel + 3) / 4);

	BITMAP dstBitmap;
	pImageDst->GetBitmap(&dstBitmap);
	int nDstBytesPerPixel = (dstBitmap.bmBitsPixel+7)/8;

	// ensure the scan line width is on a LONG boundary
	int nDstWidthBytes = 4 * ((dstBitmap.bmWidth * nDstBytesPerPixel + 3) / 4);

	// should have same pixel format
	ASSERT(nDstBytesPerPixel == nSrcBytesPerPixel);
	ASSERT(nDstWidthBytes == nSrcWidthBytes);

	// get the size of the source image
	CSize srcSize = pImageSrc->GetSize();
	CSize dstSize = pImageDst->GetSize();

	// retrieve the source and destination pixels
	UCHAR *pSrcPixels = (UCHAR *) pImageSrc->GetDIBits();
	UCHAR *pDstPixels = (UCHAR *) pImageDst->GetDIBits();

	TRACE("Resampling...\n");

#define USE_PRESAMPLED
#ifdef USE_PRESAMPLED
	// see if a recalc is needed
	if (m_bRecalc)
	{
		// recalculate the weight vectors
		RecalcWeights();
	}

	// if there are no landmarks, then no presampled vector field
	if (m_bRecalcPresample
		|| m_presampledWidth != srcSize.cx
		|| m_presampledHeight != srcSize.cy)
	{
		Presample(srcSize.cx, srcSize.cy);
	}

	CVectorD<3> vSrcPos;
	for (int dstAtY = 0; dstAtY < dstSize.cy; dstAtY++)
	{
		for (int dstAtX = 0; dstAtX < dstSize.cx; dstAtX++)
		{
			// compute the destination position
			int nDstY = dstSize.cy - dstAtY- 1;
			int nDstIndex = nDstBytesPerPixel * dstAtX 
				+ nDstY * nDstWidthBytes;

			vSrcPos[0] = dstAtX;
			vSrcPos[1] = dstAtY;
			vSrcPos += ((double)percent)*m_presampledOffsets[dstAtY * m_presampledWidth + dstAtX];

			// bounds check source position
			int nSrcY = srcSize.cy - (int)floor(vSrcPos[1]+0.5) - 1;
			if (vSrcPos[0] >= 0.0 && vSrcPos[0] < srcSize.cx
				&& nSrcY >= 0 && nSrcY < srcSize.cy)
			{
				// compute the positions
				int nSrcIndex = nSrcBytesPerPixel * (int) floor(vSrcPos[0]+0.5)
					+ nSrcY * nSrcWidthBytes;

				// and resample
				memcpy(&pDstPixels[nDstIndex], &pSrcPixels[nSrcIndex], 
					nDstBytesPerPixel);
			}
			else
			{
				// set to zero
				memset(&pDstPixels[nDstIndex], 0, nDstBytesPerPixel);
			}
		}
	}
#else
	// position of the destination
	CVectorD<3> vDstPos; 

	// position of the source
	CVectorD<3> vOffset;
	CVectorD<3> vSrcPos;

	// for each pixel in the image
	for (vDstPos[1] = 0.0; vDstPos[1] < dstSize.cy; vDstPos[1] += 1.0)
	{
		for (vDstPos[0] = 0.0; vDstPos[0] < dstSize.cx; vDstPos[0] += 1.0)
		{
			Eval(vDstPos, vOffset, percent);
			vSrcPos = vDstPos + vOffset;

			// compute the destination position
			int nDstY = dstSize.cy - (int) vDstPos[1] - 1;
			int nDstIndex = nDstBytesPixel * (int) vDstPos[0] 
				+ nDstY * nDstWidthBytes;

			// bounds check source position
			int nSrcY = srcSize.cy - (int)floor(vSrcPos[1]+0.5) - 1;
			if (vSrcPos[0] >= 0.0 && vSrcPos[0] < srcSize.cx
				&& nSrcY >= 0 && nSrcY < srcSize.cy)
			{
				// compute the positions
				int nSrcIndex = nSrcBytesPerPixel * (int) floor(vSrcPos[0]+0.5)
					+ nSrcY * nSrcWidthBytes;

				// and resample
				memcpy(&pDstPixels[nDstIndex], &pSrcPixels[nSrcIndex], 
					nDstBytesPerPixel);
			}
			else
			{
				// set to zero
				memset(&pDstPixels[nDstIndex], 0, nDstBytesPerPixel);
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::RecalcWeights
// 
// recalcs the TPS weights from the landmarks
//////////////////////////////////////////////////////////////////////
void CTPSTransform::RecalcWeights()
{
	// don't compute without at least three landmarks
	if (GetLandmarkCount() < 3)
	{
		return;
	}

	if (m_bRecalcMatrix)
	{
		// stores the L matrix
		CMatrixNxM<> mL;
		mL.Reshape(GetLandmarkCount() + 3, GetLandmarkCount() + 3);

		// iterate over the rows and columns of the L matrix
		int nAtRow;
		int nAtCol;
		for (nAtRow = 0; nAtRow < GetLandmarkCount(); nAtRow++)
		{
			for (nAtCol = 0; nAtCol < GetLandmarkCount(); nAtCol++)
			{
				// are we off-diagonal?
				if (nAtRow != nAtCol)
				{
					// populate the K part of the matrix
					mL[nAtCol][nAtRow] = 
						distance_function(GetLandmark(0, nAtRow), 
							GetLandmark(0, nAtCol), m_k, m_r_exp);
				}
				else
				{
					// zeros on the diagonal
					mL[nAtCol][nAtRow] = 0.0;
				}

				// populate the Q^T part of the matrix
				mL[nAtCol][GetLandmarkCount() + 0] = 1.0;
				mL[nAtCol][GetLandmarkCount() + 1] = GetLandmark(0, nAtCol)[0];
				mL[nAtCol][GetLandmarkCount() + 2] = GetLandmark(0, nAtCol)[1];
			}

			// populate the Q part of the matrix
			mL[GetLandmarkCount() + 0][nAtRow] = 1.0;
			mL[GetLandmarkCount() + 1][nAtRow] = GetLandmark(0, nAtRow)[0];
			mL[GetLandmarkCount() + 2][nAtRow] = GetLandmark(0, nAtRow)[1];
		}

		// fill the lower-right 3x3 with zeros
		for (nAtRow = GetLandmarkCount(); nAtRow < GetLandmarkCount()+3; nAtRow++)
		{
			for (nAtCol = GetLandmarkCount(); nAtCol < GetLandmarkCount()+3; nAtCol++)
			{
				mL[nAtCol][nAtRow] = 0.0;
			}
		}

		// form the inverse of L
		m_mL_inv.Reshape(mL.GetCols(), mL.GetRows());
		m_mL_inv = mL;
		m_mL_inv.Invert();

		m_bRecalcMatrix = FALSE;
	}

	// compute the x- and y-direction "heights"
	CVectorN<> vHx(GetLandmarkCount() + 3);
	CVectorN<> vHy(GetLandmarkCount() + 3);
	for (int nAtLandmark = 0; nAtLandmark < GetLandmarkCount(); nAtLandmark++)
	{
		vHx[nAtLandmark] = GetLandmark(1, nAtLandmark)[0] 
				- GetLandmark(0, nAtLandmark)[0];

		vHy[nAtLandmark] = GetLandmark(1, nAtLandmark)[1] 
				- GetLandmark(0, nAtLandmark)[1];
	}

	// compute the weight vectors
	m_vWx     = m_mL_inv * vHx;
	m_vWy     = m_mL_inv * vHy;

	// unset flag
	m_bRecalc = FALSE;

#ifdef _DEBUG
	// now check to ensure the offsets at each landmark is correct
	for (int nAtLandmark = 0; nAtLandmark < GetLandmarkCount(); nAtLandmark++)
	{
		const CVectorD<3>& vL0 = GetLandmark(0, nAtLandmark);
		const CVectorD<3>& vL1 = GetLandmark(1, nAtLandmark);

		CVectorD<3> vOffset;
		Eval(vL0, vOffset, 1.0);
		CVectorD<3> vL0_xform = vL0 + vOffset;

		ASSERT(vL0_xform.IsApproxEqual(vL1));
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// CTPSTransform::RecalcWeights
// 
// checks that the two transforms are inverses as per the landmarks
//////////////////////////////////////////////////////////////////////
BOOL 
	CTPSTransform::CheckInverse(CTPSTransform* pInverse)
{
	// now check to ensure the offsets at each landmark is correct
	for (int nAtLandmark = 0; nAtLandmark < GetLandmarkCount(); nAtLandmark++)
	{
		const CVectorD<3>& vL0 = GetLandmark(0, nAtLandmark);
		const CVectorD<3>& vL0_other = pInverse->GetLandmark(1, nAtLandmark);
		if (!vL0.IsApproxEqual(vL0_other))
			return FALSE;

		const CVectorD<3>& vL1 = GetLandmark(1, nAtLandmark);
		const CVectorD<3>& vL1_other = pInverse->GetLandmark(0, nAtLandmark);
		if (!vL1.IsApproxEqual(vL1_other))
			return FALSE;
	}

	return TRUE;
}