// Copyright (C) 2nd Messenger Systems
// $Id: SphereConvolve.cpp 602 2008-09-14 16:54:49Z dglane001 $
#include "StdAfx.h"
#include "SphereConvolve.h"

const int NUM_THETA = 8;
const int NUM_RADII = 64;

//////////////////////////////////////////////////////////////////////////////
SphereConvolve::SphereConvolve(void)
{
}

//////////////////////////////////////////////////////////////////////////////
SphereConvolve::~SphereConvolve(void)
{
}

//////////////////////////////////////////////////////////////////////////////
void SphereConvolve::CalcSphereConvolve() 
	// spherical convolution
{
	// initialize energy
	m_pEnergy = VolumeReal::New();
	ConformTo<VOXEL_REAL,3>(GetTerma(), m_pEnergy);
	m_pEnergy->FillBuffer(0.0);

	// calculate all LUTs
	ComputeSphereLUT();

	// Now do the convolution.
	VolumeReal::IndexType index;
	for (index[2] = 0; index[2] < GetDensity()->GetBufferedRegion().GetSize()[2]; index[2]++)         
	{
		for (index[1] = 0; index[1] < GetDensity()->GetBufferedRegion().GetSize()[1]; index[1]++)        
		{
			for (index[0] = 0; index[0] < GetDensity()->GetBufferedRegion().GetSize()[0]; index[0]++)          
			{
				// dose at zero density?
				if (GetDensity()->GetPixel(index) > 0.01) 
				{
					// spherical convolution at this point
					CalcSphereTrace(index); 

					// Convert the energy to dose by dividing by mass
					// convert to Gy cm**2 and take into account the azimuthal sum
					m_pEnergy->GetPixel(index) *= 1.0 / (REAL) NUM_THETA;
					// 	(VOXEL_REAL) (1.602e-10 / (REAL) NUM_THETA);

					if (GetDensity()->GetPixel(index) > 0.25)
						m_pEnergy->GetPixel(index) /= GetDensity()->GetPixel(index);
					else
						m_pEnergy->GetPixel(index) = 0.0;
				}
			}
		}
	}

	// get new maximum for energy dist
	// TODO: move max calc to ItkUtils
	REAL dmax = -1e+8;
	ConstVolumeRealIterator inputIt( m_pEnergy, m_pEnergy->GetBufferedRegion() );
	for ( inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
	{
		dmax = __max(inputIt.Get(), dmax);
	}

	// now normalize to dmax
	// TODO: replace with an image scale operation
	if (dmax > 0.0)
	{
		VolumeRealIterator outputIt( m_pEnergy, m_pEnergy->GetBufferedRegion() );
		for ( outputIt.GoToBegin(); !outputIt.IsAtEnd(); ++outputIt)
		{
			outputIt.Value() /= dmax;
		}
	}

}	// SphereConvolve::CalcSphereConvolve

///////////////////////////////////////////////////////////////////////////////
void 
	SphereConvolve::CalcSphereTrace(const VolumeReal::IndexType& index)
	// helper function to convolve at a single point in the energy volume
{
	// TODO: move this to EnergyDepKernel
	const REAL kernelDensity = 1.0;

	const VolumeReal::IndexType& start = m_pDistanceLUT->GetBufferedRegion().GetIndex();
	const VolumeReal::SizeType& size = m_pDistanceLUT->GetBufferedRegion().GetSize();

	// store the spacing for looking up cumulative energy
	const VolumeReal::SpacingType& spacingCumEnergyLUT = GetCumulativeEnergyLUT()->GetSpacing();

	// store reference to the current energy voxel, for speed
	VolumeReal::ValueType& currentEnergyVoxel = m_pEnergy->GetPixel(index);

	// do for all theta (azimuthal) angles
	VolumeReal::IndexType indexTrace;
	for (indexTrace[2] = start[2]; indexTrace[2] < size[2]; indexTrace[2]++)         
	{
		// do for zenith angles 
		for (indexTrace[1] = start[1]; indexTrace[1] < size[1]; indexTrace[1]++)         
		{
			// stores index for accessing the cumEnergyLUT
			Index<3> cumEnergyIndex = indexTrace;
			cumEnergyIndex[2] = 0;		// theta is always 0 for cumEnergyLUT

			// stores total radiological distance traversed
			REAL radDist = 0.0;

			// stores previous cumulative energy value
			REAL prevEnergy = 0.0;
			
			// calculate indexTrace[0] start, finish from Terma request region
			//		using OffsetToRadius LUTs
			long startRadial = start[0];
			long endRadial = size[0];
			CalcRadialClipping(indexTrace, index, startRadial, endRadial);

			// do for radial increments
			for (indexTrace[0] = startRadial; indexTrace[0] < endRadial; indexTrace[0]++)         
			{
				// integer distances between the interaction and the dose depostion voxels
				VolumeReal::IndexType indexKernel = index;
				indexKernel -= m_pOffsetLUT->GetPixel(indexTrace);
				ASSERT(GetTerma()->GetRequestedRegion().IsInside(indexKernel));

				// compute physical path length increment (in cm)
				REAL deltaPhysDist = m_pDistanceLUT->GetPixel(indexTrace);

				// compute radiological path length increment
				REAL deltaRadDist = deltaPhysDist 
						//* 1.0 // convolution
						* GetDensity()->GetPixel(indexKernel) // superposition
						/ kernelDensity;
				
				// update radiological path
				radDist += deltaRadDist;

				// quit after 60cm rad dist
				if (radDist > 10.0) // 60.0)
				{
					break;
				}

				// Use lookup table to find the value of the cumulative energy
				// deposited up to this radius. No interpolation is done.
				cumEnergyIndex[0] = Round<int>(radDist / spacingCumEnergyLUT[0]); // TODO: check this
				REAL totalEnergy = GetCumulativeEnergyLUT()->GetPixel(cumEnergyIndex);
			
				// Subtract the last cumulative energy from the new cumulative energy
				// to get the amount of energy deposited in this interval and set the
				// last cumulative energy for the next time the lookup table is used.
				REAL energy = totalEnergy - prevEnergy;
				prevEnergy = totalEnergy;             
				
				// The energy is accumulated - superposition
				currentEnergyVoxel +=
					(VOXEL_REAL) (energy * GetTerma()->GetPixel(indexKernel));
			}     
		}	
	}

}	// SphereConvolve::CalcSphereTrace

///////////////////////////////////////////////////////////////////////////////////
void 
	SphereConvolve::CalcRadialClipping(const VolumeReal::IndexType& indexTrace, 
			const VolumeReal::IndexType& index, 
			long& startRadial, long& endRadial)
	// returns starting and ending of radial trace for the given index trace,
	//		for the request region of the Terma
{
	// get start index
	VolumeReal::IndexType startTerma = GetTerma()->GetRequestedRegion().GetIndex();
	VolumeReal::IndexType endTerma = GetTerma()->GetRequestedRegion().GetIndex();
	endTerma += GetTerma()->GetRequestedRegion().GetSize();

	VolumeReal::IndexType indexOffsetToRadius = indexTrace;
	for (int nD = 0; nD < 3; nD++)
	{
		indexOffsetToRadius[0] = abs(index[nD] - startTerma[nD]);
		startRadial = __max(startRadial, 
			m_arrOffsetToRadiusLUT[nD]->GetPixel(indexOffsetToRadius));

		indexOffsetToRadius[0] = abs(index[nD] - endTerma[nD]);
		endRadial = __min(endRadial, 
			m_arrOffsetToRadiusLUT[nD]->GetPixel(indexOffsetToRadius));
	}

}

///////////////////////////////////////////////////////////////////////////////////
void 
	SphereConvolve::ComputeSphereLUT()
{
	// construct all LUTs based on cumEnergy LUT and density matrix
	InitializeLUTs();

	const VolumeReal::PointType& origin = m_pDistanceLUT->GetOrigin();
	const VolumeReal::SpacingType& spacing = m_pDistanceLUT->GetSpacing();

	const VolumeReal::IndexType& start = m_pDistanceLUT->GetBufferedRegion().GetIndex();
	const VolumeReal::SizeType& size = m_pDistanceLUT->GetBufferedRegion().GetSize();

	// index holds the current position in the LUT
	VolumeReal::IndexType index = start;		
	REAL theta = origin[2];
	for (; index[2] < size[2]; index[2]++)
	{
		REAL phi = origin[1];
		for (index[1] = start[1]; index[1] < size[1]; index[1]++)
		{
			Vector<double> vDir;
			vDir[0] = cos(theta) * sin(phi);
			vDir[1] = sin(theta) * sin(phi);
			vDir[2] = cos(phi);

			ComputeDirLUT(vDir, index);

			phi += spacing[1];
		}
		theta += spacing[2];
	}
}

///////////////////////////////////////////////////////////////////////////////////
void 
	SphereConvolve::InitializeLUTs()
{
	VolumeReal::SizeType sizeLUT = m_pCumulativeEnergyLUT->GetBufferedRegion().GetSize();
	sizeLUT[0] = NUM_RADII;
	sizeLUT[2] = NUM_THETA;

	VolumeReal::PointType originLUT = m_pCumulativeEnergyLUT->GetOrigin();
	originLUT[2] = 0.0;		// set theta origin
	VolumeReal::SpacingType spacingLUT = m_pCumulativeEnergyLUT->GetSpacing();
	spacingLUT[0] = 0.0;	// no meaningful spacing in radial direction
	spacingLUT[2] = 2.0 * PI / (REAL) NUM_THETA;	// set theta spacing

	m_pDistanceLUT = VolumeReal::New();
	m_pDistanceLUT->SetRegions(sizeLUT);
	m_pDistanceLUT->Allocate();
	m_pDistanceLUT->SetOrigin(originLUT);
	m_pDistanceLUT->SetSpacing(spacingLUT);

	m_pOffsetLUT = Image<VolumeReal::OffsetType, 3>::New();
	m_pOffsetLUT->SetRegions(sizeLUT);
	m_pOffsetLUT->Allocate();
	m_pOffsetLUT->SetOrigin(originLUT);
	m_pOffsetLUT->SetSpacing(spacingLUT);

	// the radius index LUT provides for each dimension, and for a given theta/phi value, the radius
	//		at which a given offset appears in the offsetLUT
	for (int nDim = 0; nDim < 3; nDim++)
	{
		m_arrOffsetToRadiusLUT[nDim] = VolumeShort::New();
		m_arrOffsetToRadiusLUT[nDim]->SetRegions(sizeLUT);
		m_arrOffsetToRadiusLUT[nDim]->Allocate();
		m_arrOffsetToRadiusLUT[nDim]->SetOrigin(originLUT);
		m_arrOffsetToRadiusLUT[nDim]->SetSpacing(spacingLUT);
	}
}

///////////////////////////////////////////////////////////////////////////////////
void 
	SphereConvolve::ComputeDirLUT(const Vector<REAL>& vDir, VolumeReal::IndexType index)
{
	const VolumeReal::SizeType& size = m_pDistanceLUT->GetBufferedRegion().GetSize();
	const VolumeReal::SpacingType& spacingDensity = GetDensity()->GetSpacing();

	// compute directions for each dimension
	std::vector<REAL> distances[3];
	for (int nD = 0; nD < 3; nD++)
	{
		distances[nD].resize(size[0]);
		ComputeDirSteps(vDir, nD, spacingDensity[nD] * 0.1 /* convert to CM */, 
			distances[nD]);
	}

	// set index direction based on vDir
	Index<3> offsetDirection;
	for (int nD = 0; nD < 3; nD++)
		offsetDirection[nD] = vDir[nD] < 0.0 ? -1 : (vDir[nD] > 0.0 ? 1 : 0);

	// stores current position in each of the individual distances tables
	Offset<3> offsetForDim;

	// this is used to calculate the index in the offsetToRadiusLUT
	Index<3> offsetToRadiusIndex = index;

	// now sort by distances and create index list
	for (index[0] = 0; index[0] < size[0]; index[0]++)
	{
		for (int nD = 0; nD < 3; nD++)
		{
			bool bLTE1 = distances[nD][offsetForDim[nD]] 
					<= distances[(nD+1)%3][offsetForDim[(nD+1)%3]]; 
			bool bLTE2 = distances[nD][offsetForDim[nD]] 
					<= distances[(nD+2)%3][offsetForDim[(nD+2)%3]];
			if (bLTE1 || bLTE2)
			{
				m_pDistanceLUT->SetPixel(index, distances[nD][offsetForDim[nD]]);
				m_pOffsetLUT->SetPixel(index, offsetForDim);

				// set the entry in the proper offsetToRadiusLUT
				offsetToRadiusIndex[0] = offsetForDim[nD];
				m_arrOffsetToRadiusLUT[nD]->SetPixel(offsetToRadiusIndex, index[0]);

				// next dimension step
				offsetForDim[nD]++;

				break;
			}
		}

		// correct for actual direction
		for (int nD = 0; nD < 3; nD++)
			m_pOffsetLUT->GetPixel(index)[nD] *= offsetDirection[nD];
	}	
}

///////////////////////////////////////////////////////////////////////////////////
void 
	SphereConvolve::ComputeDirSteps(const Vector<REAL>& vDir, int nDim, 
			REAL spacing, std::vector<REAL>& distances)
	// vDir is a normalized vector in the direction of the ray
	// nDim is the dimension to be computed
{
	// calculate the step length to step from one nDim boundary to the next
	double distancePerStep = 1e+6;

	// scale the direction vector to the length needed to traverse one nDim boundary (with given spacing)
	if (vDir[nDim] > 1e-6)
	{
		// calculate scaling factor needed to multiply vDir so that it just crosses one
		//		nDim boundary
		distancePerStep = spacing / vDir[nDim];
	}
	
	distances[0] = distancePerStep / 2.0;	// first step is half-way across center voxel
	for (int nAt = 0; nAt < distances.size(); nAt++)
	{
		distances[nAt] = distances[nAt-1] + distancePerStep;
	}
}
