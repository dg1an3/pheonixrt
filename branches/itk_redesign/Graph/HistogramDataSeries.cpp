//
//
#include "StdAfx.h"

#include <HistogramDataSeries.h>
#include <Graph.h>

namespace dH {

////////////////////////////////////////////////////////////////////////////
HistogramDataSeries::HistogramDataSeries()
{
	CreateMemberSPtr(HistogramGenerator, HistogramGeneratorType);

	GetHistogramGenerator()->SetNumberOfBins( 50 );
	GetHistogramGenerator()->SetMarginalScale( 10.0 );
}	

////////////////////////////////////////////////////////////////////////////
HistogramDataSeries::~HistogramDataSeries(void)
{
}	

////////////////////////////////////////////////////////////////////////////
void 
	HistogramDataSeries::SetDoseMatrix(VolumeReal *pDoseMatrix)
{
	m_pDoseMatrix = pDoseMatrix;

	GetHistogramGenerator()->SetInput(pDoseMatrix);

	if (GetStructure())
	{
		m_pConformRegion = GetStructure()->GetConformRegion(GetDoseMatrix());
		GetHistogramGenerator()->SetWeightImage(m_pConformRegion);
	}
}

////////////////////////////////////////////////////////////////////////////
void 
	HistogramDataSeries::SetStructure(dH::Structure *pStructure)
{
	m_pStructure = pStructure;

	if (GetDoseMatrix())
	{
		m_pConformRegion = pStructure->GetConformRegion(GetDoseMatrix());
		GetHistogramGenerator()->SetWeightImage(m_pConformRegion);
	}
}

////////////////////////////////////////////////////////////////////////////
void 
	HistogramDataSeries::UpdateCurve()
{
	// see if we have all info
	if (GetDoseMatrix() == NULL
		|| m_pConformRegion.IsNull())
		return;

	// see if dose matrix and structure region are unchanged
	if (GetDoseMatrix()->GetUpdateMTime() <= GetMTime()
		&& m_pConformRegion->GetUpdateMTime() <= GetMTime())
		return;

	// make sure color is set
	SetColor(GetStructure()->GetColor());

	// create or reset the curve
	if (GetCurve() == NULL)
	{
		SetCurve(dH::DataSeries::CurveType::New());
	}
	else
	{
		GetCurve()->Initialize();
	}

	// update the histogram
	HistogramGeneratorType *pGenerator = const_cast<HistogramGeneratorType *>(GetHistogramGenerator());
	pGenerator->Compute();
	const HistogramType * pHistogram = pGenerator->GetOutput();

	// get sum (for calculating bins)
	REAL Sum = pHistogram->GetTotalFrequency();
	if (Sum == 0.0)
		return;

	// now iterate over histogram bins
	HistogramType::ConstIterator iter = pHistogram->Begin();
	REAL cumulativeFrequency = 1.0;
	for (; iter != pHistogram->End(); ++iter)
	{
		REAL binValue = iter.GetMeasurementVector()[0];

		dH::DataSeries::CurveType::VertexType vertex;
		vertex[0] = 100.0 * binValue;
		vertex[1] = 100.0 * cumulativeFrequency;
		GetCurve()->AddVertex(vertex);

		cumulativeFrequency -= iter.GetFrequency() / Sum;
	}

	// set the modified flag, to avoid this calculation if nothing has changed
	this->Modified();
}

}