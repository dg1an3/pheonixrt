// Copyright (C) 2nd Messenger Systems
// $Id: Beam.h 600 2008-09-14 16:46:15Z dglane001 $
#pragma once

#include <vector>
using namespace std;

#include <ItkUtils.h>
using namespace itk;

namespace dH
{

// forward declaration
class Plan;

/**
 * represents a single treatment beam
 */
class Beam : public DataObject
{
	/** constructur/destructor */
	Beam();
	virtual ~Beam();

public:
	/** itk typedefs */
	typedef Beam Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	itkNewMacro(Self);

	// pointer to my plan
	DECLARE_ATTRIBUTE_PTR(Plan, dH::Plan);

	/** gantry angle accessors */
	double GetGantryAngle() const;
	void SetGantryAngle(double gantryAngle);

	/** beam isocenter value */
	DECLARE_ATTRIBUTE(Isocenter, itk::Vector<REAL>);

	/** beamlet accessors */
	int GetBeamletCount();
	VolumeReal *GetBeamlet(int nShift);

	/** intensity map accessors */
	typedef itk::Image<VOXEL_REAL, 1> IntensityMap;
	IntensityMap * GetIntensityMap() const;
	void SetIntensityMap(const CVectorN<>& vWeights);

	/** call to deal with intensity map changes */
	void OnIntensityMapChanged();

	/** the computed dose for this beam (NULL if no dose exists) */
	virtual VolumeReal *GetDoseMatrix();

protected:
	/** GenBeamlets must access this */
	friend void GenBeamlets(Beam *pBeam);

	/** CBeamDoseCalc must access this */
	friend class CBeamDoseCalc;

public:
	/** TODO: make this private */
	mutable VolumeReal::Pointer m_dose;
	mutable VolumeReal::Pointer m_doseAccumBuffer;

private:
	/** gantry angle for beam */
	double m_gantryAngle;

	/** flag to recalculate dose */
	mutable bool m_bRecalcDose;

public:

	/** the beamlets for the beam */
	std::vector< VolumeReal::Pointer > m_arrBeamlets;

	/** flag for recalc of beamlets */
	bool m_bRecalcBeamlets;

	/** the intensity map */
	IntensityMap::Pointer m_vBeamletWeights;

};	// class Beam

}

typedef dH::Beam CBeam;
