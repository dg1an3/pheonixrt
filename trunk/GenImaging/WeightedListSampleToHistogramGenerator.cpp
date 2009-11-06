//
//
#include "StdAfx.h"

#include <WeightedListSampleToHistogramGenerator.h>

BeginNamespace(dH)

//////////////////////////////////////////////////////////////////////////////
WeightedListSampleToHistogramGenerator::WeightedListSampleToHistogramGenerator()
{
	m_Sizes.Fill(0) ;
	m_Histogram = HistogramType::New() ;
	m_MarginalScale = 100 ;
	m_HistogramMin.Fill(0);
	m_HistogramMax.Fill(0);
	m_AutoMinMax = true;
}  

//////////////////////////////////////////////////////////////////////////////
void
WeightedListSampleToHistogramGenerator::GenerateData()
{
	TListSample::MeasurementVectorType lower;
	TListSample::MeasurementVectorType upper;

	HistogramType::MeasurementVectorType h_upper  = m_HistogramMax;
	HistogramType::MeasurementVectorType h_lower = m_HistogramMin;

	// must test for the list size to avoid making FindSampleBound() segfault.
	// Also, the min and max can't be found automatically in that case. We can
	// only return an empty histogram
	if( m_AutoMinMax && m_List->Size() != 0 )
	{
		FindSampleBound(m_List, m_List->Begin(),
			m_List->End(), lower, upper) ;

		float margin ;

		for ( unsigned int i = 0 ; i < MeasurementVectorSize ; i++ )
		{
			if ( !NumericTraits< THistogramMeasurement >::is_integer )
			{
				margin = 
					( (THistogramMeasurement)(upper[i] - lower[i]) / 
					(THistogramMeasurement) m_Sizes[i] ) / 
					(THistogramMeasurement) m_MarginalScale ;
				h_upper[i] = (THistogramMeasurement) (upper[i] + margin) ;
				if(h_upper[i] <= upper[i])
				{ 
					// an overflow has occurred therefore set upper to upper
					h_upper[i] = upper[i];
					// Histogram measurement type would force the clipping the max value.
					// Therefore we must call the following to include the max value:
					m_Histogram->SetClipBinsAtEnds(false);
					// The above function is okay since here we are within the autoMinMax 
					// computation and clearly the user intended to include min and max.
				}
			}
			else
			{
				h_upper[i] = ((THistogramMeasurement) upper[i]) + 
					NumericTraits< THistogramMeasurement >::One ;
				if(h_upper[i] <= upper[i])
				{ 
					// an overflow has occurred therefore set upper to upper
					h_upper[i] = upper[i];
					// Histogram measurement type would force the clipping the max value.
					// Therefore we must call the following to include the max value:
					m_Histogram->SetClipBinsAtEnds(false);
					// The above function is okay since here we are within the autoMinMax 
					// computation and clearly the user intended to include min and max.
				}
			}
			h_lower[i] = ( THistogramMeasurement) lower[i] ;
		}
	}

	// initialize the Histogram object using the sizes and
	// the upper and lower bound from the FindSampleBound function
	m_Histogram->Initialize(m_Sizes, h_lower, h_upper) ;

	TListSample::ConstIterator iter = m_List->Begin() ;
	TListSample::ConstIterator iterWeight = m_WeightList->Begin() ;
	TListSample::ConstIterator last = m_List->End() ;
	TListSample::ConstIterator lastWeight = m_WeightList->End() ;

	HistogramType::IndexType index ;
	TListSample::MeasurementVectorType lvector ;
	TListSample::MeasurementVectorType weight ;
	HistogramType::MeasurementVectorType hvector ;
	unsigned int i;
	while (iter != last)
	{
		ASSERT(iterWeight != lastWeight);
		lvector = iter.GetMeasurementVector() ;
		weight = iterWeight.GetMeasurementVector();
		for ( i = 0 ; i < HistogramType::MeasurementVectorSize ; i++)
		{
			hvector[i] = (THistogramMeasurement) lvector[i] ;
		}

		m_Histogram->GetIndex(hvector,index);
		if (!m_Histogram->IsIndexOutOfBounds(index))
		{
			double prevTotalFreq = m_Histogram->GetTotalFrequency();

			// if the measurement vector is out of bound then
			// the GetIndex method has returned an index set to the max size of
			// the invalid dimension - even if the hvector is less than the minimum
			// bin value.
			// If the index isn't valid, we don't increase the frequency.
			// See the comments in Histogram->GetIndex() for more info.
			m_Histogram->IncreaseFrequency(index, weight[0]) ;
		}
		++iter ;
		++iterWeight ;
	}
}

EndNamespace(dH)


