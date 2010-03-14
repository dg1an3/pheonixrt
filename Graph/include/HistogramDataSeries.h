//
//
#pragma once

#include <itkScalarImageToHistogramGenerator.h>
using namespace itk;

#include <ScalarImageToWeightedHistogramGenerator.h>
#include <DataSeries.h>
#include <Structure.h>

BeginNamespace(dH)

//////////////////////////////////////////////////////////////////////////////
class HistogramDataSeries :
	public dH::DataSeries
{
public:
	HistogramDataSeries();
	virtual ~HistogramDataSeries(void);

	// itk typedefs
	typedef HistogramDataSeries Self;
	typedef DataSeries Superclass;
	typedef SmartPointer<Self> Pointer;
	typedef SmartPointer<const Self> ConstPointer;

	// defines itk's New and CreateAnother static functions
	itkNewMacro(Self);

	// the dose matrix for which histogram is needed
	DeclareMemberSPtrGet(DoseMatrix, VolumeReal);
	void SetDoseMatrix(VolumeReal *pDoseMatrix);

	// the structure for which histogram is needed
	DeclareMemberSPtrGet(Structure, dH::Structure);
	void SetStructure(dH::Structure *pStructure);

	// helper typedefs for the embedded objects
	typedef dH::ScalarImageToWeightedHistogramGenerator HistogramGeneratorType;
	typedef HistogramGeneratorType::HistogramType HistogramType;

	// the contained histogram generator
	DeclareMemberSPtr(HistogramGenerator, HistogramGeneratorType);

	// over-ride to update histo curve
	virtual void UpdateCurve();

private:
	// pointer the structure region
	VolumeReal::ConstPointer m_pConformRegion;
};

EndNamespace(dH)