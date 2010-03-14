// Copyright (C) 2nd Messenger Systems
// $Id: Series.h 640 2009-06-13 05:06:50Z dglane001 $
#pragma once

#include <Structure.h>

BeginNamespace(dH)

///////////////////////////////////////////////////////////////////////////////
class Series : 
		public DataObject
{
public:
	Series();          
	virtual ~Series();

	// itk typedefs
	typedef Series Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// Structures for the series
	int GetStructureCount() const;
	dH::Structure *GetStructureAt(int nAt);
	dH::Structure *GetStructureFromName(const CString& strName);
	void AddStructure(dH::Structure *pStruct);

	// Volume data for the series
	DeclareMemberSPtr(Density, VolumeReal);

	// called to update structure pipelines for priorities
	void UpdateStructurePipelines();

	// serialization
	void SerializeExt(CArchive& ar, int nSchema);

private:
	// the structure array
	std::vector< dH::Structure::Pointer > m_arrStructures;
};

EndNamespace(dH)

