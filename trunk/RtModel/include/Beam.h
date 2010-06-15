// Copyright (C) 2nd Messenger Systems
// $Id: Beam.h 600 2008-09-14 16:46:15Z dglane001 $
#pragma once

#include <vector>
using namespace std;

// model object base class
#include <ModelObject.h>

// geom model classes
#include <Polygon.h>

// forward declaration
class CPlan;

//////////////////////////////////////////////////////////////////////
// class CBeam
//
// represents a single treatment beam
//////////////////////////////////////////////////////////////////////
class CBeam : public CModelObject
{
public:
	// constructur/destructor
	CBeam();
	virtual ~CBeam();

#ifdef USE_MFC_SERIALIZATION
	// serialization support
	DECLARE_SERIAL(CBeam)
#endif

	// pointer to my plan
	DECLARE_ATTRIBUTE_PTR(Plan, CPlan);

	//// angle values
	double GetGantryAngle() const;
	void SetGantryAngle(double gantryAngle);
	double GetCouchAngle() const;
	void SetCouchAngle(double couchAngle);

	// table offset
	DECLARE_ATTRIBUTE(Isocenter, itk::Vector<REAL>);

	// beamlet accessors
	int GetBeamletCount();
	VolumeReal *GetBeamlet(int nShift);

	// intensity map accessors
	typedef itk::Image<VOXEL_REAL, 1> IntensityMap;
	IntensityMap * GetIntensityMap() const;
	void SetIntensityMap(const CVectorN<>& vWeights);

	// call to deal with intensity map changes
	void OnIntensityMapChanged();

	// the computed dose for this beam (NULL if no dose exists)
	virtual VolumeReal *GetDoseMatrix();

#ifdef USE_MFC_SERIALIZATION
	// beam serialization
	void Serialize(CArchive &ar);
#endif

protected:
	friend void GenBeamlets(CBeam *pBeam);

	// CBeamDoseCalc must access this
	friend class CBeamDoseCalc;

public:
	/// TODO: make this private
	mutable VolumeReal::Pointer m_dose;
	mutable VolumeReal::Pointer m_doseAccumBuffer;

private:
	// angles
	/// TODO: get rid of collim
	double m_collimAngle;
	double m_gantryAngle;
	double m_couchAngle;

	// table parameters
	/// TODO: get rid of table offset
	itk::Vector<REAL> m_vTableOffset;

	// collimator jaw settings
	itk::Vector<REAL, 2> m_vCollimMin;
	itk::Vector<REAL, 2> m_vCollimMax;

	// collection of blocks
	/// TODO: get rid of blocks
	// CTypedPtrArray<CPtrArray, CPolygon* > m_arrBlocks;

	mutable bool m_bRecalcDose;

public:

	// the beamlets for the beam
	std::vector< VolumeReal::Pointer > m_arrBeamlets;

	// flag for recalc of beamlets
	/// TODO: get rid of this (used for sub-beamlets
	bool m_bRecalcBeamlets;

	// the intensity map
	IntensityMap::Pointer/*CVectorN<>*/ m_vBeamletWeights;

};	// class CBeam

#ifdef USING_CLI
namespace dH
{

public class Beam : public System::Object
{
	Beam();

	property REAL GantryAngle;
	property REAL CouchAngle;
	property itk::Point<3> Isocenter;

	itk::Image<3> **m_ppBeamlets;
	itk::Image<2> *m_pIntensityMap;
};

}
#endif
