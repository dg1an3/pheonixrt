//
//
#pragma once

#include <itkListSampleToHistogramGenerator.h>
#include <itkScalarImageToListAdaptor.h>
using namespace itk;
using namespace itk::Statistics;

BeginNamespace(dH)

typedef ListSampleToHistogramGenerator< 
ScalarImageToListAdaptor<VolumeReal>, 
VoxelReal, DenseFrequencyContainer > VolumeRealHistogramGenerator;

//////////////////////////////////////////////////////////////////////////////
class WeightedListSampleToHistogramGenerator :
	public VolumeRealHistogramGenerator
{
public:
	// standard defs
	typedef WeightedListSampleToHistogramGenerator Self;
	typedef VolumeRealHistogramGenerator Superclass;
	typedef SmartPointer<Self>   Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// type macro
	itkTypeMacro(WeightedListSampleToHistogramGenerator, VolumeRealHistogramGenerator);

	// Method for creation through the object factory.
	itkNewMacro(Self);

	// Type needed for defining the limits of the histogram bins
	typedef VoxelReal THistogramMeasurement;
	typedef ScalarImageToListAdaptor<VolumeReal> TListSample;

	// plug in the ListSample object 
	void SetListSample(const TListSample* list)
	{ 
		// Throw exception if the length of measurement vectors in the list is not
		// equal to the dimension of the histogram.
		if( list->GetMeasurementVectorSize() != MeasurementVectorSize )
		{
			itkExceptionMacro(<< "Length of measurement vectors in the list sample is "
				<< list->GetMeasurementVectorSize() << " but histogram dimension is "
				<< MeasurementVectorSize);
		}
		m_List = list ; 
	}

	// plug in the ListSample weight mask
	void SetWeights(const TListSample* weightList)
	{ 
		// Throw exception if the length of measurement vectors in the list is not
		// equal to the dimension of the histogram.
		if( weightList->GetMeasurementVectorSize() != MeasurementVectorSize )
		{
			itkExceptionMacro(<< "Length of measurement vectors in the list sample is "
				<< weightList->GetMeasurementVectorSize() << " but histogram dimension is "
				<< MeasurementVectorSize);
		}
		m_WeightList = weightList ; 
	}

	void SetMarginalScale(float scale)
	{ m_MarginalScale = scale ; }

	void SetNumberOfBins(HistogramSizeType sizes)
	{ m_Sizes = sizes ; }

	const HistogramType* GetOutput() const
	{ return m_Histogram ; }

	void Update() 
	{ this->GenerateData() ; }

	itkSetMacro(AutoMinMax,bool);
	itkGetConstReferenceMacro(AutoMinMax,bool);

	void SetHistogramMin(const MeasurementVectorType & histogramMin)
	{
		m_HistogramMin = histogramMin;
		m_AutoMinMax = false;

		// Sanity check.. Check to see that container m_HistogramMin has the same
		// length as the length of measurement vectors in the list sample. And the
		// same length as the container over which the list sample is instantiated,
		// if fixed.
		MeasurementVectorTraits::Assert(m_HistogramMin, MeasurementVectorSize, 
			"Length Mismatch: ListSampleToHistogramGenerator::SetHistogramMin");
		if( m_List )
		{
			if( m_List->GetMeasurementVectorSize() != MeasurementVectorSize )
			{
				itkExceptionMacro( << "Length Mismatch: ListSampleToHistogramGenerator::SetHistogramMin" );
			}
		}
	}

	void SetHistogramMax(const MeasurementVectorType & histogramMax)
	{
		m_HistogramMax = histogramMax;
		m_AutoMinMax = false;

		// Sanity check.. Check to see that container m_HistogramMax has the same
		// length as the length of measurement vectors in the list sample. And the
		// same length as the container over which the list sample is instantiated,
		// if fixed.
		MeasurementVectorTraits::Assert(m_HistogramMax, MeasurementVectorSize, 
			"Length Mismatch: ListSampleToHistogramGenerator::SetHistogramMin");
		if( m_List )
		{
			if( m_List->GetMeasurementVectorSize() != MeasurementVectorSize )
			{
				itkExceptionMacro( << "Length Mismatch: ListSampleToHistogramGenerator::SetHistogramMin" );
			}
		}
	}

protected:
	WeightedListSampleToHistogramGenerator() ;
	virtual ~WeightedListSampleToHistogramGenerator() {}
	void GenerateData() ;

private:
	const TListSample* m_List ;
	const TListSample* m_WeightList ;
	HistogramType::Pointer m_Histogram ;
	HistogramSizeType m_Sizes ;
	float m_MarginalScale ;
	MeasurementVectorType m_HistogramMin;
	MeasurementVectorType m_HistogramMax;
	bool m_AutoMinMax;
};

EndNamespace(dH)
