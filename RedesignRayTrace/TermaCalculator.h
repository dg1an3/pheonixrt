#pragma once

//////////////////////////////////////////////////////////////////////////////
class TermaCalculator
	: public ImageSource<VolumeReal>
{
public:
	TermaCalculator();

  // Standard class typedefs. 
  typedef TermaCalculator Self;
  typedef ImageSource Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  // Method for creation through the object factory
  itkNewMacro(Self);

	// source and isocenter geometries
	DeclareMember(Isocenter, VolumeReal::PointType);
	DeclareMember(Source, VolumeReal::PointType);

	// source-fluence distance (defines plane of FluenceMap)
	DeclareMember(SourceFluenceDistance, double);

	typedef OrientedImage<double,2> FluenceMapType;

	// accessor for fluence map
	 FluenceMapType *GetFluenceMap();
	void SetFluenceMap(FluenceMapType *pFM);

	// accessor for density 
	VolumeReal *GetDensity();
	void SetDensity(VolumeReal *pDensity);

	// attenuation for calculation
	DeclareMember(_mu, double);

	// generates data from fluence map and density
	virtual void GenerateData();

	// performs ray trace of a single ray
	void TraceRayTerma(VolumeReal::PointType vStart, double energy0);

	// performs ray trace of a single ray
	void InterpolateTermaOut(const ContinuousIndex<double,3>& vAt, double deltaEnergy);
};
