#include "StdAfx.h"
#include "IntensityMapAccumulateImageFilter.h"

namespace dH {

//////////////////////////////////////////////////////////////////////////////
IntensityMapAccumulateImageFilter::IntensityMapAccumulateImageFilter(void)
{
	SetBasisGroup(BasisGroupType::New());
	m_pBuffer = VolumeReal::New();
}

//////////////////////////////////////////////////////////////////////////////
IntensityMapAccumulateImageFilter::~IntensityMapAccumulateImageFilter(void)
{
	// this releases any references on the basis volumes
	InitBasisGroupGeometry();

	// this releases the inputs
	SetBasisGroupAsInput(false);
}


//////////////////////////////////////////////////////////////////////////////
void 
	IntensityMapAccumulateImageFilter::InitBasisGroupGeometry()
{
	// need to call unregister for all existing basis volumes
	m_arrBasisVolumes.clear();
	//ImageRegionIterator< BasisGroupType > iter(GetBasisGroup(), GetBasisGroup()->GetBufferedRegion());
	//for (; !iter.IsAtEnd(); ++iter)
	//{
	//	if (iter.Get() != NULL)
	//	{
	//		iter.Get()->UnRegister();
	//		iter.Set(NULL);
	//	}
	//}

	// set the new size for the buffer
	ConformTo<VolumeReal *, 2>(GetInput(), GetBasisGroup());

	// make sure we are filled with NULLs
	GetBasisGroup()->FillBuffer(NULL);

	// now create the basis volumes
	ImageRegionIterator< BasisGroupType > iter(GetBasisGroup(), GetBasisGroup()->GetBufferedRegion());
	for (; !iter.IsAtEnd(); ++iter)
	{
		VolumeReal::Pointer pBasisVolume = VolumeReal::New();
		iter.Set(pBasisVolume);
		m_arrBasisVolumes.push_back(pBasisVolume);
	}
}

//////////////////////////////////////////////////////////////////////////////
void 
	IntensityMapAccumulateImageFilter::SetBasisGroupAsInput(bool bEnable)
{
	BasisGroupType::IndexType index;
	index.Fill(0);
	if (bEnable)
	{
		for (; index[0] < GetBasisGroup()->GetBufferedRegion().GetSize()[0]; index[0]++)
		{
			VolumeReal * pBasisVolume = GetBasisGroup()->GetPixel(index);
			this->SetNthInput(index[0]+1, pBasisVolume);
		}
	}
	else
	{
		for (; index[0] < GetBasisGroup()->GetBufferedRegion().GetSize()[0]; index[0]++)
		{
			VolumeReal * pBasisVolume = GetBasisGroup()->GetPixel(index);
			this->RemoveInput(pBasisVolume);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
void 
	IntensityMapAccumulateImageFilter::GenerateData()
	// generates the data on update
{
	// Call a method that can be overriden by a subclass to allocate
	// memory for the filter's outputs
	this->AllocateOutputs();

	// get pointers to the input and output
	OutputImagePointer outputPtr = this->GetOutput();

	// clear voxels for accumulation
	outputPtr->FillBuffer(0.0);

	// set up the accumulation buffer
	m_pBuffer->SetRegions(outputPtr->GetLargestPossibleRegion());
	m_pBuffer->Allocate();

	// store the update region (for all volumes)
	const VolumeReal::RegionType& updateRegion = outputPtr->GetBufferedRegion();

	// iterators for the intensity map and basis volume map
	ImageRegionConstIterator<IntensityMapType> iterIntensity(GetInput(), GetInput()->GetBufferedRegion());
	for (iterIntensity.GoToBegin();	!iterIntensity.IsAtEnd(); ++iterIntensity)
	{
		// pointer to current basis volume
		const VolumeReal * pBasisVolume = GetBasisGroup()->GetPixel(iterIntensity.GetIndex());
		if (!pBasisVolume
			|| pBasisVolume->GetPixelContainer()->Size() == 0)
			continue;

		// index to iterate through the planes
		VolumeReal::IndexType index = updateRegion.GetIndex();
		for (; index[2] < updateRegion.GetSize()[2]; index[2]++)
		{
			// multiple basis volume by intensity value
			CK_IPP( ippiMulC_32f_C1R(
				&pBasisVolume->GetPixel(index), Stride<VOXEL_REAL, 3>(pBasisVolume), 
				iterIntensity.Value(),
				&m_pBuffer->GetPixel(index), Stride<VOXEL_REAL, 3>(m_pBuffer), 
				MakeIppiSize(updateRegion)));

			// add scaled basis volume to the output volume
			CK_IPP( ippiAdd_32f_C1IR(
				&m_pBuffer->GetPixel(index), Stride<VOXEL_REAL, 3>(m_pBuffer), 
				&outputPtr->GetPixel(index), Stride<VOXEL_REAL, 3>(outputPtr), 
				MakeIppiSize(updateRegion)));
		}
	}

	// need to explicitly set modified on output??? or maybe not
	outputPtr->Modified();
}

}