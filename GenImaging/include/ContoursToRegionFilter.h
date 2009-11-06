#pragma once

#include <itkPolylineMask2DImageFilter.h>
#include <itkPathToImageFilter.h>
#include <itkPolylineParametricPath.h>

using namespace itk;

namespace dH {

typedef PolyLineParametricPath<3> ContourType;
typedef ContinuousIndex<REAL, 3> VertexType;

//////////////////////////////////////////////////////////////////////////////
class ContoursToRegionFilter : 
	public PathToImageFilter<ContourType, VolumeReal>
{
public:
	ContoursToRegionFilter(void);
	~ContoursToRegionFilter(void);

	// itk typedefs
	typedef ContoursToRegionFilter Self;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// adds a contour to the filter
	void AddContour(ContourType::Pointer pContour);

	// Helper method to set the output parameters based on this image 
	void SetOutputParametersFromImage ( OutputImageType::Pointer Image );

	// over-ridden to set up size of output
	virtual void GenerateOutputInformation();

	// generates the data on update
  virtual void GenerateData();

	// generates data for a single contour
	void GeneratePlaneRegion(ContourType::ConstPointer contour);
};

}