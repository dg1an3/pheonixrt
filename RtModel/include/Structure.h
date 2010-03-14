// Copyright (C) 2nd Messenger Systems
// $Id: Structure.h 640 2009-06-13 05:06:50Z dglane001 $
#pragma once

#include <ItkUtils.h>
using namespace itk;

#include <itkMultiResolutionPyramidImageFilter.h> 
#include <ContoursToRegionFilter.h>
#include <MultiMaskNegatedImageFilter.h>
// #include <InPlaneResampleImageFilter.h>

BeginNamespace(dH)

class Series;

//////////////////////////////////////////////////////////////////////
class Structure : public DataObject
{
	// constructor / destructor
	Structure();
	virtual ~Structure();

public:
	// itk typedefs
	typedef Structure Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// serialization
	void SerializeExt(CArchive& ar, int nSchema);

	// name of the structure
	DeclareMember(Name, CString);

	// series accessor
	DeclareMemberPtr(Series, dH::Series);

	// contour accessors
	int GetContourCount() const;
	dH::ContourType *GetContourPoly(int nAt);
	void AddContourPoly(dH::ContourType *pContour);

	// constant for maximum scales
	static const int MAX_SCALES = 5;

	// multi-scale region accessor
	const VolumeReal * GetRegion(int nLevel);

	// forms / returns a region conformant to another volume
	VolumeReal * GetConformRegion(itk::ImageBase<3> *pVolume);

	// enum for structure type
	enum  StructType 
	{ 
		eNONE = 0, 
		eTARGET = 1, 
		eOAR = 2
	};

	// accessors for struct type
	DeclareMember(Type, StructType);

	// priority for structure (determines excluded region logic)
	DeclareMemberGI(Priority, int);

	// accessor for visible flag
	DeclareMember(Visible, bool);

	// accessor for display color
	DeclareMember(Color, COLORREF);

//protected:

	// called to update inputs to the mask filter 
	//	(if priorities change, or if structures are added / removed)
	void UpdatePipeline();


private:
	// contours-to-region filter (also container for contours)
	dH::ContoursToRegionFilter::Pointer m_pContoursToRegion;

	// negated mask filter (for exclusion operation)
	dH::MultiMaskNegatedImageFilter::Pointer m_pMultiMaskNegatedFilter;

	// pyramid for the regions
	typedef itk::MultiResolutionPyramidImageFilter<VolumeReal, VolumeReal> PyramidType;
	PyramidType::Pointer m_pPyramid;

	// stores cache of resampled regions
	typedef itk::ResampleImageFilter<VolumeReal, VolumeReal> ResamplerType;
	std::vector< ResamplerType::Pointer > m_arrResamplers;

};	// class Structure

EndNamespace(dH)