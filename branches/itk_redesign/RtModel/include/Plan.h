// Copyright (C) 2000-2008  Derek G. Lane
// $Id: Plan.h,v 1.13 2007-12-10 02:57:08 Derek Lane Exp $
#pragma once

#include <Series.h>
#include <Beam.h>

namespace dH {

//////////////////////////////////////////////////////////////////////
class Plan : 
		public DataObject
	// represents a treatment plan
{
public:
	// constructor
	Plan();
	virtual ~Plan();

	// itk typedefs
	typedef Plan Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// series accessor
	DeclareMemberSPtrGet(Series, dH::Series);
	void SetSeries(dH::Series *pValue);

	// plan name
	DeclareMember(Name, CString);

	// the beams for this plan
	int GetBeamCount() const;
	dH::Beam * GetBeamAt(int nAt);
	int AddBeam(dH::Beam *pBeam);

	// creates a set of N equidistant beams
	void CreateEquidistantBeams(int nBeamCount, 
		const Vector<REAL>& vIsocenter);

	// helper functions
	int GetTotalBeamletCount();

	// sets shape for dose matrix
	DeclareMemberGI(DoseResolution, REAL);
	// void SetDoseResolution(const Real& value);

	// the computed dose for this plan (NULL if no dose exists)
	DeclareMemberSPtr(DoseMatrix, VolumeReal);

	// called to update dose matrix geometry, when DoseResolution or Series changes
	void UpdateDoseMatrixGeometry();

	// accumulator for adding up beamlet doses
	DeclareMemberSPtr(Accumulator, dH::IntensityMapAccumulateImageFilter);

	// called to update Accumulator geometry
	void UpdateBeamAccumulator();

	// serialization support
	void SerializeExt(CArchive& ar, int nSchema);

protected:
	// the plan's beams
	std::vector<dH::Beam::Pointer> m_arrBeams;

};	// class Plan

}
