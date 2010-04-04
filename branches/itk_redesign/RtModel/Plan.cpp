// Copyright (C) DGLane
// $Id$
#include "stdafx.h"

#include "Plan.h"

#include <EnergyDepKernel.h>

namespace dH {

///////////////////////////////////////////////////////////////////////////////
Plan::Plan()
	: m_DoseResolution(2.0)
{
	SetAccumulator(dH::IntensityMapAccumulateImageFilter::New());
	GetAccumulator()->SetInput(0, dH::IntensityMapType::New());

	SetDoseMatrix(GetAccumulator()->GetOutput()); 
}

///////////////////////////////////////////////////////////////////////////////
Plan::~Plan()
{
}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::SetSeries(dH::Series *pSeries)
{
	// store the series pointer
	m_pSeries = pSeries;

	// trigger calc of dose matrix basis
	UpdateDoseMatrixGeometry();
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::GetBeamCount() const
{
	return m_arrBeams.size();
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
dH::Beam * 
	Plan::GetBeamAt(int nAt)
{
	return m_arrBeams[nAt];
}

///////////////////////////////////////////////////////////////////////////////
int 
	Plan::AddBeam(dH::Beam *pBeam)
{
	m_arrBeams.push_back(pBeam);
	pBeam->SetPlan(this);

	UpdateBeamAccumulator();

	return m_arrBeams.size()-1;
}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::CreateEquidistantBeams(int nBeamCount, const Vector<REAL>& vIsocenter)
{
	// delete existing beams
	m_arrBeams.clear();
	UpdateBeamAccumulator();

	// create the beams
	for (int nAt = nBeamCount-1; nAt >= 0; nAt--)
	{
		// create the new beam
		dH::Beam *pBeam = new dH::Beam();

		// calculate gantry for the beam
		double gantry;
		gantry = 90.0 + (double) nAt * 360.0 / (double) nBeamCount;
		pBeam->SetGantryAngle(gantry * PI / 180.0);

		// calculate iso position for the beam
		pBeam->SetIsocenter(vIsocenter);

		AddBeam(pBeam);
	}
}

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::UpdateBeamAccumulator()
	// called to update Accumulator geometry
{
	// create the uniform intensity map
	dH::IntensityMapType::Pointer pBeamWeightMap = 
		const_cast<dH::IntensityMapType *>(GetAccumulator()->GetInput(0));
	pBeamWeightMap->SetRegions(MakeSize(GetBeamCount(), 1));
	pBeamWeightMap->Allocate();
	pBeamWeightMap->FillBuffer(1.0);
	pBeamWeightMap->Modified();

	// set up the basis group to point to the individual beam doses
	GetAccumulator()->InitBasisGroupGeometry();
	dH::BasisGroupType * pBasisGroup = GetAccumulator()->GetBasisGroup();

	ImageRegionIterator<dH::BasisGroupType> iter(pBasisGroup, pBasisGroup->GetBufferedRegion());
	for (; !iter.IsAtEnd(); ++iter)
	{
		dH::Beam *pBeam = GetBeamAt(iter.GetIndex()[0]);
		pBeam->UpdateDoseBeamletGeometry();

		VolumeReal * pOutput = pBeam->GetDoseResampler()->GetOutput();
		iter.Set(pOutput);
		// TODO: figure out how this affects the Accumulator 
		//		(because we are replacing the accumulator's own basis volumes with our own)
		// pOutput->Register();		// need to register, because the basis group will unregister
	}

	// make sure the beam dose's are officially inputs to the accumulator
	GetAccumulator()->SetBasisGroupAsInput(true);
}

///////////////////////////////////////////////////////////////////////////////
void
	Plan::SetDoseResolution(const REAL& res)
	// sets shape for dose matrix
{
	m_DoseResolution = res;

	UpdateDoseMatrixGeometry();
}

///////////////////////////////////////////////////////////////////////////////
void
	Plan::UpdateDoseMatrixGeometry()
	// sets shape for dose matrix
{
	// accessor to planning volume
	const VolumeReal *pVolume = GetSeries()->GetDensity();
	const VolumeReal::SpacingType& spacing = pVolume->GetSpacing();

	// compute the adjusted size
	VolumeReal::SizeType size = pVolume->GetBufferedRegion().GetSize();
	for (int nN = 0; nN < 3; nN++)
		size[nN] = Round<int>((Real) size[nN] * spacing[nN] / m_DoseResolution);

	// set dimensions
	GetDoseMatrix()->SetRegions(size);
	GetDoseMatrix()->Allocate();

	GetDoseMatrix()->SetOrigin(pVolume->GetOrigin());
	GetDoseMatrix()->SetDirection(pVolume->GetDirection());
	GetDoseMatrix()->SetSpacing(
		MakeVector<3>(m_DoseResolution, m_DoseResolution, m_DoseResolution));

	// now update individual beam dose geometries
	for (int nAt = 0; nAt < GetBeamCount(); nAt++)
		GetBeamAt(nAt)->UpdateDoseBeamletGeometry();
}	

///////////////////////////////////////////////////////////////////////////////
void 
	Plan::SerializeExt(CArchive& ar, int nSchema)
{
	nSchema = 1;
	SerializeValue(ar, nSchema);

	if (ar.IsLoading())
	{
		// delete existing structures
		m_arrBeams.clear();

		int nBeamCount = 0;
		SerializeValue(ar, nBeamCount);
		for (int nBeam = 0; nBeam < nBeamCount; nBeam++)
		{
			dH::Beam::Pointer pBeam = dH::Beam::New();
			pBeam->SerializeExt(ar, -1);
			AddBeam(pBeam);
		}
	}
	else if (ar.IsStoring())
	{
		int nBeamCount = GetBeamCount();
		SerializeValue(ar, nBeamCount);
		for (int nBeam = 0; nBeam < nBeamCount; nBeam++)
		{
			GetBeamAt(nBeam)->SerializeExt(ar, -1);
		}
	}

	// use temp CVolume for serialization
	SerializeVolume<VOXEL_REAL>(ar, GetDoseMatrix());

	// set up the accumulator, if needed
	if (ar.IsLoading())
	{
		m_DoseResolution = GetDoseMatrix()->GetSpacing()[0];
		UpdateBeamAccumulator();
	}
}

}