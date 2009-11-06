// Copyright (C) 2nd Messenger Systems - U. S. Patent 0,000,000
// $Id: IntensityMapAccumulateImageFilter.h 2008-MM-DD Derek Lane $
#pragma once

#include <itkImageToImageFilter.h>
using namespace itk;

namespace dH {

typedef Image<VoxelReal, 2> IntensityMapType;
typedef Image<VolumeReal *, 2> BasisGroupType;

//////////////////////////////////////////////////////////////////////////////
class IntensityMapAccumulateImageFilter :
	public ImageToImageFilter<IntensityMapType, VolumeReal>
{
public:
	IntensityMapAccumulateImageFilter(void);
	~IntensityMapAccumulateImageFilter(void);

	// itk typedefs
	typedef IntensityMapAccumulateImageFilter Self;
	typedef ImageToImageFilter<IntensityMapType, VolumeReal> Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// sets the basis volumes for the accumulation
	DeclareMemberSPtr(BasisGroup, BasisGroupType);

	// initialize the geometry of the basis group to match that of the intensity map
	//		NOTE: this will call UnRegister on any existing basis volumes and sets
	//			all pointers to NULL
	void InitBasisGroupGeometry();

	// sets the members of the basis group as inputs to the filter - needed if 
	//		changes in the basis volumes need to propagate through the accumulator
	void SetBasisGroupAsInput(bool bEnable);

	// over-ridden to do nothing (sizes need to be set prior to calling the filter)
	virtual void GenerateOutputInformation() { }

	// over-ride because base class will erroneously pass 3-d to 2-d
	virtual void GenerateInputRequestedRegion() { }

	// generates the data on update
	virtual void GenerateData();

private:
	// this used to hold references to the basis volumes (because the actual BasisGroupType doesn't hold references)
	std::vector< VolumeReal::Pointer > m_arrBasisVolumes;

	// helper buffer for accumulation
	VolumeReal::Pointer m_pBuffer;
};

}