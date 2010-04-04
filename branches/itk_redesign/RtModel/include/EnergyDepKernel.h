// Source.h: interface for the CSource class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <VectorN.h>
#include <MatrixNxM.h>

using namespace itk;

namespace dH {

// constants for the number of entries in the lok
const int NUM_THETA = 12; // 8;
const int NUM_RADIAL_STEPS = 64;

//////////////////////////////////////////////////////////////////////////////////
class EnergyDepKernel : 
		public DataObject
{
public:
	EnergyDepKernel();
	virtual ~EnergyDepKernel();

	// itk typedefs
	typedef EnergyDepKernel Self;
	typedef DataObject Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// member to set the energy - triggers load on setting
	DeclareMember(Energy, REAL);

	// reads the appropriate EDK
	void LoadKernel();

	// returns kernels attenuation coefficient
	DeclareMember(_mu, REAL);

	// returns kernels attenuation coefficient
	DeclareMember(TerminateDistance, REAL);

	// top-level spherical convolution
	void CalcSphereConvolve(const VolumeReal *pDensity, 
			const VolumeReal *pTerma, int nSlice,
			VolumeReal *pEnergy);

	// spherical convolution ray trace (at a single point)
	void CalcSphereTrace(const VolumeReal *pDensity, 
			const VolumeReal *pTerma, 
			const VolumeReal::IndexType& nNdx, VolumeReal *pEnergy);

protected:
	// returns number of phi (azimuth) angle increments
	int GetNumPhi();

	// EDK lookup table helpers

	// sets up cumulative energy LUT
	void InterpCumEnergy(const CMatrixNxM<>& mIncEnergy, 
					   const CVectorN<>& vRadialBounds);

	// returns cumulative energy for given angle / distance
	double GetCumEnergy(int nPhi, double rad_dist);

	// Raytrace lookup table helpers

	// sets up the radial lookup-table for the corresponding dose matrix grid
	void SetupRadialLUT(const Vector<REAL>& vPixSpacing);

	// returns the index offset for the given index
	const VolumeReal::OffsetType& 
		GetIndexOffset(int nTheta, int nPhi, int nRadial);

	// returns the radius for the given index
	double GetRadius(int nTheta, int nPhi, int nRadInc);

private:
	// energy for this kernel
	double m_energy;

	// attenuation coeffecient (H2O) for this energy
	double m_mu;

	// mean angle values from kernel
	CVectorN<double> m_vAnglesIn;

	// interpolated energy lookup table
	CMatrixNxM<double> m_mCumEnergy;

	// spherical xform lookup table
	Vector<REAL> m_vPixSpacing;

	// physical radius for three indices
	//		order of dimensions: THETA, PHI, radial
	double m_radius[NUM_THETA][48][65];

	// index offset for three indices
	//		order of dimensions: THETA, PHI, radial
	VolumeReal::OffsetType m_radialToOffset[NUM_THETA][48][64];

};	// class CEnergyDepKernel

}