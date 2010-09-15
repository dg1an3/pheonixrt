// Copyright (C) 2nd Messenger Systems
// $Id: Series.h 640 2009-06-13 05:06:50Z dglane001 $
#if !defined(_SERIES_H__INCLUDED_)
#define _SERIES_H__INCLUDED_

#include <Structure.h>

namespace dH
{

/**
 * Series groups structure and CT in to single object
 */
class Series : public DataObject
{
	Series();    
public:
	/** itk typedefs */
	typedef Series Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	/** defines itk's New and CreateAnother static functions */
	itkNewMacro(Self);      

	/** Volume data for the series */ 
	VolumeReal *GetDensity();
	void SetDensity(VolumeReal *pValue);

	/** Structures for the series */
	int GetStructureCount() const;
	Structure * GetStructureAt(int nAt);
	Structure * GetStructureFromName(const std::string &strName);
	void AddStructure(Structure *pStruct);

private:
	/** density volume for the series */
	VolumeReal::Pointer m_pDensity;

	/** the structure array */
	std::vector<Structure::Pointer> m_arrStructures;
};

}	// namespace dH

#endif // !defined(_SERIES_H__INCLUDED_)