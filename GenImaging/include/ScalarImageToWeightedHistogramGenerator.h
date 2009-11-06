//
//
#pragma once

#include <itkScalarImageToListAdaptor.h>
#include <itkObject.h>
using namespace itk;
using namespace itk::Statistics;

#include <WeightedListSampleToHistogramGenerator.h>

BeginNamespace(dH)

//////////////////////////////////////////////////////////////////////////////
class ScalarImageToWeightedHistogramGenerator : 
		public Object
{
public:
	// Standard typedefs 
	typedef ScalarImageToWeightedHistogramGenerator Self;
	typedef Object Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// Run-time type information (and related methods). 
	itkTypeMacro(ScalarImageToWeightedHistogramGenerator, Object);

	// standard New() method support 
	itkNewMacro(Self);

	// typedefs for templates
	typedef VolumeReal ImageType;
	typedef ScalarImageToListAdaptor<ImageType> AdaptorType;
	typedef AdaptorType::Pointer AdaptorPointer;
	typedef ImageType::PixelType PixelType;
	typedef NumericTraits<PixelType>::RealType RealPixelType;

	// use the weighted generator
	typedef dH::WeightedListSampleToHistogramGenerator GeneratorType;
	typedef GeneratorType::Pointer GeneratorPointer;

	// typedefs for the generator
	typedef GeneratorType::HistogramType HistogramType;
	typedef HistogramType::Pointer HistogramPointer;
	typedef HistogramType::ConstPointer HistogramConstPointer;

public:

	// Triggers the Computation of the histogram 
	void Compute( void );

	// Connects the input image for which the histogram is going to be computed 
	void SetInput( const ImageType * );

	// sets the image to use as the weight mask
	void SetWeightImage( const ImageType * );

	// Return the histogram. 
	// \warning This output is only valid after the Compute() method has been invoked 
	// \sa Compute 
	const HistogramType * GetOutput() const;

	// Set number of histogram bins 
	void SetNumberOfBins( unsigned int numberOfBins );

	// Set marginal scale value to be passed to the histogram generator 
	void SetMarginalScale( double marginalScale );

	// Set the minimum value from which the bins will be computed 
	void SetHistogramMin( RealPixelType minimumValue );

	// Set the maximum value from which the bins will be computed 
	void SetHistogramMax( RealPixelType maximumValue );

protected:
	ScalarImageToWeightedHistogramGenerator();
	virtual ~ScalarImageToWeightedHistogramGenerator() {};
	void PrintSelf(std::ostream& os, Indent indent) const;

private:
	AdaptorPointer m_ImageToListAdaptor;
	AdaptorPointer m_WeightImageToListAdaptor;
	GeneratorPointer m_HistogramGenerator;

	ScalarImageToWeightedHistogramGenerator(const Self&); //purposely not implemented
	void operator=(const Self&); //purposely not implemented
};


EndNamespace(dH)
