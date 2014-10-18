// Copyright (C) 2nd Messenger Systems
// $Id: Structure.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkResampleImageFilter.h>

#include <Structure.h>
#include <Series.h>

namespace dH
{

///////////////////////////////////////////////////////////////////////////////
Structure::Structure()
	: m_pSeries(NULL)
	, m_Visible(true)
	, m_Type(eNONE)
	, m_Priority(1)
	, m_bRecalcRegion(true)
	// constructs a structure
{
	m_pRegion0 = VolumeReal::New();
	m_pPyramid = PyramidType::New(); 

	m_pPyramid->SetInput(m_pRegion0);
	m_pPyramid->SetNumberOfLevels(MAX_SCALES);
}

///////////////////////////////////////////////////////////////////////////////
Structure::~Structure()
	// destroys structure
{
	//for (int nAt = 0; nAt < m_arrContours.size(); nAt++)
	//{
	//	delete m_arrContours[nAt];
	//}

}

///////////////////////////////////////////////////////////////////////////////
int 
	Structure::GetContourCount() const
	// returns the number of contours in the mesh
{
	return (int) m_arrContours.size();
}

///////////////////////////////////////////////////////////////////////////////
Structure::PolygonType *
	Structure::GetContour(int nIndex)
	// returns the contour at the given index
{
	ContourMapType::iterator iterAt = m_arrContours.begin();
	for (int nAt = 0; nAt < nIndex; nAt++, iterAt++);
	return iterAt->second.GetPointer();
}

///////////////////////////////////////////////////////////////////////////////
REAL 
	Structure::GetContourRefDist(int nIndex)
	// returns the reference distance of the indicated contour
{
	ContourMapType::iterator iterAt = m_arrContours.begin();
	for (int nAt = 0; nAt < nIndex; nAt++, iterAt++);
	return iterAt->first;
}

///////////////////////////////////////////////////////////////////////////////
void 
Structure::AddContour(Structure::PolygonType::Pointer pPoly, REAL refDist)
	// adds a new contour to the structure
{
	m_arrContours.insert(std::make_pair(refDist, pPoly));
}


///////////////////////////////////////////////////////////////////////////////
const VolumeReal * 
	Structure::GetRegion(int nScale)
	// forms a new region and returns at requested scale
{
	if (nScale >= MAX_SCALES)
	{
		nScale = MAX_SCALES-1;
	}

	if (m_bRecalcRegion)
	{
		CalcRegion();
	}

	if (nScale == 0)
	{
		return m_pPyramid->GetInput();
	}

	// return m_pPyramid->GetOutput(nScale-1);
	return m_pPyramid->GetOutput(MAX_SCALES-1 - nScale);

}

///////////////////////////////////////////////////////////////////////////////
void
	Structure::CalcRegion()
	// forms the base level region
{
	// set size of region
	ConformTo<VOXEL_REAL,3>(GetSeries()->GetDensity(), m_pRegion0);
	ContoursToRegion(m_pRegion0);

	// now exclude
	for (int nAt = 0; nAt < GetSeries()->GetStructureCount(); nAt++)
	{
		Structure *pOtherStruct = GetSeries()->GetStructureAt(nAt);
		if (GetPriority() > pOtherStruct->GetPriority())
		{
			const VolumeReal *pExclRegion = pOtherStruct->GetRegion(0);
			ConstVolumeRealIterator iterExcl(pExclRegion, pExclRegion->GetBufferedRegion());
			VolumeRealIterator iter(m_pRegion0, m_pRegion0->GetBufferedRegion());
			for ( iter.GoToBegin(), iterExcl.GoToBegin(); !iterExcl.IsAtEnd();
				++iter, ++iterExcl)
			{
				if (iterExcl.Get() > 0.0)
					iter.Set(0.0);
			}
		}
	}

	// update the pyramid
	m_pPyramid->Update();

	m_bRecalcRegion = false;

}

///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
		Structure::GetConformRegion(itk::ImageBase<3> *pVolume)
		// forms / returns a resampled region for a given basis
{
	// search for closest level in structure's pyramid
	int nLevel = -1;
	itk::Vector<REAL> vDosePixelSpacing = pVolume->GetSpacing();
	itk::Vector<REAL> vRegionPixelSpacing;
	do
	{
		nLevel++;
		vRegionPixelSpacing = GetRegion(nLevel)->GetSpacing();
	} while (
		(vRegionPixelSpacing[0] < vDosePixelSpacing[0] * 0.9
		|| vRegionPixelSpacing[1]  < vDosePixelSpacing[1] * 0.9)
		&& nLevel < MAX_SCALES);

	// now resample to the requested resolution

	itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resampler = 
		itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
	resampler->SetInput(GetRegion(nLevel));

	typedef itk::AffineTransform<REAL, 3> TransformType;
	TransformType::Pointer transform = TransformType::New();
	transform->SetIdentity();
	resampler->SetTransform(transform);

	typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	resampler->SetInterpolator( interpolator );

	VolumeReal::Pointer pPointToVolume = static_cast<VolumeReal*>(pVolume);
	resampler->SetOutputParametersFromImage(pPointToVolume);
	resampler->Update();
	m_arrResamplers.push_back(resampler);
	return resampler->GetOutput();
}

typedef itk::PolygonSpatialObject<2> PolygonType;

///////////////////////////////////////////////////////////////////////////////
// CreateRegion
// 
// Creates a region (bit mask) from a polygon
///////////////////////////////////////////////////////////////////////////////
template<class VOXEL_TYPE>
void CreateRegionForPolygonSpatialObject(const CArray<PolygonType *, PolygonType *>& arrPolygons,
				  itk::Image<VOXEL_TYPE,3> *pRegion, int nSlice)
{
	CDC dc;
	BOOL bRes = dc.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bRes = bitmap.CreateBitmap(pRegion->GetBufferedRegion().GetSize()[0], 
		pRegion->GetBufferedRegion().GetSize()[1], 1, 1, NULL);

	CBitmap *pOldBitmap = (CBitmap *) dc.SelectObject(&bitmap);
	dc.SelectStockObject(WHITE_PEN);
	dc.SelectStockObject(WHITE_BRUSH);

	itk::Point<REAL,3> vOrigin = pRegion->GetOrigin();
	itk::Vector<REAL,3> vSpacing = pRegion->GetSpacing();
	for (int nAtPoly = 0; nAtPoly < arrPolygons.GetSize(); nAtPoly++)
	{
		// static 
			CArray<CPoint, CPoint&> arrPoints;
		arrPoints.SetSize(arrPolygons[nAtPoly]->GetNumberOfPoints/*GetVertexCount*/());
		for (int nAt = 0; nAt < arrPolygons[nAtPoly]->GetNumberOfPoints/*GetVertexCount*/(); nAt++)
		{
			itk::SpatialObjectPoint<2> vVert = *(arrPolygons[nAtPoly]->GetPoint/*GetVertexAt*/(nAt));
			//vVert[0] = (vVert[0] - vOrigin[0]) / vSpacing[0];
			//vVert[1] = (vVert[1] - vOrigin[1]) / vSpacing[1];

			arrPoints[nAt].x = (vVert.GetPosition()[0] - vOrigin[0]) / vSpacing[0];
			arrPoints[nAt].y = (vVert.GetPosition()[1] - vOrigin[1]) / vSpacing[1];
		}
		dc.Polygon(arrPoints.GetData(), arrPolygons[nAtPoly]->GetNumberOfPoints/*GetVertexCount*/());
	}

	// finished with DC
	dc.SelectObject(pOldBitmap);
	dc.DeleteDC();

	// now get the bitmap descriptor (for scan width
	BITMAP bm;
	bitmap.GetBitmap(&bm);

	// resize the buffer
	// static 
		CArray<BYTE, BYTE> arrBuffer;
	arrBuffer.SetSize(pRegion->GetBufferedRegion().GetSize()[1] * bm.bmWidthBytes);
	int nByteCount = bitmap.GetBitmapBits((DWORD) arrBuffer.GetSize(), arrBuffer.GetData());

	// now populate region
	// DON'T CLEAR REGION HERE -- this is called multiple times
	// pRegion->FillBuffer(0.0); 

	int nStride = pRegion->GetBufferedRegion().GetSize()[0];
	int nStrideZ = pRegion->GetBufferedRegion().GetSize()[1] * nStride;
	// now populate the region
	for (int nY = 0; nY < pRegion->GetBufferedRegion().GetSize()[1]; nY++)
	{
		for (int nX = 0; nX < pRegion->GetBufferedRegion().GetSize()[0]; nX++)
		{
			if ((arrBuffer[nY * bm.bmWidthBytes + nX / 8] >> (7 - nX % 8)) & 0x01)
			{
				// (*pRegion)[0][nY][nX] 
				pRegion->GetBufferPointer()[nSlice * nStrideZ + nY * nStride + nX]
					= (VOXEL_TYPE) 1.0;
			}
		}
	}

	// done with bitmap
	bitmap.DeleteObject();

}	

///////////////////////////////////////////////////////////////////////////////
void 
	Structure::ContoursToRegion(VolumeReal *pRegion)
		// converts the contours to a region
{
	// clear region before drawing any contours
	pRegion->FillBuffer(0.0); 

	for (int nSlice = 0; nSlice < pRegion->GetBufferedRegion().GetSize()[2]; nSlice++)
	{
		REAL slicePos = pRegion->GetOrigin()[2] + pRegion->GetSpacing()[2] * nSlice;
		CArray<PolygonType *, PolygonType *> arrContours;
		for (int nAt = 0; nAt < GetContourCount(); nAt++)
		{
			REAL zPos = GetContourRefDist(nAt);
			if (IsApproxEqual(zPos, slicePos, pRegion->GetSpacing()[2] / 2.0))
			{
				arrContours.Add(GetContour(nAt));
			}
		}
		CreateRegionForPolygonSpatialObject(arrContours, pRegion, nSlice);
	}
}

///////////////////////////////////////////////////////////////////////////////
void 
	Structure::SetPriority(const int& nPriority)
{
	m_Priority = nPriority;

	// set flags to recalc region
	for (int nAt = 0; nAt < GetSeries()->GetStructureCount(); nAt++)
	{
		GetSeries()->GetStructureAt(nAt)->m_bRecalcRegion = true;
	}
}


} // namespace dH