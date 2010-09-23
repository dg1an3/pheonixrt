// Copyright (C) 2nd Messenger Systems
// $Id: Structure.h 640 2009-06-13 05:06:50Z dglane001 $
#if !defined(_STRUCTURE_H__INCLUDED_)
#define _STRUCTURE_H__INCLUDED_

// #include <ModelObject.h>

// #include <Polygon.h>
#include <ItkUtils.h>
using namespace itk;

#include <itkPolygonSpatialObject.h>
#include <itkMultiResolutionPyramidImageFilter.h> 

#include <itkGroupSpatialObject.h>
#include <itkSpatialObjectToImageFilter.h>
#include <itkJoinSeriesImageFilter.h>

namespace dH
{

// forward definition of Series class
class Series;

/**
 * represents a structure (ROI) defined in the CT coordinate system.
 * read in as a stack a contours, it is converted to a binary volume
 */
class Structure : public itk::DataObject
{
	Structure();
	virtual ~Structure();

public:
	/** itk typedefs */
	typedef Structure Self;
	typedef itk::DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	/** defines itk's New and CreateAnother static functions */
	itkNewMacro(Self);

	/** name of the structure */
	const std::string& GetName();
	void SetName(const std::string& strName);

	/** contour accessors */
	typedef itk::PolygonSpatialObject<2> PolygonType;
	int GetContourCount() const;
	PolygonType *GetContour(int nAt);
	REAL GetContourRefDist(int nIndex);

	void AddContour(PolygonType::Pointer pPoly, REAL refDist);

	/** constant for maximum scales */
	static const int MAX_SCALES = 5;

	/** multi-scale region accessor */
	const VolumeReal * GetRegion(int nLevel);

	/** forms / returns a region conformant to another volume */
	VolumeReal * GetConformRegion(itk::ImageBase<3> *pVolume);

	/** enum for structure type */
	enum  StructType 
	{ 
		eNONE = 0, 
		eTARGET = 1, 
		eOAR = 2
	};

	/** accessors for struct type */
	DeclareMember(Type, StructType);

	/** priority for structure (determines excluded region logic) */
	DeclareMemberGI(Priority, int);

	/** accessor for visible flag */
	DeclareMember(Visible, bool);

	/** accessor for display color */
	DeclareMember(Color, COLORREF);

	/** series accessor */
	DeclareMemberPtr(Series, dH::Series);

protected:

	/** region calc for base scale */
	void CalcRegion();

	/** helper - converts contours to a region */
	void ContoursToRegion(VolumeReal *pRegion);

private:
	/** the structure's name */
	std::string m_strName;

	/** contours for the structure */
	typedef std::multimap<REAL, PolygonType::Pointer> ContourMapType;
	ContourMapType m_arrContours;

	/** NEW: planar groups and regions */
	typedef GroupSpatialObject<2> ContourPlanarGroupType;
	typedef SpatialObjectToImageFilter<ContourPlanarGroupType, VolumeSliceReal> PlanarRegionFilterType;
	typedef std::map<REAL, PlanarRegionFilterType::Pointer> PlanarRegionFilterMapType;
	PlanarRegionFilterMapType m_mapPlanarRegionFilters;

	/** NEW: slice-to-volume filter: joins slices in to a single volume */
	typedef itk::JoinSeriesImageFilter<VolumeSliceReal, VolumeReal> SliceToVolumeFilterType;
	SliceToVolumeFilterType::Pointer m_pSliceToVolumeFilter;

	/** region (binary volume) representation (for base layer) */
	VolumeReal::Pointer m_pRegion0;

	/** pyramid for the regions */
	typedef MultiResolutionPyramidImageFilter<VolumeReal, VolumeReal> PyramidType;
	PyramidType::Pointer m_pPyramid;

	/** flag to indicate region recalc is needed */
	bool m_bRecalcRegion;

	/** stores cache of resampled regions */
	typedef itk::ResampleImageFilter<VolumeReal, VolumeReal> ResampleFilterType;
	std::vector< ResampleFilterType::Pointer > m_arrResamplers;

};	// class Structure

//////////////////////////////////////////////////////////////////////////////
inline
const std::string& 
	Structure::GetName()
{
	return m_strName;
}

//////////////////////////////////////////////////////////////////////////////
inline
void 
	Structure::SetName(const std::string& strName)
{
	m_strName = strName;
}

} // namespace dH

#endif // !defined(_STRUCTURE_H__INCLUDED_)