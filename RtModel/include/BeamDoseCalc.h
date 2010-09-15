// Copyright (C) 2nd Messenger Systems
// $Id: BeamDoseCalc.h 600 2008-09-14 16:46:15Z dglane001 $
#pragma once

#include <ItkUtils.h>

using namespace itk;

class CEnergyDepKernel;
namespace dH
{
class Beam;
}

//////////////////////////////////////////////////////////////////////////////////
class CBeamDoseCalc  
{
public:
	// constructor / destructor
	CBeamDoseCalc(dH::Beam *pBeam, CEnergyDepKernel *pKernel); 
	virtual ~CBeamDoseCalc();

	// triggers calculation of beam's pencil beams
	void InitCalcBeamlets();
	void CalcBeamlet(int nBeamlet);

	// sets the rectangular region for the current beamlet, in IEC beam coordinates on
	//		the isocentric plane
	void SetBeamletMinMax(const Vector<REAL,2>& vMin_in,
					const Vector<REAL,2>& vMax_in);

	// vMin, vMax in physical coords at isocentric plane
	void CalcTerma();

	// performs single ray-trace calculation of terma
	void TraceRayTerma(Vector<REAL> vRay, const REAL fluence0);


	// helper functions for TERMA ray trace
	REAL GetPhysicalLength(const Vector<REAL>& vDir);
	REAL TrilinearInterpDensity(const Vector<REAL>& vPos, 
														const VolumeReal::IndexType& nNdx, 
														REAL (&weights)[3][3]);
	void UpdateTermaNeighborhood(const VolumeReal::IndexType& nNdx, 
														REAL (&weights)[3][3],  REAL value);

	//// top-level spherical convolution
	//void CalcSphereConvolve();

	//// spherical convolution ray trace (at a single point)
	//void CalcSphereTrace(const VolumeReal::IndexType& nNdx);

private:
	// my beam
	dH::Beam *m_pBeam;

	// reference to the source
	CEnergyDepKernel *m_pKernel;

	// Mass Density Dist variables
	VolumeReal::Pointer m_densityRep;

	// vSource -- source position, in voxel coordinates
	Vector<REAL> m_vSource_vxl;
	Vector<REAL> m_vIsocenter_vxl;

	// current beamlet min / max rectangle (in voxel coordinates)
	Vector<REAL> m_vMin_vxl;
	Vector<REAL> m_vMax_vxl;


	// TERMA calc variables

	// minimum number of rays to use per voxel (on top boundary)
	REAL m_raysPerVoxel;

	// initialize surface integral of fluence 
	REAL m_fluenceSurfIntegral;

	// returns computed terma
	VolumeReal::Pointer m_pTerma;


	// SphereConvolve variables

	// computed energy
	VolumeReal::Pointer m_pEnergy;

};	// class CBeamDoseCalc

