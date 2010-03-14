#include "StdAfx.h"
#include "MultiMaskNegatedImageFilter.h"

namespace dH {

//////////////////////////////////////////////////////////////////////////////
MultiMaskNegatedImageFilter::MultiMaskNegatedImageFilter(void)
{
}

//////////////////////////////////////////////////////////////////////////////
MultiMaskNegatedImageFilter::~MultiMaskNegatedImageFilter(void)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
	MultiMaskNegatedImageFilter::SetMaskEnabled(int nMask, bool bEnabled)	
	// can be used to turn on / off individual masks
{
	while (m_arrMaskEnabled.size() < nMask+1)
		m_arrMaskEnabled.push_back(false);
	m_arrMaskEnabled[nMask] = bEnabled;
}

//////////////////////////////////////////////////////////////////////////////
void 
	MultiMaskNegatedImageFilter::GenerateData()
	// generates the data on update
{
	VolumeReal::ConstPointer pInputVolume = GetInput(0);
	VolumeReal::Pointer pOutputVolume = GetOutput();

	CopyImage<VOXEL_REAL, 3>(pInputVolume, pOutputVolume);

	// now exclude
	for (int nAt = 0; nAt < GetNumberOfInputs(); nAt++)
	{
		if (m_arrMaskEnabled[nAt])
		{
			VolumeReal::ConstPointer pInputMask = GetInput(nAt+1);
			ConstVolumeRealIterator iterMask(pInputMask, pInputMask->GetBufferedRegion());

			VolumeRealIterator iter(pOutputVolume, pOutputVolume->GetBufferedRegion());
			for (iter.GoToBegin(), iterMask.GoToBegin(); !iterMask.IsAtEnd();
					++iter, ++iterMask)
			{
				if (iterMask.Value() > 0.0)
					iter.Set(0.0);
			}
		}
	}
}

}