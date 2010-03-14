#pragma once

#include <Dib.h>

#include <itkRGBAPixel.h>
#include <itkImageToImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkColorTable.h>

using namespace itk;

namespace dH {

typedef RGBAPixel< unsigned char > RGBAPixelType;
typedef Image<RGBAPixelType, 3> RGBAImageType;

//////////////////////////////////////////////////////////////////////////////
class DisplayBlendFilter
	: public ImageToImageFilter<VolumeChar, RGBAImageType>
{
public:
	DisplayBlendFilter(void);
	~DisplayBlendFilter(void);

	// itk typedefs
	typedef DisplayBlendFilter Self;
	typedef ImageToImageFilter<VolumeChar, RGBAImageType> Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	DeclareMember(Alpha, REAL);
	DeclareMemberSPtr(PseudoColorTable, ColorTable<unsigned char>);

	// helper to set pseudocolor from a loaded DIB
	void SetPseudoColorTableFromDib(CDib& dib);

	// generates the data on update
	virtual void GenerateData();
};

}
