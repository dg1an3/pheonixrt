// Copyright (C) 2nd Messenger Systems
// $Id: Structure.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkScalarImageToListAdaptor.h>
#include <itkMeanCalculator.h>
#include <itkResampleImageFilter.h>

#include <Structure.h>
#include <Series.h>

BeginNamespace(dH)

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

}	// Structure::Structure

///////////////////////////////////////////////////////////////////////////////
Structure::~Structure()
	// destroys structure
{
	for (int nAt = 0; nAt < m_arrContours.GetSize(); nAt++)
	{
		delete m_arrContours[nAt];
	}

}	// Structure::~Structure

///////////////////////////////////////////////////////////////////////////////
int 
	Structure::GetContourCount() const
	// returns the number of contours in the mesh
{
	return (int) m_arrContours.GetSize();

}	// Structure::GetContourCount

///////////////////////////////////////////////////////////////////////////////
CPolygon *
	Structure::GetContour(int nIndex)
	// returns the contour at the given index
{
	return (CPolygon *) m_arrContours[nIndex];

}	// Structure::GetContour

///////////////////////////////////////////////////////////////////////////////
REAL 
	Structure::GetContourRefDist(int nIndex) const
	// returns the reference distance of the indicated contour
{
	return m_arrRefDist[nIndex];

}	// Structure::GetContourRefDist

///////////////////////////////////////////////////////////////////////////////
void 
	Structure::AddContour(CPolygon *pPoly, REAL refDist)
	// adds a new contour to the structure
{
	m_arrContours.Add(pPoly);
	m_arrRefDist.Add(refDist);

}	// Structure::AddContour


///////////////////////////////////////////////////////////////////////////////
const VolumeReal * 
	Structure::GetRegion(int nScale)
	// forms a new region and returns at requested scale
{
	if (nScale >= MAX_SCALES)
	{
		::AfxMessageBox(_T("Too big"));
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

}	// Structure::GetRegion

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

	// set the input to the pyramid and update
	m_pPyramid->SetInput(m_pRegion0);
	m_pPyramid->SetNumberOfLevels(MAX_SCALES);
	m_pPyramid->Update();

	// check the calculated levels
	for (int nAt = 0; nAt < MAX_SCALES; nAt++)
	{
		VolumeReal *pMask;
		if (nAt == 0)
			pMask = m_pRegion0;
		else
			pMask = m_pPyramid->GetOutput(MAX_SCALES-1 - nAt);
		TRACE("Level %i spacing = %lf, %lf, %lf\n", nAt, 
			pMask->GetSpacing()[0], pMask->GetSpacing()[1], pMask->GetSpacing()[2]);

		REAL maxVoxel = -1e+6;
		for (int nAt = 0; nAt < pMask->GetBufferedRegion().GetSize()[2]; nAt++)
		{
			REAL sliceMax = GetSliceMax(pMask, nAt);
			TRACE("Slice %i max %lf\n", nAt, sliceMax);
			maxVoxel = __max(sliceMax, maxVoxel);
		}

		// now adjust for the max (i.e. normalize to max 1)
		typedef itk::ImageRegionConstIterator< VolumeReal > ConstIteratorType;
		typedef itk::ImageRegionIterator< VolumeReal > IteratorType;

		VolumeReal::RegionType inputRegion;
		VolumeReal::RegionType::IndexType inputStart = pMask->GetBufferedRegion().GetIndex();
		VolumeReal::RegionType::SizeType size = pMask->GetBufferedRegion().GetSize();

		inputRegion.SetSize( size );
		inputRegion.SetIndex( inputStart );

		IteratorType voxelIt( pMask, inputRegion );
		for ( voxelIt.GoToBegin(); !voxelIt.IsAtEnd(); ++voxelIt)
		{
			/// TODO: figure out if this works (normalizing by the max voxel)
			/// voxelIt.Set(voxelIt.Get() / maxVoxel);
		}

		//typedef itk::Statistics::ScalarImageToListAdaptor< VolumeReal > VolumeRealSample;
		//VolumeRealSample::Pointer sample = VolumeRealSample::New();
		//sample->SetImage(pMask);

		//typedef itk::Statistics::MeanCalculator< VolumeRealSample > VolumeRealMean;
		//VolumeRealMean::Pointer mean = VolumeRealMean::New();
		//mean->SetInputSample(sample);
		//mean->Update();
		//VolumeRealMean::OutputType *out = mean->GetOutput();

	}
	m_bRecalcRegion = false;

}	// Structure::CalcRegion

///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
		Structure::GetConformRegion(itk::ImageBase<3> *pVolume)
		// forms / returns a resampled region for a given basis
		//	TODO: change this to only return the nearest conformant region 
		//		(so that caller is solely responsible for maintaining the resampled)
{
	VolumeReal::Pointer pConformRegion;

	// find if there is already a region
	for (int nAt = 0; nAt < (int) m_arrConformRegions.size(); nAt++)
	{

		VolumeReal * pTestConformRegion = m_arrConformRegions[nAt];
		if (IsApproxEqual<3>(pTestConformRegion->GetOrigin(), pVolume->GetOrigin())
				&& IsApproxEqual<3>(pTestConformRegion->GetSpacing(), pVolume->GetSpacing())
				&& pTestConformRegion->GetBufferedRegion() == pVolume->GetBufferedRegion())
		{
			pConformRegion = pTestConformRegion;
		}
	}

// #ifdef NEVER
	if (pConformRegion.IsNull())
	{
		// else form the resampled region
		pConformRegion = VolumeReal::New(); 

		// and add to cache map
		m_arrConformRegions.push_back(pConformRegion); 
	}

	ConformTo<VOXEL_REAL,3>(pVolume, pConformRegion);
	pConformRegion->FillBuffer(0.0);
// #endif

	// search for closest level in structure's pyramid
	int nLevel = -1;
	itk::Vector<REAL> vDosePixelSpacing = /*pConformRegion*/pVolume->GetSpacing();
	itk::Vector<REAL> vRegionPixelSpacing;
	do
	{
		nLevel++;
		vRegionPixelSpacing = GetRegion(nLevel)->GetSpacing();
	} while (
		(vRegionPixelSpacing[0] < vDosePixelSpacing[0] * 0.9/*- 1e-3*/
		|| vRegionPixelSpacing[1]  < vDosePixelSpacing[1] * 0.9/*- 1e-3*/)
		&& nLevel < MAX_SCALES);
		//vRegionPixelSpacing[0] /** 2.0 <*/ > vDosePixelSpacing[0]
		//|| vRegionPixelSpacing[1] /** 2.0 <*/ > vDosePixelSpacing[1]);
	
	CString strMsg;
	strMsg.Format(_T("DoseSpacing = %lf, %lf, %lf\nLevel Spacing = %lf, %lf, %lf"),
		vDosePixelSpacing[0], vDosePixelSpacing[1], vDosePixelSpacing[2],
		vRegionPixelSpacing[0], vRegionPixelSpacing[1], vRegionPixelSpacing[2]);
	//::AfxMessageBox(strMsg);

	// now resample to appropriate basis
	::Resample3D(GetRegion(nLevel), pConformRegion, TRUE);

#ifdef NEVER
	itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resampler = 
		itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
	resampler->SetInput(GetRegion(nLevel));
	VolumeReal::Pointer pPointToVolume = static_cast<VolumeReal*>(pVolume);
	resampler->SetOutputParametersFromImage(pPointToVolume);
	resampler->Update();

	pConformRegion = resampler->GetOutput();
	// and add to cache map
	m_arrConformRegions.push_back(pConformRegion); 
#endif

	return pConformRegion;

}	// Structure::GetConformRegion


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
		CArray<CPolygon *, CPolygon *> arrContours;
		for (int nAt = 0; nAt < GetContourCount(); nAt++)
		{
			REAL zPos = GetContourRefDist(nAt);
			if (IsApproxEqual(zPos, slicePos, pRegion->GetSpacing()[2] / 2.0))
			{
				arrContours.Add(GetContour(nAt));
			}
		}

		CreateRegion(arrContours, pRegion, nSlice);
	}

}	// Structure::ContoursToRegion


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


///////////////////////////////////////////////////////////////////////////////
void 
	Structure::SerializeExt(CArchive& ar, int nSchema)
{
	// schema for the plan object
	// UINT nSchema = ar.IsLoading() ? ar.GetObjectSchema() : STRUCTURE_SCHEMA;

	// CModelObject::Serialize(ar);
	if (ar.IsLoading())
	{
		ar >> m_Name;	
	}
	else
	{
		ar << m_Name;
	}

	//if (ar.IsLoading())
	//{
	//	ar >> m_pSeries;	
	//}
	//else
	//{
	//	ar << m_pSeries;
	//}

	m_arrContours.Serialize(ar);
	m_arrRefDist.Serialize(ar);

	if (nSchema >= 2)
	{
		if (ar.IsLoading())
		{
			ar >> m_Color;	
		}
		else
		{
			ar << m_Color;
		}
	}

	if (nSchema >= 3)
	{
		if (ar.IsLoading())
		{
			int nIntType;
			ar >> nIntType;
			m_Type = (StructType) nIntType;
		}
		else
		{
			ar << (int) m_Type;
		}
	}

	if (nSchema >= 4)
	{
		if (ar.IsLoading())
		{
			ar >> m_Visible;
		}
		else
		{
			ar << m_Visible;
		}
	}

	if (ar.IsLoading())
		m_bRecalcRegion = true;

}	// Structure::Serialize

EndNamespace(dH)