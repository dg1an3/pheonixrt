#include "StdAfx.h"
#include "TermaCalculator.h"

#include "SortedGridBoundaryMap.h"

///////////////////////////////////////////////////////////////////////////
TermaCalculator::TermaCalculator(void)
{
}

///////////////////////////////////////////////////////////////////////////
TermaCalculator::FluenceMapType *
	TermaCalculator::GetFluenceMap() 
{ 
	return (FluenceMapType *) GetInput(0); 
}

///////////////////////////////////////////////////////////////////////////
void 
TermaCalculator::SetFluenceMap(TermaCalculator::FluenceMapType *pFM) 
{ 
	SetNthInput(0, pFM); 
}

///////////////////////////////////////////////////////////////////////////
VolumeReal *
TermaCalculator::GetDensity() 
{ 
	return (VolumeReal *) GetInput(1); 
}

///////////////////////////////////////////////////////////////////////////
void 
TermaCalculator::SetDensity(VolumeReal *pFM) 
{ 
	SetNthInput(1, pFM); 
}

///////////////////////////////////////////////////////////////////////////
void 
TermaCalculator::GenerateData()
{
	AllocateOutputs();

	// calculate offset vector from source to fluence plane
	Vector<double,3> vSourceFluenceOffset = GetIsocenter() - GetSource();
	vSourceFluenceOffset.Normalize();
	vSourceFluenceOffset *= GetSourceFluenceDistance();

	// calculate position of central axis on fluence plane
	VolumeReal::PointType vFluencePlaneCAX = GetSource() + vSourceFluenceOffset;

	// iterate over fluence map
	ImageRegionConstIterator<FluenceMapType> iter(GetFluenceMap(), GetFluenceMap()->GetBufferedRegion());
	for (; !iter.IsAtEnd(); ++iter)
	{
		// determine ray starting point - begin at the central axis
		VolumeReal::PointType vStart = vFluencePlaneCAX;

		// and offset in the map x- and y-directions to the final ray starting point
		vStart += (GetDensity()->GetDirection() * MakeVector<3>(0.0, GetFluenceMap()->GetSpacing()[0], 0.0)) 
			* iter.GetIndex()[0];
		vStart += (GetDensity()->GetDirection() * MakeVector<3>(0.0, 0.0, GetFluenceMap()->GetSpacing()[1]))
			* iter.GetIndex()[1];

		// convert initial fluence to energy
		double energy0 = iter.Value() 
			* GetFluenceMap()->GetSpacing()[0] * GetFluenceMap()->GetSpacing()[1];

		// and perform the ray trace
		TraceRayTerma(vStart, energy0);
	}
}


//////////////////////////////////////////////////////////////////////////////
void 
TermaCalculator::TraceRayTerma(VolumeReal::PointType vStart, double energy0)
{
	ContinuousIndex<double,3> vStartIndex;
	GetDensity()->TransformPhysicalPointToContinuousIndex(vStart, vStartIndex);

	ContinuousIndex<double,3> vSourceIndex;
	GetDensity()->TransformPhysicalPointToContinuousIndex(GetSource(), vSourceIndex);

	// calculate the normalized direction vector, in index coordinates
	Vector<double,3> vDirIndex = vStartIndex - vSourceIndex;
	vDirIndex.Normalize();

	// form the sorted coordinate boundary map based on initial ray position and direction 
	//			(both in index coordinates)
	SortedGridBoundaryMap<double,3> boundaryMap(vStartIndex, vDirIndex, GetDensity());

	// stores the accumulated radiological path length
	double radiologicalPath = 0.0;

	// create the density interpolator
	LinearInterpolateImageFunction<VolumeReal>::Pointer pInterpolator =
		LinearInterpolateImageFunction<VolumeReal>::New();
	pInterpolator->SetInputImage(GetDensity());

	// store values of physical length and index coordinate from previous entry 
	//		in boundary map
	ContinuousIndex<double,3> vPrevIndex = vStartIndex;
	double prevLength = 0.0;

	SortedGridBoundaryMap<double,3>::iterator iter = boundaryMap.begin();
	for (; iter != boundaryMap.end(); ++iter)
	{
		// calculate mid point (for determining discrete index in to density)
		ContinuousIndex<double,3> vDensityIndex;
		vDensityIndex.SetToMidPoint(vPrevIndex, iter->second);

		// form density index and see if it is inside the density
		if (!pInterpolator->IsInsideBuffer(vDensityIndex))
			continue;

		// calculate change in physical length
		const double deltaLength = iter->first - prevLength;

		// change in radiological path length is change in physical length
		//		scaled by mass density
		const double deltaRadiologicalPath = deltaLength 
			* pInterpolator->EvaluateAtContinuousIndex(vDensityIndex);
		radiologicalPath += deltaRadiologicalPath;

		// compute change in energy and 
		const double deltaEnergy = energy0 * exp(-Get_mu() * radiologicalPath) 
			* Get_mu() * deltaRadiologicalPath;

		// set the TERMA pixel values surrounding the current point
		InterpolateTermaOut(vDensityIndex, deltaEnergy);

		// update values for next iteration
		vPrevIndex = iter->second;
		prevLength = iter->first;
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	TermaCalculator::InterpolateTermaOut(const ContinuousIndex<double,3>& vIndex, 
																	double deltaEnergy)
{
	// Compute base index = closet index below point and 
	//		distance from point to base index
	VolumeReal::IndexType baseIndex;
	Vector<double,3> distance;
	for(int nD = 0; nD < VolumeReal::ImageDimension; nD++ )
	{
		baseIndex[nD] = (int) vcl_floor(vIndex[nD] );
		distance[nD] = vIndex[nD] - double(baseIndex[nD]);
	}

	// Interpolated value is the weighted sum of each of the surrounding
	// neighbors. The weight for each neighbor is the fraction overlap
	// of the neighbor pixel with respect to a pixel centered on point.
	double value = 0.0;
	double totalOverlap = 0.0;

	int nNeighbors = 1 << VolumeReal::ImageDimension;
	for (int counter = 0; counter < nNeighbors; counter++ )
	{
		double overlap = 1.0;          // fraction overlap
		int upper = counter;					// each bit indicates upper/lower neighbour
		VolumeReal::IndexType neighIndex;

		// get neighbor index and overlap fraction
		for (int nD = 0; nD < VolumeReal::ImageDimension; nD++)
		{
			if (upper & 1)
			{
				neighIndex[nD] = baseIndex[nD] + 1;
				overlap *= distance[nD];
			}
			else
			{
				neighIndex[nD] = baseIndex[nD];
				overlap *= 1.0 - distance[nD];
			}

			upper >>= 1;
		}

		// set interpolated value only if overlap is not zero
		if (overlap > 0.0)
		{
			GetOutput()->GetPixel(neighIndex) += overlap * deltaEnergy;
			totalOverlap += overlap;
		}

		if (totalOverlap == 1.0)
		{			
			break;	// finished
		}
	}
}
