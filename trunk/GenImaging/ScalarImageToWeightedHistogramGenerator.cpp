//
//
#include "StdAfx.h"

#include <ScalarImageToWeightedHistogramGenerator.h>


namespace dH {

//////////////////////////////////////////////////////////////////////////////
ScalarImageToWeightedHistogramGenerator
	::ScalarImageToWeightedHistogramGenerator() 
{
	m_ImageToListAdaptor = AdaptorType::New();
	m_WeightImageToListAdaptor = AdaptorType::New();
	m_HistogramGenerator = GeneratorType::New();
	m_HistogramGenerator->SetListSample( m_ImageToListAdaptor );
	m_HistogramGenerator->SetWeights( m_WeightImageToListAdaptor );
}

//////////////////////////////////////////////////////////////////////////////
void
	ScalarImageToWeightedHistogramGenerator::SetInput( const ImageType * image ) 
{
	m_ImageToListAdaptor->SetImage( image );
}

//////////////////////////////////////////////////////////////////////////////
void 
	ScalarImageToWeightedHistogramGenerator::SetWeightImage(const ImageType * weightImage)
{
	m_WeightImageToListAdaptor->SetImage(weightImage);
}

//////////////////////////////////////////////////////////////////////////////
const ScalarImageToWeightedHistogramGenerator::HistogramType *
	ScalarImageToWeightedHistogramGenerator
	::GetOutput() const
{
	return m_HistogramGenerator->GetOutput();
}

//////////////////////////////////////////////////////////////////////////////
void
	ScalarImageToWeightedHistogramGenerator
	::Compute() 
{
	m_HistogramGenerator->Update();
}

//////////////////////////////////////////////////////////////////////////////
void
	ScalarImageToWeightedHistogramGenerator
	::SetNumberOfBins( unsigned int numberOfBins ) 
{
	HistogramType::SizeType size;
	size.Fill( numberOfBins );
	m_HistogramGenerator->SetNumberOfBins( size );
}

//////////////////////////////////////////////////////////////////////////////
void
	ScalarImageToWeightedHistogramGenerator
	::SetHistogramMin( RealPixelType minimumValue ) 
{
	typedef GeneratorType::MeasurementVectorType     MeasurementVectorType;
	MeasurementVectorType minVector;
	minVector[0] = minimumValue;
	m_HistogramGenerator->SetHistogramMin( minVector );
}

//////////////////////////////////////////////////////////////////////////////
void
	ScalarImageToWeightedHistogramGenerator
	::SetHistogramMax( RealPixelType maximumValue ) 
{
	typedef GeneratorType::MeasurementVectorType     MeasurementVectorType;
	MeasurementVectorType maxVector;
	maxVector[0] = maximumValue;
	m_HistogramGenerator->SetHistogramMax( maxVector );
}

//////////////////////////////////////////////////////////////////////////////
void
	ScalarImageToWeightedHistogramGenerator
	::SetMarginalScale( double marginalScale )
{
	m_HistogramGenerator->SetMarginalScale( marginalScale );
}

//////////////////////////////////////////////////////////////////////////////
void
	ScalarImageToWeightedHistogramGenerator
	::PrintSelf(std::ostream& os, Indent indent) const
{
	Superclass::PrintSelf(os,indent);
	os << "ImageToListSample adaptor = " << m_ImageToListAdaptor << std::endl;
	os << "HistogramGenerator = " << m_HistogramGenerator << std::endl;
}

}


