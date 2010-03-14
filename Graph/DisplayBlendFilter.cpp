#include "StdAfx.h"
#include "DisplayBlendFilter.h"

namespace dH {

//////////////////////////////////////////////////////////////////////////////
DisplayBlendFilter::DisplayBlendFilter(void)
{
}

//////////////////////////////////////////////////////////////////////////////
DisplayBlendFilter::~DisplayBlendFilter(void)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
	DisplayBlendFilter::SetPseudoColorTableFromDib(CDib& dibColormap)
{
	CSize size = dibColormap.GetSize();

	CArray<UCHAR, UCHAR&> arrRaw;
	arrRaw.SetSize(size.cx * size.cy * 3);
	dibColormap.GetBitmapBits(size.cx * size.cy * 3, arrRaw.GetData());

	itk::ColorTable<unsigned char>::Pointer pColormap = itk::ColorTable<unsigned char>::New();
	pColormap->UseGrayColors(256);

	for (int nAt = 0; nAt < pColormap->GetNumberOfColors(); nAt++)
	{
		// compute closest color
		int nAtRaw = nAt * size.cy / 256;
		nAtRaw *= (3 * size.cx);	// adjust for width 
		pColormap->SetColor(nAt, arrRaw[nAtRaw], arrRaw[nAtRaw+1], arrRaw[nAtRaw+2], "c");
	}

	SetPseudoColorTable(pColormap);
}

//////////////////////////////////////////////////////////////////////////////
void 
	DisplayBlendFilter::GenerateData()
{
	// Call a method that can be overriden by a subclass to allocate
	// memory for the filter's outputs
	this->AllocateOutputs();

	VolumeChar::ConstPointer pInput0 = GetInput(0);
	VolumeChar::ConstPointer pInput1 = GetInput(1);
	const VolumeChar::RegionType& inRegion = pInput0->GetBufferedRegion();

	RGBAImageType::Pointer pOutput = GetOutput();
	const RGBAImageType::RegionType& outRegion = pOutput->GetBufferedRegion();

	// stores alpha1 value
	const REAL alpha1 = 1.0 - GetAlpha();

	ImageRegionConstIterator<VolumeChar> iterInput0(pInput0, inRegion);
	ImageRegionConstIterator<VolumeChar> iterInput1(pInput1, inRegion);
	ImageRegionIterator<RGBAImageType> iterOutput(pOutput, outRegion);

	// no pseudocolor table?
	if (GetPseudoColorTable() == NULL)
	{
		while (!iterInput0.IsAtEnd())
		{
			// set to blended
			iterOutput.Set(RGBAPixelType(GetAlpha() * iterInput0.Value() 
				+ alpha1 * iterInput1.Value()));

			++iterInput0;
			++iterInput1;
			++iterOutput;
		}

		// done
		return;
	}

	// perform blending
	while (!iterInput0.IsAtEnd())
	{
		if (iterInput1.Value() < 5)		// check if we are in transparent part of color table
		{
			// just set to grayscale value
			iterOutput.Set(RGBAPixelType(iterInput0.Value()));
		}
		else
		{
			int colorIndex = __min(iterInput1.Value(), 
				GetPseudoColorTable()->GetNumberOfColors()-1);
			const RGBPixel<UCHAR> * color1 = GetPseudoColorTable()->GetColor(colorIndex);

			dH::RGBAPixelType blendPixel;
			blendPixel.SetRed(GetAlpha() * iterInput0.Value() + alpha1 * color1->GetRed());
			blendPixel.SetGreen(GetAlpha() * iterInput0.Value() + alpha1 * color1->GetGreen());
			blendPixel.SetBlue(GetAlpha() * iterInput0.Value() + alpha1 * color1->GetBlue());
			iterOutput.Set(blendPixel);
		}

		++iterInput0;
		++iterInput1;
		++iterOutput;
	}
}

}