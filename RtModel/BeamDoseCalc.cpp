// Copyright (C) 2008 DGLane
// $Id$
#include "stdafx.h"
#include "BeamDoseCalc.h"

#include <Beam.h>
#include <Plan.h>

#include <EnergyDepKernel.h>
#include <TermaCalculator.h>
#include <SphereConvolve.h>

using namespace itk;

namespace dH {

///////////////////////////////////////////////////////////////////////////////
BeamDoseCalc::BeamDoseCalc()
{
	SetHoundsfieldToMassDensityFilter(HoundsfieldToMassDensityFilter::New());
	GetHoundsfieldToMassDensityFilter()->SetInPlace(false);

	SetDensityResampler(dH::InPlaneResampleImageFilter::New());
	GetDensityResampler()->SetInput(GetHoundsfieldToMassDensityFilter()->GetOutput());
}

///////////////////////////////////////////////////////////////////////////////
BeamDoseCalc::~BeamDoseCalc()
{
}

///////////////////////////////////////////////////////////////////////////////
void
	BeamDoseCalc::SetBeam(dH::Beam *pBeam)
{
	m_pBeam = pBeam;

	dH::Series::Pointer pSeries = GetBeam()->GetPlan()->GetSeries();
	GetHoundsfieldToMassDensityFilter()->SetInput(pSeries->GetDensity());

	GetDensityResampler()->SetOutputParametersFromImage(GetBeam()->GetDoseMatrix());

	// clear out existing beamlets in beam
	GetBeam()->GetAccumulator()->InitBasisGroupGeometry();
}

///////////////////////////////////////////////////////////////////////////////////////
bool BeamDoseCalc::CalcNextBeamlet(dH::BasisGroupType::IndexType& currentIndex)
{
	// determine next beamlet
	dH::BasisGroupType::Pointer pBasisGroup = GetBeam()->GetAccumulator()->GetBasisGroup();
	ImageRegionIterator<dH::BasisGroupType> iter(pBasisGroup, pBasisGroup->GetBufferedRegion());
	while (iter.Value()->GetPixelContainer()->Size() != 0)
	{
		++iter;
		if (iter.IsAtEnd())
			return false;
	}

	// store the current beamlet index
	currentIndex = iter.GetIndex();

	VolumeReal::Pointer pDensity = GetDensityResampler()->GetOutput();

	dH::TermaCalculator::Pointer pTermaCalc = dH::TermaCalculator::New();
	pTermaCalc->SetInput(pDensity);
	pTermaCalc->SetIsocenter(GetBeam()->GetIsocenter());
	pTermaCalc->SetSAD(1000.0);
	pTermaCalc->Set_mu(GetKernel()->Get_mu());

	// calculate the beamlet raytrace min and max
	Vector<REAL,2> halfPixel;
	halfPixel.Fill(0.5);

	ContinuousIndex<REAL, 2> indexMin(iter.GetIndex());
	indexMin -= halfPixel;
	Point<REAL, 2> rayMin;
	pBasisGroup->TransformContinuousIndexToPhysicalPoint<REAL>(indexMin, rayMin);
	pTermaCalc->SetRayTraceMin(rayMin);

	ContinuousIndex<REAL, 2> indexMax(iter.GetIndex());
	indexMax += halfPixel;	
	Point<REAL, 2> rayMax;
	pBasisGroup->TransformContinuousIndexToPhysicalPoint<REAL>(indexMax, rayMax);
	pTermaCalc->SetRayTraceMax(rayMax);

	// compute terma
	VolumeReal::Pointer pTerma = pTermaCalc->GetOutput();

	dH::SphereConvolve::Pointer pConvolver = dH::SphereConvolve::New();
	pConvolver->SetInput(0, pDensity);
	pConvolver->SetInput(1, pTerma);
	pConvolver->SetKernel(GetKernel());

	// calc isocenter slice
	VolumeReal::PointType origin = GetDensityResampler()->GetOutputOrigin(); 
	VolumeReal::SpacingType spacing = GetDensityResampler()->GetOutputSpacing();
	int nSlice = Round<int>((GetBeam()->GetIsocenter()[2] - origin[2]) / spacing[2]);
	pConvolver->SetSlice(nSlice);

	pConvolver->UpdateLargestPossibleRegion();
	VolumeReal::Pointer pEnergy = pConvolver->GetOutput();

	// set the beamlet
	CopyImage<VoxelReal,3>(pEnergy, iter.Value());

	return true;
}

}