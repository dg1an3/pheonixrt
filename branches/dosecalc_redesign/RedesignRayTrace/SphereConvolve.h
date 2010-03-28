// Copyright (C) 2008 DGLane
// $Id$
#pragma once

#include <itkImageToImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
using namespace itk;

//#include <EnergyDepKernel.h>

namespace dH {

//////////////////////////////////////////////////////////////////////////////
class SphereConvolve 
	: public ImageToImageFilter<VolumeReal, VolumeReal>
{
public:
	SphereConvolve(void);
	~SphereConvolve(void);

	// itk typedefs
	typedef SphereConvolve Self;
	typedef ImageToImageFilter<VolumeReal, VolumeReal> Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// inputs to sphere convolve
	DeclareMemberSPtrGet(Density, VolumeReal);
	void SetDensity(VolumeReal *pDensity);
	DeclareMemberSPtrGet(Terma, VolumeReal);
	void SetTerma(VolumeReal *pTerma);

	// the kernel density
	DeclareMember(KernelDensity, REAL);

	// number of azimuthal (theta) angles to compute
	DeclareMember(NumberOfAzimuthalAngles, int);

	// distance at which to quit superposition, in mm
	DeclareMember(MaximumRadiologicalDistance, REAL);

	// cumulative energy LUT, from the EDK
	// Z size = 1, as only one theta angle needs to be represented (because the kernel is rotationally symetric)
	DeclareMemberSPtr(CumulativeEnergyLUT, VolumeReal);

	// filter to scale dose to dmax
	DeclareMemberSPtr(NormalizedEnergyFilter, RescaleIntensityImageFilter<VolumeReal>);

	// generates the data on update
	virtual void GenerateData();

	// top-level spherical convolution
	void CalcSphereConvolve();

protected:
	// spherical convolution ray trace (at a single point)
	void CalcSphereTrace(const VolumeReal::IndexType& index);

	// computes the spherical lookup table for the given output pixel spacing
	void ComputeSphereLUT();

private:
	// these are all spherical-coordinate volumes
	//		X -> radial distance (in CM)
	//		Y -> phi angle (in deg)
	//		Z -> theta (azimuth) angle (in deg)
	// pixel spacing for each axis gives the distance in the designated unit

	// distance LUT gives physical distances for given radial, phi, theta
	//   NOTE: radial entry does not have uniform spacing, as it is the successive boundaries through
	//		all three cartesian dimensions
	VolumeReal::Pointer m_pDistanceLUT;

	// offset LUT gives voxel index offset for given radial, phi, theta
	//	 index is the same as for m_pDistanceLUT
	Image<VolumeReal::OffsetType, 3>::Pointer m_pOffsetLUT;
};

}