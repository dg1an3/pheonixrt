#pragma once

#include <itkImageToImageFilter.h>

using namespace itk;

namespace dH {

// automatically sets output to conform to input 0

class MultiMaskNegatedImageFilter :
	public ImageToImageFilter<VolumeReal, VolumeReal>
{
public:
	MultiMaskNegatedImageFilter(void);
	~MultiMaskNegatedImageFilter(void);

	// itk typedefs
	typedef MultiMaskNegatedImageFilter Self;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// can be used to turn on / off individual masks
	void SetMaskEnabled(int nMask, bool bEnabled = true);

	// generates the data on update
  virtual void GenerateData();

private:
	std::vector< bool > m_arrMaskEnabled;
};

}