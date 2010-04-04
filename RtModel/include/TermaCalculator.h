// Copyright (C) 2nd Messenger Systems
// $Id: TermaCalculator.h,v 1.0 2007-12-10 02:57:30 Derek Lane Exp $
#pragma once

#include <itkImageToImageFilter.h>
using namespace itk;

#include <IntensityMapAccumulateImageFilter.h>

namespace dH {

//////////////////////////////////////////////////////////////////////////////
class TermaCalculator
	: public ImageToImageFilter<VolumeReal, VolumeReal>
{
public:
	TermaCalculator(void);
	~TermaCalculator(void);

	// itk typedefs
	typedef TermaCalculator Self;
	typedef ImageToImageFilter<VolumeReal, VolumeReal> Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// beamlet geometry
	DeclareMember(Isocenter, VolumeReal::SpacingType);
	DeclareMember(SAD, Real);

	// ray trace geometry
	DeclareMember(RayTraceMin, dH::IntensityMapType::PointType);
	DeclareMember(RayTraceMax, dH::IntensityMapType::PointType);

	DeclareMember(RaysPerVoxel, Real);

	// _mu = attenuation coefficient
	DeclareMember(_mu, Real);

	// stores the energy surface integral for testing purposes
	DeclareMember(EnergySurfaceIntegral, Real);

	// generates the data on update
	virtual void GenerateData();

protected:
	// main ray trace
	void TraceRayTerma(ContinuousIndex<Real,3> vRay, const Vector<Real>& vDir, 
						const Real energy0);

	// helper functions for TraceRayTerma

	Real GetPhysicalLength(const VolumeReal::SpacingType& spacing, 
									   const Vector<Real>& vDir);

	Real TrilinearInterpDensity(const VolumeReal *pDensity,
									const ContinuousIndex<Real,3>& vPos, 
									const VolumeReal::IndexType& index, 
									Real (&weights)[3][3]);

	void UpdateTermaNeighborhood(VolumeReal *pTerma,
									 const VolumeReal::IndexType& nNdx, 
									 Real (&weights)[3][3], Real value);
};

}