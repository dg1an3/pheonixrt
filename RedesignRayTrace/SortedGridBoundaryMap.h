#pragma once

//////////////////////////////////////////////////////////////////////////////
template<typename TCoordRep, unsigned int VDim>
class SortedGridBoundaryMap :
	public std::map<TCoordRep, ContinuousIndex<TCoordRep,VDim> >
{
public:
	// calculates the boundary from a given start index, with a given
	//		direction vector, up to maximum length
	SortedGridBoundaryMap(ContinuousIndex<TCoordRep,VDim> vStart, 
		Vector<TCoordRep,VDim> vDir, 
		const ImageBase<VDim> *pGeomImage, 
		TCoordRep lengthMax = -1);
};

//////////////////////////////////////////////////////////////////////////////
template<typename TCoordRep>
inline TCoordRep 
	StepSize(TCoordRep at, TCoordRep dir)
		// calculates the step size in 1-D at a given point to move to the next
		//		integral boundary, given the sign of the direction of movement
{
	if (dir > 0.0)
		return (floor(at + 0.5) - 0.5 + 1.0) - at;
	else
		return (ceil (at - 0.5) + 0.5 - 1.0) - at;
}

//////////////////////////////////////////////////////////////////////////////
template<typename TCoordRep, unsigned int VDim>
SortedGridBoundaryMap<TCoordRep,VDim>::SortedGridBoundaryMap(
																		ContinuousIndex<TCoordRep,VDim> vStart, 
 																	  Vector<TCoordRep,VDim> vDir, 
																	  const ImageBase<VDim> *pGeomImage,
																		TCoordRep lengthMax)
		 // calculates the boundary from a given start index, with a given
		 //		direction vector, up to maximum length
{
	// only works correctly for normalized direction vector
	vDir.Normalize();

	// get the spacing to use for physical length calculation
	ImageBase<VDim>::SpacingType lengthScale = pGeomImage->GetSpacing();

	// calculate max length from size of image
	if (lengthMax < 0.0)
	{
		ImageBase<VDim>::SizeType size = pGeomImage->GetLargestPossibleRegion().GetSize();
		for (int nD = 0; nD < VDim; nD++)
		{
			lengthMax = __max(lengthMax, lengthScale[nD] * size[nD]);
		}
	}

	for (int nD = 0; nD < VDim; nD++)
	{
		if (fabs(vDir[nD]) < 1e-6)
			continue;

		// track the current indiex position
		ContinuousIndex<TCoordRep,VDim> vNext = vStart;

		TCoordRep length = 0.0;
		while (length < lengthMax)
		{
			// this is the length of the vDir vector that needs to be added to 
			//		the index to move to the next integral boundary
			TCoordRep lengthStepIndex = StepSize(vNext[nD], vDir[nD]) / vDir[nD];

			// compute the updated total physical length
			length += lengthScale[nD] * lengthStepIndex;

			// move to the next boundary and store
			vNext += lengthStepIndex * vDir;
			(*this)[length] = vNext;
		}
	}
}
