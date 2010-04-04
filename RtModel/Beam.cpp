// Copyright (C) 1999-2008 DG Lane
// $Id: Beam.cpp,v 1.28 2007-12-10 02:57:30 Derek Lane Exp $
#include "stdafx.h"

#include <itkEuler3DTransform.h>

// class declaration
#include <Beam.h>
#include <Plan.h>

namespace dH {

///////////////////////////////////////////////////////////////////////////////
Beam::Beam()
	// constructs a new dH::Beam object
	: m_pPlan(NULL)
		, m_CollimAngle(0.0)
		, m_GantryAngle(PI)
		, m_CouchAngle(0.0)
{
	// set up intensity map
	SetIntensityMap(dH::IntensityMapType::New());

	// set up accumulator for calculating beam dose from beamlets + weights
	SetAccumulator(dH::IntensityMapAccumulateImageFilter::New());
	GetAccumulator()->SetInput(GetIntensityMap());
	SetDoseMatrix(GetAccumulator()->GetOutput()); 

	// store reference to the beamlet group (= accumulator basis group)
	SetBeamletGroup(GetAccumulator()->GetBasisGroup());

	// the dose resampler transforms the dose back to the plan's geometry
	SetDoseResampler(dH::InPlaneResampleImageFilter::New());
	GetDoseResampler()->SetInput(GetDoseMatrix());

	// initialize the intensity map
	UpdateIntensityMapGeometry(4.0, MakeSize(9, 1));
}

//////////////////////////////////////////////////////////////////////
Beam::~Beam()
	// destroys the dH::Beam object
{
	SetPlan(NULL);
	SetAccumulator(NULL);
	SetBeamletGroup(NULL);
	SetIntensityMap(NULL);
	SetDoseMatrix(NULL);
	SetDoseResampler(NULL);
}

//////////////////////////////////////////////////////////////////////
void 
	Beam::SetGantryAngle(const REAL& gantryAngle)
	// sets the gantry angle value
{
	m_GantryAngle = gantryAngle;
	UpdateDoseBeamletGeometry();
}	

///////////////////////////////////////////////////////////////////////////////
int 
	Beam::GetBeamletCount()
{
	return GetBeamletGroup()->GetPixelContainer()->Size();
}	

///////////////////////////////////////////////////////////////////////////////
void 
	Beam::UpdateIntensityMapGeometry(REAL spacing, dH::IntensityMapType::SizeType size)
{
	if (size[0] % 2 != 1)
		throw new std::exception("Odd-numbered intensity map only supported");

	// set initial index for -n..n
	dH::IntensityMapType::IndexType index = {{ -(long) (size[0] - 1) / 2, 
																						 -(long) (size[1] - 1) / 2 }};
	GetIntensityMap()->SetRegions(dH::IntensityMapType::RegionType(index, size));
	GetIntensityMap()->Allocate();

	// set up spacing
	GetIntensityMap()->SetSpacing(dH::IntensityMapType::SpacingType(spacing));

	// make sure basis group conforms to intensity map
	//		NOTE: this will remove any existing beamlets, so be careful
	GetAccumulator()->InitBasisGroupGeometry();
}

//////////////////////////////////////////////////////////////////////////////
void 
	Beam::UpdateDoseBeamletGeometry()
	// set up the dose matrix from gantry angle and series volume
{
	// can't update until we have a plan
	if (!GetPlan())
		return;

	// now set up the rotated dose matrix
	ConformTo<VoxelReal,3>(GetPlan()->GetDoseMatrix(), GetDoseMatrix());

	Euler3DTransform<REAL>::Pointer rotXform = Euler3DTransform<REAL>::New();
	rotXform->SetRotation(0.0, 0.0, GetGantryAngle());
	
	// set the center of rotation
	VolumeReal::SizeType size = GetDoseMatrix()->GetBufferedRegion().GetSize();
	itk::ContinuousIndex<REAL,3> vCenterIndex = 
		MakeContinuousIndex<3>(size[0]/2, size[1]/2, size[2]/2);

	itk::Point<Real> vCenter;
	GetDoseMatrix()->TransformContinuousIndexToPhysicalPoint(vCenterIndex, vCenter);
	rotXform->SetCenter(vCenter);

	// calculate new dose matrix direction matrix
	itk::Matrix<Real> mRot = 
		rotXform->GetMatrix() * GetDoseMatrix()->GetDirection();
	GetDoseMatrix()->SetDirection(mRot);

	// set new dose matrix origin point
	itk::Point<Real> vOrigin = rotXform->TransformPoint(GetDoseMatrix()->GetOrigin());
	GetDoseMatrix()->SetOrigin(vOrigin);

	// set up the dose resampler
	GetDoseResampler()->SetOutputParametersFromImage(GetPlan()->GetDoseMatrix());
}

//////////////////////////////////////////////////////////////////////
void 
	Beam::SerializeExt(CArchive &ar, int nSchema)
	// loads/saves beam to archive
{
	// schema serialization
	nSchema = 1;
	SerializeValue(ar, nSchema);

	// serialize the object's name
	SerializeValue(ar, m_Name);

	// serialize basic geom values
	SerializeValue(ar, m_CollimAngle);
	SerializeValue(ar, m_GantryAngle);
	SerializeValue(ar, m_CouchAngle);

	SerializeValue(ar, m_Isocenter);

	// serialize the intensity map
	SerializeImage<VoxelReal>(ar, GetIntensityMap());

	// if loading, update the basis group geometry from the intensity map 
	//		(this will clear out any existing beamlets)
	if (ar.IsLoading())
	{
		GetAccumulator()->InitBasisGroupGeometry();
	}
	dH::BasisGroupType::Pointer pBasisGroup = GetBeamletGroup();

	ImageRegionIterator<dH::BasisGroupType> iter(pBasisGroup, pBasisGroup->GetBufferedRegion());
	for (; !iter.IsAtEnd(); ++iter)
	{
		if (ar.IsLoading())
		{
			// VolumeReal::Pointer pBeamlet = VolumeReal::New();
			SerializeVolume<VoxelReal>(ar, iter.Value()/*pBeamlet*/);
			// iter.Set(pBeamlet);
			// pBeamlet->Register();		// always need to register pointers to basis volumes in the basis group
		}
		else if (ar.IsStoring())
		{
			SerializeVolume<VoxelReal>(ar, iter.Get());
		}
	}
}

}