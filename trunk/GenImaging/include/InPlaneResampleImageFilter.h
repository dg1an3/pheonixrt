// Copyright (C) 2008 DGLane
// $Id$
#pragma once

#include <itkImageToImageFilter.h>
#include <itkResampleImageFilter.h>

using namespace itk;

namespace dH {

//////////////////////////////////////////////////////////////////////////////
class InPlaneResampleImageFilter :
	public ResampleImageFilter<VolumeReal, VolumeReal>
{
public:
	InPlaneResampleImageFilter(void);
	~InPlaneResampleImageFilter(void);

	// itk typedefs
	typedef InPlaneResampleImageFilter Self;
	typedef ResampleImageFilter<VolumeReal, VolumeReal> Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// generates the data on update
	virtual void GenerateData();

protected:
	void ResamplePlaneLinearZ(
		REAL planeZ,
		VolumeReal::IndexType& inIndex, 
		VolumeReal::IndexType& outIndex, 
		const double coeffs[][3]);

	void ResamplePlaneNNeighborZ(
		VolumeReal::IndexType& inIndex, 
		VolumeReal::IndexType& outIndex,
		const double coeffs[][3]);

private:
	OutputImagePointer m_pBuffer;

	bool m_bResampleLinear;
};

}
