// Copyright (C) 2nd Messenger Systems
// $Id: SphereConvolve.h 601 2008-09-14 16:48:05Z dglane001 $
#pragma once

using namespace itk;
using namespace std;

class SphereConvolve
{
public:
	SphereConvolve(void);
	~SphereConvolve(void);

	// inputs to sphere convolve
	DECLARE_ATTRIBUTE_PTR(Density, VolumeReal);
	DECLARE_ATTRIBUTE_PTR(Terma, VolumeReal);

	// output from sphere convolve
	DECLARE_ATTRIBUTE_PTR(Energy, VolumeReal);

	// cumulative energy LUT, from the EDK
	// Z size = 1, as only one theta angle needs to be represented (because the kernel is rotationally symetric)
	DECLARE_ATTRIBUTE_PTR_GI(CumulativeEnergyLUT, VolumeReal);

	// top-level spherical convolution
	void CalcSphereConvolve();

protected:
	// spherical convolution ray trace (at a single point)
	void CalcSphereTrace(const VolumeReal::IndexType& index);

	// returns starting and ending of radial trace for the given index trace,
	//		for the request region of the Terma
	void CalcRadialClipping(const VolumeReal::IndexType& indexTrace, const VolumeReal::IndexType& index, 
		long& startRadial, long& endRadial);

	// using the cum energy LUT to interpolate the cumulative energy at the given rad dist
	REAL InterpCumEnergy(int nPhi, REAL radDist);

	// computes the spherical lookup table for the given output pixel spacing
	void ComputeSphereLUT();

	// initializes all LUTs
	void InitializeLUTs();

	// computes the lookup table entries for all radial values of a particular direction
	void ComputeDirLUT(const Vector<REAL>& vDir, VolumeReal::IndexType index);

	// computes the steps for one dimension in one direction
	void ComputeDirSteps(const Vector<REAL>& vDir, int nDim, REAL spacing, std::vector<REAL>& distances);

private:
	// these are all spherical-coordinate volumes
	//		X -> radial distance (in CM)
	//		Y -> phi angle (in deg)
	//		Z -> theta (azimuth) angle (in deg)
	// pixel spacing for each axis gives the distance in the designated unit

	// distance LUT gives physical distances for given radial, phi, theta
	//   NOTE: radial entry does not have uniform spacing, as it is the successive boundaries through
	//		all three cartesian dimensions
	VolumeReal::Pointer m_pDistanceLUT;

	// offset LUT gives voxel index offset for given radial, phi, theta
	//	 index is the same as for m_pDistanceLUT
	Image<VolumeReal::OffsetType, 3>::Pointer m_pOffsetLUT;

	// this is three LUTs (one for each dimension) that give the radial index at which a given X, Y, or Z, offset
	//		occurs
	// X -> absolute value of offset X/Y/Z - has no physical units
	// Y -> phi angle (in deg)
	// Z -> theta angle (in deg)
	VolumeShort::Pointer m_arrOffsetToRadiusLUT[3];
};
