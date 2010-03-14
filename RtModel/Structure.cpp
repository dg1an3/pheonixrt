// Copyright (C) 2nd Messenger Systems
// $Id: Structure.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include <Structure.h>
#include <Series.h>

BeginNamespace(dH)

///////////////////////////////////////////////////////////////////////////////
Structure::Structure()
	: m_pSeries(NULL)
	, m_Visible(true)
	, m_Type(eNONE)
	, m_Priority(1)
	// constructs a structure
{
	m_pContoursToRegion = dH::ContoursToRegionFilter::New();
	m_pMultiMaskNegatedFilter = dH::MultiMaskNegatedImageFilter::New();
	m_pPyramid = PyramidType::New(); 

}	// Structure::Structure

///////////////////////////////////////////////////////////////////////////////
Structure::~Structure()
	// destroys structure
{
	// contours-to-region filter (also container for contours)
	m_pContoursToRegion = NULL;
	m_pMultiMaskNegatedFilter = NULL;
	m_pPyramid = NULL; 
	m_arrResamplers.clear();
}	

///////////////////////////////////////////////////////////////////////////////
int 
	Structure::GetContourCount() const
	// returns the number of contours in the mesh
{
	return m_pContoursToRegion->GetNumberOfInputs();
}	

///////////////////////////////////////////////////////////////////////////////
dH::ContourType *
	Structure::GetContourPoly(int nAt)
	// returns the contour at the given index
{
	return const_cast<dH::ContourType*>(m_pContoursToRegion->GetInput(nAt));
}

///////////////////////////////////////////////////////////////////////////////
void 
	Structure::AddContourPoly(dH::ContourType *pContour)
{
	m_pContoursToRegion->AddContour(pContour);
}


///////////////////////////////////////////////////////////////////////////////
void 
	Structure::SetPriority(const int& nPriority)
{
	m_Priority = nPriority;

	// need to update all structure pipelines for new priority
	GetSeries()->UpdateStructurePipelines();
}


///////////////////////////////////////////////////////////////////////////////
const VolumeReal * 
	Structure::GetRegion(int nScale)
	// forms a new region and returns at requested scale
{
	if (nScale == 0)
	{
		return m_pPyramid->GetInput();
	}

	return m_pPyramid->GetOutput(MAX_SCALES-nScale-1);

}	// Structure::GetRegion

///////////////////////////////////////////////////////////////////////////////
VolumeReal *
		Structure::GetConformRegion(itk::ImageBase<3> *pVolume)
		// forms / returns a resampled region for a given basis
{
	m_pPyramid->UpdateLargestPossibleRegion();

	// find if there is already a region
	for (int nAt = 0; nAt < (int) m_arrResamplers.size(); nAt++)
	{
		VolumeReal::Pointer pConformRegion = m_arrResamplers[nAt]->GetOutput();
		if (IsApproxEqual<3>(pConformRegion->GetOrigin(), pVolume->GetOrigin())
				&& IsApproxEqual<3>(pConformRegion->GetSpacing(), pVolume->GetSpacing())
				&& pConformRegion->GetBufferedRegion() == pVolume->GetBufferedRegion())
		{
			return pConformRegion;
		}
	}

	// create a new resampler
	ResamplerType::Pointer pResampler = ResamplerType::New();
	pResampler->SetOutputParametersFromImage(pVolume);
	m_arrResamplers.push_back(pResampler);

	// search for closest level in structure's pyramid
	VolumeReal::SpacingType outSpacing = pVolume->GetSpacing();
	for (int nLevel = 0; nLevel < MAX_SCALES; nLevel++)
	{
		VolumeReal::SpacingType inSpacing = GetRegion(nLevel)->GetSpacing();
		if (inSpacing[0] >= outSpacing[0] * 0.9
			&& inSpacing[1] >= outSpacing[1] * 0.9)
		{
			pResampler->SetInput(GetRegion(nLevel));
			break;
		}
	}
	ASSERT(pResampler->GetInput() != NULL);
	pResampler->UpdateLargestPossibleRegion();

	return pResampler->GetOutput();
}


///////////////////////////////////////////////////////////////////////////////
void 
	Structure::UpdatePipeline()
{
	if (GetSeries()->GetDensity())
	{
		// set up the output geometry for ContoursToRegionFilter
		m_pContoursToRegion->SetOutputParametersFromImage(GetSeries()->GetDensity());
	}

	// set up mask filter input
	m_pMultiMaskNegatedFilter->SetInput(0, m_pContoursToRegion->GetOutput());

	// set up mask filter exclude maps
	for (int nAt = 0; nAt < GetSeries()->GetStructureCount(); nAt++)
	{
		Structure *pOtherStruct = GetSeries()->GetStructureAt(nAt);
		m_pMultiMaskNegatedFilter->SetInput(nAt+1, pOtherStruct->GetRegion(0));
		m_pMultiMaskNegatedFilter->SetMaskEnabled(nAt+1, pOtherStruct->GetPriority() < GetPriority());
	}

	// set up the pyramid
	m_pPyramid->SetInput(m_pContoursToRegion->GetOutput());
	m_pPyramid->SetNumberOfLevels(MAX_SCALES);
}


///////////////////////////////////////////////////////////////////////////////
void 
	Structure::SerializeExt(CArchive& ar, int nSchema)
{
	// schema serialization
	nSchema = 1;
	SerializeValue(ar, nSchema);

	SerializeValue(ar, m_Name);

	SerializeValue(ar, m_Color);

	int nIntType = (int) m_Type;
	SerializeValue(ar, nIntType);
	if (ar.IsLoading())
		m_Type = (StructType) nIntType;

	SerializeValue(ar, m_Visible);

	if (ar.IsLoading())
	{
		// delete existing contours by re-creating filter
		m_pContoursToRegion = dH::ContoursToRegionFilter::New();

		int nContourCount = 0;
		SerializeValue(ar, nContourCount);
		for (int nAt = 0; nAt < nContourCount; nAt++)
		{
			ContourType::Pointer pContour = ContourType::New();
			SerializePolyLine<3>(ar, pContour);
			AddContourPoly(pContour);
		}
	}	
	else if (ar.IsStoring())
	{
		int nContourCount = GetContourCount();
		SerializeValue(ar, nContourCount);
		for (int nAt = 0; nAt < nContourCount; nAt++)
		{
			SerializePolyLine<3>(ar, GetContourPoly(nAt));
		}
	}
}

EndNamespace(dH)