#include "StdAfx.h"
#include "SphereConvolve.h"

#include "SortedGridBoundaryMap.h"

namespace dH {

	//////////////////////////////////////////////////////////////////////////////
	SphereConvolve::SphereConvolve(void)
	{
		SetNumberOfAzimuthalAngles(8);
		SetMaximumRadiologicalDistance(10.0);	// mm

		SetNormalizedEnergyFilter(RescaleIntensityImageFilter<VolumeReal>::New());
		GetNormalizedEnergyFilter()->SetInput(this->GetOutput());
		GetNormalizedEnergyFilter()->SetOutputMinimum(0.0);	
		GetNormalizedEnergyFilter()->SetOutputMaximum(1.0);			// this will automatically normalize the energy
	}

	//////////////////////////////////////////////////////////////////////////////
	SphereConvolve::~SphereConvolve(void)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	void 
		SphereConvolve::GenerateData()
	{
		// make sure terma is properly allocated
		this->AllocateOutputs();

		// set up LUT, if needed
		if (m_pDistanceLUT.IsNull())
			ComputeSphereLUT();

		CalcSphereConvolve();
	}

	//////////////////////////////////////////////////////////////////////////////
	void SphereConvolve::CalcSphereConvolve() 
		// spherical convolution
	{
		// initialize energy
		GetOutput()->FillBuffer(0.0);

		// Now do the convolution by iterating over the density voxels
		ImageRegionConstIterator<VolumeReal> iterDensity(GetDensity(), GetDensity()->GetBufferedRegion());
		for (; !iterDensity.IsAtEnd(); ++iterDensity)
		{
			// dose at zero density?
			if (iterDensity.Value() > 0.01) 
			{
				// spherical convolution at this point
				CalcSphereTrace(iterDensity.GetIndex()); 

				// normalize for azimuthal sum
				GetOutput()->GetPixel(iterDensity.GetIndex()) /= GetNumberOfAzimuthalAngles();

#ifdef USE_CONVERT_TO_DOSE
				// convert to Gy cm**2 and take into account the azimuthal sum
				GetOutput()->GetPixel(iterDensity.GetIndex()) *= 1.602e-10;
#endif
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	void 
		SphereConvolve::CalcSphereTrace(const VolumeReal::IndexType& index)
		// helper function to convolve at a single point in the energy volume
	{
		// set up the iteration region for the theta (azimuthal) and phi angle iterations
		VolumeReal::RegionType regionAngles = m_pDistanceLUT->GetBufferedRegion();
		// set size to 1 for radial dimension, to only iterate over angles
		regionAngles.SetSize(MakeSize(1, regionAngles.GetSize()[1], regionAngles.GetSize()[2]));

		// iterate over the theta and phi angles
		ImageRegionConstIterator<VolumeReal> iterAngles(m_pDistanceLUT, regionAngles);
		for (; !iterAngles.IsAtEnd(); ++iterAngles)
		{
			// stores total radiological distance traversed
			REAL radiologicalDistance = 0.0;

			// stores previous physical distance value
			REAL prevPhysicalDistance = 0.0;

			// stores previous cumulative energy value
			REAL prevEnergy = 0.0;

			// stores pointer to cumulative energy buffer for current angles values
			Index<3> cumEnergyIndex = iterAngles.GetIndex();
			cumEnergyIndex[2] = 0;		// theta is always 0 for cumEnergyLUT
			const VoxelReal *pCumEnergyPhiBuffer = &GetCumulativeEnergyLUT()->GetPixel(cumEnergyIndex);

			// set up the iteration region for the radial iterations
			VolumeReal::RegionType regionRadial = m_pDistanceLUT->GetBufferedRegion();
			regionRadial.SetIndex(iterAngles.GetIndex());
			// set size to 1 for angles, to only iterate over radial
			regionRadial.SetSize(MakeSize(regionRadial.GetSize()[0], 1, 1));

			// do for radial increments
			ImageRegionConstIterator<VolumeReal> iterDistanceLUT(m_pDistanceLUT, regionRadial);
			ImageRegionConstIterator< Image<VolumeReal::OffsetType, 3> > 
				iterOffsetLUT(m_pOffsetLUT, regionRadial);
			for (; !iterDistanceLUT.IsAtEnd(); ++iterDistanceLUT, ++iterOffsetLUT)         
			{
				// integer distances between the interaction and the dose depostion voxels
				VolumeReal::IndexType indexKernel = index - iterOffsetLUT.Value();
				if (!GetTerma()->GetRequestedRegion().IsInside(indexKernel))
					continue;		// continue, because we may not be already inside the volume

				// compute physical path length increment (in cm)
				REAL deltaPhysicalDistance = iterDistanceLUT.Value() - prevPhysicalDistance;
				prevPhysicalDistance = iterDistanceLUT.Value();

				// compute radiological path length increment
				REAL deltaRadiologicalDistance = deltaPhysicalDistance;
#ifdef USE_SUPERPOSITION
				// superposition
				deltaRadiologicalDistance *= GetDensity()->GetPixel(indexKernel); 
#endif
				deltaRadiologicalDistance /= GetKernelDensity();

				// update radiological path
				radiologicalDistance += deltaRadiologicalDistance;

				// quit after max distance
				if (radiologicalDistance < GetMaximumRadiologicalDistance())
					break;

				// Use lookup table to find the value of the cumulative energy
				// deposited up to this radius. No interpolation is done.
				int nCumEnergyAt = Round<int>(radiologicalDistance 
					/ GetCumulativeEnergyLUT()->GetSpacing()[0]);
				REAL totalEnergy = pCumEnergyPhiBuffer[nCumEnergyAt];

				// Subtract the last cumulative energy from the new cumulative energy
				// to get the amount of energy deposited in this interval and set the
				// last cumulative energy for the next time the lookup table is used.
				REAL deltaEnergy = totalEnergy - prevEnergy;
				prevEnergy = totalEnergy;             

				// The energy is accumulated - superposition
				GetOutput()->GetPixel(index) +=
					(VoxelReal) (deltaEnergy * GetTerma()->GetPixel(indexKernel));
			}     
		}	
	}

	///////////////////////////////////////////////////////////////////////////////////
	void 
		SphereConvolve::ComputeSphereLUT()
	{
		// construct all LUTs based on cumEnergy LUT and density matrix
		VolumeReal::SizeType sizeLUT = m_pCumulativeEnergyLUT->GetBufferedRegion().GetSize();
		sizeLUT[0] = 0; // size will be calculated by max from density volume size
		sizeLUT[2] = GetNumberOfAzimuthalAngles();

		// determine the maximum length and size for the boundary LUT
		VolumeReal::ConstPointer pDensity = GetInput(0);
		REAL lengthMax = 0.0;
		for (int nD = 0; nD < VolumeReal::ImageDimension; nD++)
		{
			int nSize = pDensity->GetLargestPossibleRegion().GetSize()[nD];
			sizeLUT[0] = __max(sizeLUT[0], nSize * 2);	
			lengthMax = __max(lengthMax, pDensity->GetSpacing()[nD] * nSize);
		}

		VolumeReal::PointType originLUT = m_pCumulativeEnergyLUT->GetOrigin();
		originLUT[2] = 0.0;		// set theta origin

		VolumeReal::SpacingType spacingLUT = m_pCumulativeEnergyLUT->GetSpacing();
		spacingLUT[0] = 0.0;	// no meaningful spacing in radial direction
		spacingLUT[2] = 2.0 * PI / (REAL) GetNumberOfAzimuthalAngles();	// set theta spacing

		// allocate the distance lookup table
		m_pDistanceLUT = VolumeReal::New();
		m_pDistanceLUT->SetRegions(sizeLUT);
		m_pDistanceLUT->Allocate();
		m_pDistanceLUT->SetOrigin(originLUT);
		m_pDistanceLUT->SetSpacing(spacingLUT);

		// allocate the offset lookup table
		m_pOffsetLUT = Image<VolumeReal::OffsetType, 3>::New();
		ConformTo<VolumeReal::OffsetType, 3>(m_pDistanceLUT, m_pOffsetLUT);

		// calculate the starting position (= index <0,0,0>)
		ContinuousIndex<REAL,3> vCenterIndex;
		vCenterIndex.Fill(0.0);

		// set up the iteration region for the theta (azimuthal) and phi angle iterations
		VolumeReal::RegionType regionAngles = m_pDistanceLUT->GetBufferedRegion();
		// set size to 1 for radial dimension, to only iterate over angles
		regionAngles.SetSize(MakeSize(1, regionAngles.GetSize()[1], regionAngles.GetSize()[2]));

		// iterate over the theta and phi angles
		ImageRegionConstIterator<VolumeReal> iterAngles(m_pDistanceLUT, regionAngles);
		for (; !iterAngles.IsAtEnd(); ++iterAngles)
		{
			// determine theta and phi from the spacing for the LUTs
			VolumeReal::PointType dirAngles;
			m_pDistanceLUT->TransformIndexToPhysicalPoint(iterAngles.GetIndex(), dirAngles);
			REAL phi = dirAngles[1];
			REAL theta = dirAngles[2];

			// calculate current direction in index coordinates
			Vector<double> vDirIndex;
			vDirIndex[0] = cos(theta) * sin(phi) / pDensity->GetSpacing()[0];
			vDirIndex[1] = sin(theta) * sin(phi) / pDensity->GetSpacing()[1];
			vDirIndex[2] = cos(phi) / pDensity->GetSpacing()[2];

			// now set up the grid boundary map
			SortedGridBoundaryMap<REAL,3> boundaryMap(vCenterIndex, vDirIndex, pDensity, lengthMax);

			// form an iterator for the boundary map
			SortedGridBoundaryMap<double,3>::const_iterator iterBoundaryMap = boundaryMap.begin();

			// set up the iteration region for the radial iterations in the LUT
			VolumeReal::RegionType regionRadial = m_pDistanceLUT->GetBufferedRegion();
			regionRadial.SetIndex(iterAngles.GetIndex());
			// set size to 1 for angles, to only iterate over radial
			regionRadial.SetSize(MakeSize(regionRadial.GetSize()[0], 1, 1));

			// do for radial increments
			ImageRegionIterator<VolumeReal> iterDistanceLUT(m_pDistanceLUT, regionRadial);
			ImageRegionIterator< Image<VolumeReal::OffsetType, 3> > 
				iterOffsetLUT(m_pOffsetLUT, regionRadial);
			for (; !iterDistanceLUT.IsAtEnd(); 
					++iterDistanceLUT, ++iterOffsetLUT, ++iterBoundaryMap)         
			{
				iterDistanceLUT.Set(iterBoundaryMap->first);
				iterOffsetLUT.Set(MakeOffset(iterBoundaryMap->second));
			}
		}
	}

}