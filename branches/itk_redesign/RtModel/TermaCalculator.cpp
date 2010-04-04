// Copyright (C) 2nd Messenger Systems
// $Id: TermaCalculator.h,v 1.0 2007-12-10 02:57:30 Derek Lane Exp $
#include "StdAfx.h"
#include "TermaCalculator.h"

#include <itkOrientedImage.h>

namespace dH {

//////////////////////////////////////////////////////////////////////////////
TermaCalculator::TermaCalculator(void)
{
	SetSAD(1000.0);
	SetRaysPerVoxel(12);
}

//////////////////////////////////////////////////////////////////////////////
TermaCalculator::~TermaCalculator(void)
{
}

// consts for index positions
const int X = 1;
const int Y = 2;
const int Z = 0;

//////////////////////////////////////////////////////////////////////////////
void 
	TermaCalculator::GenerateData()
{
	// make sure terma is properly allocated
	this->AllocateOutputs();

	VolumeReal::ConstPointer pDensity = GetInput();

	// OrientedImage handles directions correctly (unlike Image)
	OrientedImage<VoxelReal,3>::Pointer pDensityHelper = OrientedImage<VoxelReal,3>::New();
	ConformTo<VoxelReal,3>(pDensity, pDensityHelper);

	// form the isocenter coordinate in density index coordinates
	ContinuousIndex<Real,3> vIsocenterIndex;
	pDensityHelper->TransformPhysicalPointToContinuousIndex<Real>(MakePoint(GetIsocenter()), vIsocenterIndex);

	// spacing factor for three coordinates
	//		= conversion from voxel to physical coordinates
	const VolumeReal::SpacingType& spacing = pDensity->GetSpacing();

	// calc source index position using SAD
	// Vector<Real> 
	Point<Real> vSourceIndex = vIsocenterIndex;
	vSourceIndex[0] -= GetSAD() / spacing[Z];

	// calculate conversion from isocenter to top boundary
	const Real rayTraceStartZ = -0.5;		// start coordinate in the Z-direciton
	const Real isocenterToTopScale = 
		(rayTraceStartZ - vSourceIndex[Z])		// z-distance from upper boundary to source
			/ (vIsocenterIndex[Z] - vSourceIndex[Z]);

	// set up starting min vector position in index coords
	ContinuousIndex<Real,3> vRayTraceMinIndex = vIsocenterIndex;
	vRayTraceMinIndex[X] += isocenterToTopScale * GetRayTraceMin()[0] / spacing[X];
	vRayTraceMinIndex[Y] += isocenterToTopScale * GetRayTraceMin()[1] / spacing[Y];

	// set up max vector position in index coords
	ContinuousIndex<Real,3> vRayTraceMaxIndex = vIsocenterIndex;
	vRayTraceMaxIndex[X] += isocenterToTopScale * GetRayTraceMax()[0] / spacing[X];
	vRayTraceMaxIndex[Y] += isocenterToTopScale * GetRayTraceMax()[1] / spacing[Y];

	// now set starting z-coord (at upper boundary of voxels)
	vRayTraceMinIndex[Z] = vRayTraceMaxIndex[Z] = rayTraceStartZ;

	// initialize surface integral of fluence 
	SetEnergySurfaceIntegral(0.0);

	// get the output terma volume and initialize to zeroes
	VolumeReal::Pointer pTerma = GetOutput();
	pTerma->FillBuffer(0.0);

	// calculate change in ray for each step
	const Real deltaRay = 1.0 / GetRaysPerVoxel();

	// iterate over X & Y voxel positions
	ContinuousIndex<Real,3> vX = vRayTraceMinIndex;
	while (vX[X] < vRayTraceMaxIndex[X])
	{
		ContinuousIndex<Real,3> vY = vX;
		while (vY[Y] < vRayTraceMaxIndex[Y])
		{
			// calculate unit ray direction vector (voxel coordinates)
			Vector<Real,3> vDir = vY - vSourceIndex;
			vDir.Normalize();

			// initial emergy per ray calculated based on fluence (= initial area of each ray)
			const Real energy0 = spacing[X] * spacing[Y] * deltaRay * deltaRay;
			TraceRayTerma(vY, vDir, energy0);

			vY[Y] += deltaRay;
		}
		vX[X] += deltaRay;
	}	
}

///////////////////////////////////////////////////////////////////////////////
inline Real 
	MinDistToIntersectPlan(const ContinuousIndex<Real,3>& vRay,
			const Vector<Real>& vDir,
			VolumeReal::IndexType& index)
	// Helper function to compute min distance of a vector component to interset the
	//	next plane at a 0.5-boundary
{
	const Real EPS = (Real) 1e-6;
	Real minDist = 1e+6;

	for (int nDim = 0; nDim <= 2; nDim++)
	{
		if (vDir[nDim] > 0)
		{
			index[nDim] = (int) floor(vRay[nDim] + 0.5 + EPS);
			minDist = __min(minDist, 
				((Real(index[nDim]) + 0.5) - vRay[nDim]) / vDir[nDim]);
		}
		else
		{
			index[nDim] = (int) ceil(vRay[nDim] - 0.5 - EPS);
			minDist = __min(minDist, 
				((Real(index[nDim]) - 0.5) - vRay[nDim]) / vDir[nDim]);
		}
	}

	return minDist;
}

//////////////////////////////////////////////////////////////////////////////
void 
	TermaCalculator::TraceRayTerma(ContinuousIndex<Real,3> vRay, const Vector<Real>& vDir, 
						const Real energy0)
{
	VolumeReal::ConstPointer pDensity = GetInput();
	VolumeReal::Pointer pTerma = GetOutput();

	// calc physical length of vDir, in cm!!! (not mm)
	const Real dirLength = GetPhysicalLength(pTerma->GetSpacing(), vDir);

	// stores mu
	const Real mu = Get_mu();

	// initialize radiological path length for raytrace
	Real path = 0.0;

	// increment by incident fluence
	m_EnergySurfaceIntegral += energy0;

	// stores current voxel indices
	VolumeReal::IndexType index;

	// compute initial min distance for the ray
	Real minDist = MinDistToIntersectPlan(vRay, vDir, index);

	// iterate ray trace until volume boundary is reached 
	// TODO: with index as an IndexType, is there a function to directly test for containment?  
	//		use ShrunkRegion (see IsocurveOverlay)
	while (index[X] >= 1 
		&& index[X] < pTerma->GetBufferedRegion().GetSize()[1]-1
		&& index[Y] >= 1 
		&& index[Y] < pTerma->GetBufferedRegion().GetSize()[2]-1
		&& index[Z] < pTerma->GetBufferedRegion().GetSize()[0]-1)
	{
		// compute avg position of ray within voxel
		//		use this position for trilinear weights
		ContinuousIndex<Real,3> vPos = vRay;
		vPos += ((Real) (0.5 * minDist)) * vDir;

		// compute tri-linear interpolation weights
		Real weights[3][3];

		// interpolate the deltaPath from the density, storing trilinear weights for later use
		Real deltaPath = TrilinearInterpDensity(pDensity, vPos, index, weights);

		// update radiological path for length of partial volume traversal
		deltaPath *= minDist * dirLength;

		// update cumulative path
		path += deltaPath;

		// add to terma = -(derivative of energy along ray)	
		const Real energyIncrement = energy0 * exp(-mu * path) * mu * deltaPath;

		// update the neighborhood terma voxels using the previously calculated trilinear weights
		UpdateTermaNeighborhood(pTerma, index, weights, energyIncrement);

		// update current ray position to move to the given plane intersection
		vRay += minDist * vDir;

		// find next min distance
		minDist = MinDistToIntersectPlan(vRay, vDir, index);

	}	// while

	// reduce by amount of remaining (un-attenuated) fluence exiting volume
	m_EnergySurfaceIntegral -= energy0 * exp(-mu * path) ;
}

///////////////////////////////////////////////////////////////////////////////
Real 
	TermaCalculator::GetPhysicalLength(const VolumeReal::SpacingType& spacing, 
									   const Vector<Real>& vDir)
	// Helper function to calculate physical length of a normalized direction
	//		vector, based on the pixel spacing
	// NOTE: Returned value is in CM, not MM!
{
	Vector<Real> vDirPhysical = vDir;
	for (int n = 0; n < 2; n++)
		vDirPhysical[n] *= spacing[n];
	return 0.1 * vDirPhysical.GetNorm();
}

///////////////////////////////////////////////////////////////////////////////
Real 
	TermaCalculator::TrilinearInterpDensity(const VolumeReal *pDensity,
											const ContinuousIndex<Real,3>& vPos, 
											const VolumeReal::IndexType& index, 
											Real (&weights)[3][3])
	// Helper function to trilinear interpolate the density rep at the given 
	//		position
{
	// calculate trilinear weights
	for (int nDim = 0; nDim < 3; nDim++)
	{
		weights[nDim][-1 + 1] = vPos[nDim] < (Real) index[nDim] 
			? fabs((Real) index[nDim] - vPos[nDim]) : 0.0;
		weights[nDim][ 0 + 1] = 1.0 - fabs((Real) index[nDim] - vPos[nDim]);
		weights[nDim][ 1 + 1] = vPos[nDim] > (Real) index[nDim] 
			? fabs((Real) index[nDim] - vPos[nDim]) : 0.0;
	}

	// delta path = radiological path (based on mass density) through voxel
	Real density = 0.0;	
	VolumeReal::OffsetType offset;
	for (offset[Z] = -1; offset[Z] <= 1; offset[Z]++)
	{
		for (offset[Y] = -1; offset[Y] <= 1; offset[Y]++)
		{
			for (offset[X] = -1; offset[X] <= 1; offset[X]++)
			{
				density += 
					weights[X][offset[X]+1] 
					* weights[Y][offset[Y]+1] 
					* weights[Z][offset[Z]+1] 
					* pDensity->GetPixel(index+offset);
			}
		}
	}

	return density;
}

///////////////////////////////////////////////////////////////////////////////
void
	TermaCalculator::UpdateTermaNeighborhood(VolumeReal *pTerma,
											 const VolumeReal::IndexType& index, 
											 Real (&weights)[3][3], Real value)
	// Helper function to trilinear interpolate the density rep at the given 
	//		position
{
	// iterate over neighborhood
	VolumeReal::OffsetType offset;
	for (offset[Z] = -1; offset[Z] <= 1; offset[Z]++)
	{
		for (offset[Y] = -1; offset[Y] <= 1; offset[Y]++)
		{
			for (offset[X] = -1; offset[X] <= 1; offset[X]++)
			{
				//	use trilinear interpolation weights to update all neighboring 
				//		terma voxels
				pTerma->GetPixel(index+offset) += (VoxelReal)
					( weights[X][offset[X]+1] 
					* weights[Y][offset[Y]+1] 
					* weights[Z][offset[Z]+1] 
					* value );
			}
		}
	}
}

}