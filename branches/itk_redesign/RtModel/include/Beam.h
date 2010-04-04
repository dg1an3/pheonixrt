// Copyright (C) 2000-2008 DG Lane
// $Id: Beam.cpp,v 1.28 2007-12-10 02:57:30 Derek Lane Exp $
#pragma once

#include <IntensityMapAccumulateImageFilter.h>
#include <inPlaneResampleImageFilter.h>

using namespace itk;

namespace dH {

// forward declaration
class Plan;

//////////////////////////////////////////////////////////////////////
class Beam : 
		public DataObject 
	// represents a single treatment beam
{
public:
	// constructur/destructor
	Beam();
	virtual ~Beam();

	// itk typedefs
	typedef Beam Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// pointer to my plan
	DeclareMemberPtr(Plan, dH::Plan);

	// name of the beam
	DeclareMember(Name, CString);

	// angle values
	DeclareMemberGI(GantryAngle, double);
	// void SetGantryAngle(double angle);

	DeclareMember(CouchAngle, double);
	DeclareMember(CollimAngle, double);

	// table offset
	DeclareMember(Isocenter, Vector<REAL>);

	// pointer to the accumulator filter
	DeclareMemberSPtr(Accumulator, dH::IntensityMapAccumulateImageFilter);

	// beamlet accessors
	DeclareMemberSPtr(BeamletGroup, dH::BasisGroupType);
	int GetBeamletCount();
	VolumeReal::Pointer GetBeamlet(int nAt) { return NULL; }

	// sets up the dose geometry and beamlet arrays
	//		given the gantry angle, plan dose spacing, and passed beamlet count
	void UpdateDoseBeamletGeometry();

	// intensity map accessors
	DeclareMemberSPtr(IntensityMap, dH::IntensityMapType);

	// sets the intensity map geometry
	void UpdateIntensityMapGeometry(REAL spacing, dH::IntensityMapType::SizeType size);

	// the computed dose for this beam (NULL if no dose exists)
	// this dose is in the beam-specific geometry; i.e. aligned to IEC G system
	DeclareMemberSPtr(DoseMatrix, VolumeReal);

	// helper to resample the dose back to the plan's geometry
	DeclareMemberSPtr(DoseResampler, dH::InPlaneResampleImageFilter);

	// beam serialization
	void SerializeExt(CArchive &ar, int nSchema);

};	// class Beam

} 