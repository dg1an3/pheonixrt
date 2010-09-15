// Copyright (C) 2nd Messenger Systems
// $Id: Plan.cpp 640 2009-06-13 05:06:50Z dglane001 $
#include "stdafx.h"

#include "Plan.h"

#include <EnergyDepKernel.h>

namespace dH 
{

///////////////////////////////////////////////////////////////////////////////
Plan::Plan()
	: m_pSeries(NULL)
	, m_DoseResolution(4.0) // 
		// 2.0)
{
	m_pKernel = new CEnergyDepKernel(6.0); // 
		// 15.0);

	m_pMassDensity = VolumeReal::New();

	m_pDose = VolumeReal::New();
	m_pBeamDoseRot = VolumeReal::New();
	m_pTempBuffer = VolumeReal::New();

}

///////////////////////////////////////////////////////////////////////////////
Plan::~Plan()
{
	// delete the histograms
	POSITION pos = m_mapHistograms.GetStartPosition();
	while (NULL != pos)
	{
		CString strName;
		CHistogram *pHistogram = NULL;
		m_mapHistograms.GetNextAssoc(pos, strName, pHistogram); 

		delete pHistogram;
	}

	// delete the kernel
	delete m_pKernel;
}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::SetSeries(dH::Series *pSeries)
{
	// store the series pointer
	m_pSeries = pSeries;

	// trigger calc of dose matrix basis
	SetDoseResolution(GetDoseResolution());
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::GetBeamCount() const
{
	return (int) m_arrBeams.size();
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::GetTotalBeamletCount()
{
	int nBeamlets = 0;

	for (int nAtBeam = 0; nAtBeam < GetBeamCount(); nAtBeam++)
	{
		nBeamlets += GetBeamAt(nAtBeam)->GetBeamletCount();
	}

	return nBeamlets;
}

///////////////////////////////////////////////////////////////////////////////
CBeam * 
	Plan::GetBeamAt(int nAt)
{
	return m_arrBeams.at(nAt);
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::AddBeam(CBeam *pBeam)
{
	m_arrBeams.push_back(pBeam);
	int nIndex = (int) m_arrBeams.size();
	pBeam->SetPlan(this);

	// a change has occurred, so fire
	// GetChangeEvent().Fire();

	return nIndex;
}

///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
	Plan::GetDoseMatrix()
{
	// total the dose for all beams
	if (GetBeamCount() > 0)
	{
		// clear the dose matrix
		m_pDose->FillBuffer(0.0);

		for (int nAt = 0; nAt < GetBeamCount(); nAt++)
		{
			ConformTo<VOXEL_REAL,3>(m_pDose, m_pBeamDoseRot);
			m_pBeamDoseRot->FillBuffer(0.0); 

			VolumeReal *pBeamDose = GetBeamAt(nAt)->GetDoseMatrix();
			// Resample(pBeamDose, m_pBeamDoseRot, TRUE);
			// Resample3D(pBeamDose, m_pBeamDoseRot, TRUE);
			itk::ResampleImageFilter<VolumeReal, VolumeReal>::Pointer resampler = 
				itk::ResampleImageFilter<VolumeReal, VolumeReal>::New();
			resampler->SetInput(pBeamDose);

			typedef itk::AffineTransform<REAL, 3> TransformType;
			TransformType::Pointer transform = TransformType::New();
			transform->SetIdentity();
			resampler->SetTransform(transform);

			typedef itk::LinearInterpolateImageFunction<VolumeReal, REAL> InterpolatorType;
			InterpolatorType::Pointer interpolator = InterpolatorType::New();
			resampler->SetInterpolator( interpolator );

			VolumeReal::Pointer pPointToVolume = static_cast<VolumeReal*>(m_pBeamDoseRot);
			resampler->SetOutputParametersFromImage(m_pBeamDoseRot);
			resampler->Update();
			CopyImage<VOXEL_REAL, 3>(resampler->GetOutput(), m_pBeamDoseRot);

			// add this beam's dose matrix to the total
			ConformTo<VOXEL_REAL,3>(m_pDose, m_pTempBuffer);
			// Accumulate<VOXEL_REAL>(m_pBeamDoseRot, 
			//	/* beam weight = */ 1.0, m_pDose, m_pTempBuffer);
			Accumulate3D<VOXEL_REAL>(m_pBeamDoseRot, 
				/* beam weight = */ 1.0, m_pDose, m_pTempBuffer);
		}
	}

	return m_pDose;

}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::UpdateAllHisto()
{
#ifdef USE_RTOPT
	// first recalc dose matrix
	GetDoseMatrix();

	// now iterate over histo's
	POSITION pos = m_mapHistograms.GetStartPosition();
	CString strName;
	CHistogram *pHisto;
	while (pos != NULL)
	{
		m_mapHistograms.GetNextAssoc(pos, strName, pHisto);
		pHisto->OnVolumeChange(); // NULL, NULL);
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
void
	Plan::SetDoseResolution(const REAL& res)
	// sets shape for dose matrix
{
	m_DoseResolution = res;

	// accessor to planning volume
	const VolumeReal *pVolume = GetSeries()->GetDensity();
	const itk::Vector<REAL> vVolSpacing = pVolume->GetSpacing();

	// compute height / width / depth
	int nHeight = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[1] 
			* vVolSpacing[1] / m_DoseResolution);
	int nWidth = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[0] 
			* vVolSpacing[0] / m_DoseResolution);
	int nDepth = 
		Round<int>(pVolume->GetBufferedRegion().GetSize()[2] 
			* vVolSpacing[2] / m_DoseResolution);

	// set dimensions
	m_pDose->SetRegions(MakeSize(nWidth, nHeight, nDepth));
	m_pDose->Allocate();

	m_pDose->SetOrigin(pVolume->GetOrigin());
	m_pDose->SetDirection(pVolume->GetDirection());
	m_pDose->SetSpacing(
		MakeVector<3>(m_DoseResolution, m_DoseResolution, m_DoseResolution));

}


///////////////////////////////////////////////////////////////////////////////
CHistogram *
	Plan::GetHistogram(dH::Structure *pStructure, bool bCreate)
{
	CHistogram *pHisto = NULL;
#ifdef USE_RTOPT
	if (!m_mapHistograms.Lookup(CString(pStructure->GetName().c_str()), pHisto))
	{
		if (bCreate)
		{
			pHisto = new CHistogram();

			pHisto->SetBinning((REAL) 0.0, (REAL) 0.01 /* 0.02 */, GBINS_BUFFER);
			pHisto->SetVolume(GetDoseMatrix());

			// resample region, if needed
			VolumeReal *pResampRegion = pStructure->GetConformRegion(GetDoseMatrix());
			pHisto->SetRegion(pResampRegion);

			// calculate slice number for the isocenter
			REAL sliceZ = GetBeamAt(0)->GetIsocenter()[2];
			int nSlice = Round<int>((sliceZ - pResampRegion->GetOrigin()[2]) / pResampRegion->GetSpacing()[2]);
			pHisto->SetSlice(nSlice);

			// add to map
			m_mapHistograms[CString(pStructure->GetName().c_str())] = pHisto;
		}
	}

	if (pHisto != NULL)
	{
		// resample region, if needed (this is to always create the exclusion region
		VolumeReal *pResampRegion = pStructure->GetConformRegion(GetDoseMatrix());
		pHisto->SetRegion(pResampRegion);
	}
#endif

	return pHisto;

}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::RemoveHistogram(dH::Structure *pStructure)
{
#ifdef USE_RTOPT
	CHistogram *pHisto = NULL;
	if (m_mapHistograms.Lookup(CString(pStructure->GetName().c_str()), pHisto))
	{
		m_mapHistograms.RemoveKey(CString(pStructure->GetName().c_str()));
		delete pHisto;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
VolumeReal * 
	Plan::GetMassDensity()
	// used to format the mass density array, conformant to dose matrix
{
	// fix mass density
	ConformTo<VOXEL_REAL,3>(GetSeries()->GetDensity(), m_pMassDensity);
	m_pMassDensity->FillBuffer(0.0); 

	// lookup values
	VOXEL_REAL *pCTVoxels = GetSeries()->GetDensity()->GetBufferPointer(); 
	VOXEL_REAL *pMDVoxels = m_pMassDensity->GetBufferPointer(); 
	int nVoxels = m_pMassDensity->GetBufferedRegion().GetNumberOfPixels();
	for (int nAtVoxel = 0; nAtVoxel < nVoxels; nAtVoxel++)
	{
		if (pCTVoxels[nAtVoxel] < 0.0)
		{
			pMDVoxels[nAtVoxel] =
				(VOXEL_REAL)(0.0 + 1.0 * (pCTVoxels[nAtVoxel] - -1024.0) / 1024.0);
		}
		else if (pCTVoxels[nAtVoxel] < 1024.0)
		{
			pMDVoxels[nAtVoxel] = 
				(VOXEL_REAL)(1.0 + 0.0/*0.5*/ * pCTVoxels[nAtVoxel] / 1024.0);
		}
		else
		{
			pMDVoxels[nAtVoxel] = 1.0/*1.5*/;
		}
	}

	return m_pMassDensity;

}

}	// namespace dH
