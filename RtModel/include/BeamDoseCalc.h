// Copyright (C) 2008 DGLane
// $Id$
#pragma once

#include <itkUnaryFunctorImageFilter.h>
using namespace itk;

#include <ItkUtils.h>

#include <InPlaneResampleImageFilter.h>

#include <Beam.h>
#include <EnergyDepKernel.h>

namespace dH {

//////////////////////////////////////////////////////////////////////////////////
class BeamDoseCalc : public ProcessObject
{
public:
	// constructor / destructor
	BeamDoseCalc(); 
	virtual ~BeamDoseCalc();

	// itk typedefs
	typedef BeamDoseCalc Self;
	typedef ProcessObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// my beam
	DeclareMemberSPtrGet(Beam, dH::Beam);
	void SetBeam(dH::Beam *pBeam);

	// reference to the source
	DeclareMemberSPtr(Kernel, EnergyDepKernel);

	// triggers calculation of beam's pencil beams
	bool CalcNextBeamlet(dH::BasisGroupType::IndexType& currentIndex);

	///////////////////////////////////////////////////////////////////////////////
	class HoundsfieldToMassDensityTransform
	{
	public:
		HoundsfieldToMassDensityTransform() {	}

		// computes the mass density for a given HU pixel value
		inline VoxelReal operator()( const VoxelReal& x )
		{
			const VoxelReal AirHoundsfield = -1024.0;
			const VoxelReal AirDensity = 0.0;

			const VoxelReal WaterHoundsfield = 0.0;
			const VoxelReal WaterDensity = 1.0;

			const VoxelReal BoneHoundsfield = 1024.0;
			const VoxelReal BoneDensity = 1.5;

			if (x < WaterHoundsfield)
			{
				VoxelReal fraction = (x - AirHoundsfield) / (WaterHoundsfield - AirHoundsfield);
				return AirDensity + (WaterDensity - AirDensity) * fraction;
			}
			else if (x < BoneHoundsfield)
			{
				VoxelReal fraction = (x - WaterHoundsfield) / (BoneHoundsfield - WaterHoundsfield);
				return WaterDensity + (BoneDensity - WaterDensity) * fraction;
			}

			return BoneDensity;
		}
	}; 

	typedef UnaryFunctorImageFilter<VolumeReal,VolumeReal,
			HoundsfieldToMassDensityTransform> HoundsfieldToMassDensityFilter;

	// helper to get formatted mass density volume
	DeclareMemberSPtr(HoundsfieldToMassDensityFilter, HoundsfieldToMassDensityFilter);

	// density resampler
	DeclareMemberSPtr(DensityResampler, dH::InPlaneResampleImageFilter);

};	// class CBeamDoseCalc

}